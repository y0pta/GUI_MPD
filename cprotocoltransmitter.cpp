#include "cprotocoltransmitter.h"
#include <QList>
#include <stdlib.h>
#include <QDebug>
// максимальное время ожидания(мс)
const int TIMEOUT = 1000;

///Проверка на вхождение заголовка секции в data. Если заголовка нет, веозврат -1. В параметре
int indexOfSectionHeader(const QByteArray &data, ESectionType &type)
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

CProtocolTransmitter::CProtocolTransmitter(QIODevice *dev, QObject *parent) : QObject(parent)
{
    m_timer.setSingleShot(true);
    connect(&m_timer, &QTimer::timeout, this, &CProtocolTransmitter::requestTimeout);
    setDevice(dev);
}
CProtocolTransmitter::CProtocolTransmitter(QObject *parent) : QObject(parent)
{
    m_timer.setSingleShot(true);
    connect(&m_timer, &QTimer::timeout, this, &CProtocolTransmitter::requestTimeout);
}

void CProtocolTransmitter::setDevice(QIODevice *dev)
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
        requests.enqueue(SSerialSection());
        break;
    case eCommon:
        requests.enqueue(SCommonSection());
        break;
    case eStat:
        requests.enqueue(SStatSection());
        break;
    default:
        return;
    }
}

void CProtocolTransmitter::setRequest(const SSection &section)
{
    auto type = section.getType();
    /// запросы на изменение можно посылать только для eSeria и eCommon
    if (!(type == eSerial || type == eCommon))
        return;

    requests.enqueue(section);
}

void CProtocolTransmitter::serviceRequest(EServiceCommand cmd)
{
    SDebugSection temp;
    temp.fields[SERVICE_COMMAND[cmd]] = SERVICE_COMMAND[cmd];
    requests.enqueue(temp);
}

void CProtocolTransmitter::readyRead()
{
    /// результат обработки последнего запроса
    bool res = true;
    ESectionType lastReq = requests.head().getType();

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
        case eDebug:
            sect = SDebugSection();
            break;
        case eStat:
            sect = SStatSection();
            break;
        }
        fillSection(rawSection, sect);

        /// разделяем подтверждения и ответы на запросы

        if (SSection::isConfirmation(sect)) {
            res = processConfirmation(sect);
        } else {
            emit s_sectionRead(sect);
            /// на serial-запрос должно приходить несколько секций, поэтому serial-запрос
            /// завершается по тайм-ауту
            if (lastReq == eSerial) {
                res = false;
            } else {
                requests.dequeue();
                m_timer.stop();
                res = true;
            }
        }
    }
    if (res)
        processNextRequest();
}

void CProtocolTransmitter::requestTimeout()
{
    auto request = requests.dequeue();
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
    QByteArray sectData = m_device->read(posEnd + SECTION_INTERRUPTOR.size());
    if (posBegin != 0)
        sectData.remove(0, posBegin);
    return sectData;
}

void CProtocolTransmitter::fillSection(const QByteArray &rawSection, SSection &sect)
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
                    // удаляем имя секции, двоеточие и пробел
                    string.remove(0, pos + fieldName.size() + 2);
                    it.value() = string;
                }
            }
        }
    }
    QDebug dbg = qDebug();
    dbg << sect;
}

bool CProtocolTransmitter::processConfirmation(const SSection &sect)
{
    QDebug dbg = qDebug();
    auto request = requests.head();
    if (request.getType() != sect.getType())
        return false;
    if (request.getType() == eDebug) {
        return processServiceConfirmation(sect);
    }

    auto listErrors = SSection::getErrorFields(sect);
    if (listErrors.size() < 1) {
        emit s_requestConfirmed(request);
        dbg << "Last request confirmed" << request;
    } else {
        emit s_requestError(listErrors);
        dbg << "Error list: " << endl;
        foreach (auto err, listErrors) {
            dbg << err << endl;
        }
    }
    /// запрос получил подтверждение, удаляем его из списка ожидания
    requests.dequeue();
    m_timer.stop();
    return true;
}

bool CProtocolTransmitter::processServiceConfirmation(const SSection &sect)
{
    auto request = requests.head();
    /// если предыдущий запрос был service
    if (request.getType() == eDebug && sect.getType() == eDebug) {
        /// ищем подтверждение
        for (auto it = sect.fields.begin(); it != sect.fields.end(); it++) {
            if (!it.value().isEmpty()) {
                if (it.key() == SECTIONVAL_CONFIRMED) {
                    emit s_serviceRequestConfirmed(it.value());
                    qDebug() << "Last service-request confirmed: " << it.value();

                } else {
                    emit s_serviceRequestFailed(it.value());
                    qDebug() << "Last service-request failed: " << it.value();
                }
            }
        }
        /// запрос обработан, удаляем его из списка ожидания
        requests.dequeue();
        m_timer.stop();
        return true;
    }
    return false;
}

void CProtocolTransmitter::processNextRequest()
{
    if (requests.size() > 0) {
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
        else {

            str += getStr(type);
            str += "\n";
            for (auto it = req.fields.begin(); it != req.fields.end(); it++) {
                if (!it.value().isEmpty()) {
                    /// service - запрос
                    if (type == eDebug)
                        str = it.key() + "\n";
                    /// set - запрос
                    else
                        str = str + it.key() + ": " + it.value() + "\n";
                }
            }
        }
        /// отправляем запрос
        if (m_device) {
            m_device->write(str.toStdString().c_str());
            m_timer.start();
        }
    }
}
