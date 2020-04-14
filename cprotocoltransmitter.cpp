#include "cprotocoltransmitter.h"
#include <QDebug>
#include <QList>
#include <QtAlgorithms>
#include <stdlib.h>
// максимальное время ожидания(мс)
const int TIMEOUT = 2000;

///Проверка на вхождение заголовка секции в data. Если заголовка нет, веозврат -1. В параметре
int indexOfSectionHeader(const QByteArray& data, ESectionType& type)
{
    ///позиция - длина заголовка
    QMap<int, ESectionType> headers;
    for (auto it = SECTION_TYPES.begin(); it != SECTION_TYPES.end(); it++) {
        int posHeader = data.indexOf(it.value());
        if (posHeader != -1) {
            headers[posHeader] = it.key();
        }
    }

    if (headers.size() < 1) {
        type = eNoSection;
        return -1;
    } else {
        auto it = headers.lowerBound(0);
        type = it.value();
        return it.key();
    }
}

CProtocolTransmitter::CProtocolTransmitter(QIODevice* dev, QObject* parent)
    : QObject(parent)
{
    m_timer.setInterval(TIMEOUT);
    m_timer.setSingleShot(true);
    connect(&m_timer, &QTimer::timeout, this, &CProtocolTransmitter::requestTimeout);
    setDevice(dev);
}
CProtocolTransmitter::CProtocolTransmitter(QObject* parent)
    : QObject(parent)
{
    m_timer.setInterval(TIMEOUT);
    m_timer.setSingleShot(true);
    connect(&m_timer, &QTimer::timeout, this, &CProtocolTransmitter::requestTimeout);
}

void CProtocolTransmitter::setDevice(QIODevice* dev)
{
    m_device = dev;
    connect(m_device, &QIODevice::readyRead, this, &CProtocolTransmitter::readyRead);
}

void CProtocolTransmitter::removeDevice()
{
    if (!m_device)
        disconnect(m_device, &QIODevice::readyRead, this, &CProtocolTransmitter::readyRead);
    m_device = nullptr;
}

void CProtocolTransmitter::getRequest(ESectionType type)
{
    switch (type) {
    case eSerial:
        pushQueu(SSerialSection());
        break;
    case eCommon:
        pushQueu(SCommonSection());
        break;
    case eStat:
        pushQueu(SStatSection());
        break;
    default:
        return;
    }
}

void CProtocolTransmitter::setRequest(const SSection& section)
{
    auto type = section.getType();
    /// запросы на изменение можно посылать только для eSeria и eCommon
    if (!(type == eSerial || type == eCommon))
        return;

    pushQueu(section);
}

void CProtocolTransmitter::serviceRequest(EServiceCommand cmd)
{
    SDebugSection temp;
    temp.fields[SERVICE_COMMAND[cmd]] = SERVICE_COMMAND[cmd];
    pushQueu(temp);
}

void CProtocolTransmitter::readyRead()
{
    // if (reqProcessing) {
    /// результат обработки последнего запроса
    bool res = true;
    auto lastReq = SSection();
    if (reqProcessing)
        lastReq = requests.head();

    /// отделяем всю служебную информацию (все, что не в секции) и узнаем тип следующей
    /// секции пока есть, что читать
    for (ESectionType type = separateNoSection(); type != eNoSection; type = separateNoSection()) {

        SSection sect = SSection();
        /// читаем секцию
        auto rawSection = readRawSection(getStr(type));
        /// нечего читать, секция не найдена
        if (rawSection.size() < 1)
            return;
        /// секция есть, заполняем
        switch (type) {
        case eCommon:
            sect = SCommonSection();
            break;
        case eSerial:
            sect = SSerialSection();
            break;
        case eDebug:
            sect = SDebugSection();
            break;
        case eStat:
            sect = SStatSection();
            break;
        }
        fillSection(rawSection, sect);
        /// тип ответа не совпадает с типом запроса, просто выходим
        if (type != lastReq.getType())
            return;

        /// разделяем подтверждения и ответы на запросы
        if (SSection::isConfirmation(sect)) {
            res = processConfirmation(lastReq, sect);
        } else {
            emit s_sectionRead(sect);
            /// на serial-запрос должно приходить несколько секций, поэтому serial-запрос
            /// завершается по тайм-ауту
            if (lastReq.getType() == eSerial) {
                res = false;
            } else {
                res = true;
            }
        } /// запрос выполнен, сворачиваемся
        if (res) {
            popQueu();
            break;
        }
    }
    //}
}

void CProtocolTransmitter::requestTimeout()
{
    s_debugInfo("  Last request time-out");
    auto request = popQueu();
    if (request.getType() == eDebug) {
        for (auto it = request.fields.begin(); it != request.fields.end(); it++) {
            if (!it.value().isEmpty())
                emit s_serviceRequestFailed(it.key());
        }
    } else if (request.getType() == eSerial)
        return;
    else
        emit s_requestFailed(request);
}

/// возвращает имя секции, если дальше есть целая секция
ESectionType CProtocolTransmitter::separateNoSection()
{
    while (m_device->size() > 0) {
        /// смотрим, что лежит в буфере (отладочное инфо или секция)
        QByteArray data = m_device->peek(m_device->size());
        s_debugInfo("Buffer: " + data);

        int posInterruptor = data.indexOf(SECTION_INTERRUPTOR);
        ESectionType typeHeader;
        int posHeader = indexOfSectionHeader(data, typeHeader);

        /// нет начала => все в отладку, ждем следующей инфы в буфере
        if (posHeader == -1) {
            emit s_debugInfo(QString(m_device->readAll()));
            return eNoSection;
        }

        ///перед началом что-то есть=>кидаем все, что до начала в отладку и начинаем поиск
        ///заново
        if (posHeader > 0) {
            emit s_debugInfo(m_device->read(posHeader));
            continue;
        }

        ///есть начало с 0й позиции, нет конца =>все до начала в отладку, ждем следующей инфы в
        ///буфере
        if (posInterruptor == -1) {
            return eNoSection;
        }
        ///есть начало с 0й позиции, есть конец (начало перед концом) => миссия выполнена
        else if (posInterruptor > -1) {
            return typeHeader;
        }
    }
    return eNoSection;
}

QByteArray CProtocolTransmitter::readRawSection(QString sectionName)
{
    /// проверяем, есть ли целая секция дальше
    QByteArray data = m_device->peek(m_device->size());

    int posBegin = data.indexOf(sectionName);
    int posEnd = data.indexOf(SECTION_INTERRUPTOR, posBegin);
    /// не нашли конца или начала
    if (posEnd == -1 || posBegin == -1)
        return QByteArray();

    /// читаем данные, стираем все лишнее
    QByteArray sectData = m_device->read(posEnd + SECTION_INTERRUPTOR.size() + LINE_BREAKER.size());
    if (posBegin != 0)
        sectData.remove(0, posBegin);
    return sectData;
}

void CProtocolTransmitter::fillSection(const QByteArray& rawSection, SSection& sect)
{
    /// статистика не разделется на строки и идет целым куском
    if (sect.getType() == eStat) {
        sect.fields[STAT_TEXT] = rawSection;
        sect.fields[STAT_TEXT].remove(SECTION_INTERRUPTOR);
        sect.fields[STAT_TEXT].remove(getStr(eStat));
    }

    else {
        auto strings = rawSection.split('\n');
        for (auto string : strings) {
            for (auto it = sect.fields.begin(); it != sect.fields.end(); it++) {
                auto fieldName = it.key();
                if (string.contains(fieldName.toStdString().c_str())) {
                    int pos = string.indexOf(fieldName);
                    // удаляем имя секции, двоеточие и пробел, '\r'
                    string.remove(0, pos + fieldName.size() + 2);
                    string.replace('\r', "");
                    it.value() = string;
                }
            }
        }
    }
    QDebug dbg = qDebug();
    dbg << sect;
}

bool CProtocolTransmitter::processConfirmation(const SSection& request, SSection& answer)
{
    QDebug dbg = qDebug();
    if (request.getType() != answer.getType())
        return false;

    if (request.getType() == eDebug) {
        return processServiceConfirmation(request, answer);
    }
    if (request.getType() == eSerial)
        answer.fields[SERIAL_IFACE] = request.fields[SERIAL_IFACE];

    auto listErrors = SSection::getErrorFields(answer);
    if (listErrors.size() < 1) {
        emit s_requestConfirmed(request);
        dbg << "Last request confirmed" << request;
    } else {
        emit s_requestError(answer);
        dbg << "Error list: " << endl;
        foreach (auto err, listErrors) {
            dbg << err << endl;
        }
    }

    return true;
}

bool CProtocolTransmitter::processServiceConfirmation(const SSection& request, const SSection& answer)
{
    /// если предыдущий запрос был service
    if (request.getType() == eDebug && answer.getType() == eDebug) {
        /// ищем подтверждение
        for (auto it = answer.fields.begin(); it != answer.fields.end(); it++) {
            if (!it.value().isEmpty()) {
                if (it.value() == SECTIONVAL_CONFIRMED) {
                    emit s_serviceRequestConfirmed(it.key());
                    qDebug() << "Last service-request confirmed: " << it.key();

                } else {
                    emit s_serviceRequestFailed(it.key());
                    qDebug() << "Last service-request failed: " << it.key();
                }
            }
        }
        return true;
    }
    return false;
}

bool CProtocolTransmitter::processNextRequest()
{
    QString str;
    SSection req = requests.head();
    auto type = req.getType();
    /// секция пустая => get-запрос
    if (SSection::isEmpty(req)) {
        str = "get(" + getStr(type) + ")\n";
        str.remove('[');
        str.remove(']');

    }
    /// секция заполнена => service- или set-зпрос
    else if (req.getType() == eDebug) {
        for (auto it = req.fields.begin(); it != req.fields.end(); it++) {
            if (!it.value().isEmpty())
                str = it.key() + "\n";
        }
    }
    /// set - запрос
    else {
        str += getStr(type);
        str += "\n";
        for (auto it = req.fields.begin(); it != req.fields.end(); it++) {
            if (!it.value().isEmpty())
                str = str + it.key() + ": " + it.value() + "\n";
        }
        str = str + SECTION_INTERRUPTOR + "\n";
    }
    /// отправляем запрос
    if (m_device) {
        qDebug() << "Request sent: " << str;
        m_device->write(str.toStdString().c_str());
        s_debugInfo("Request sent: " + str);
        m_timer.start();
        return true;
    } else
        return false;
}

void CProtocolTransmitter::pushQueu(const SSection& sect)
{
    requests.enqueue(sect);
    if (!reqProcessing) {
        reqProcessing = processNextRequest();
    }
}

SSection CProtocolTransmitter::popQueu()
{
    m_timer.stop();
    auto req = requests.dequeue();
    s_debugInfo("Last request closed: " + getStr(req.getType()));
    if (requests.size() > 0)
        reqProcessing = processNextRequest();
    else
        reqProcessing = false;
    return req;
}
