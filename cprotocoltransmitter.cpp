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
    m_device = dev;
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
    if (m_device) {
        switch (type) {
        case eSerial:
            lastReq = SSerialSection();
            break;
        case eCommon:
            lastReq = SCommonSection();
            break;
        case eStat:
            lastReq = SStatSection();
            break;
        default:
            return;
        }
        QString str = "get(" + getStr(type) + ")\n";
        str.remove('[');
        str.remove(']');
        m_device->write(str.toStdString().c_str());
    }
}

void CProtocolTransmitter::setRequest(const SSection &section)
{
    lastReq = section;
    if (m_device) {
        auto type = section.getType();
        /// запросы на изменение можно посылать только для eSeria и eCommon
        if (!(type == eSerial || type == eCommon))
            return;

        QString str;
        str += getStr(type);
        str += "\n";
        for (auto it = section.fields.begin(); it != section.fields.end(); it++) {
            if (!it.value().isEmpty())
                str = str + it.key() + ": " + it.value() + "\n";
        }
        m_device->write(str.toStdString().c_str());

        m_timer.start(TIMEOUT);
    }
}

void CProtocolTransmitter::serviceRequest(EServiceCommand cmd)
{
    if (m_device) {
        QString str = SERVICE_COMMAND[cmd] + "\n";
        m_device->write(str.toStdString().c_str());
        SDebugSection temp;
        temp.fields[SERVICE_COMMAND[cmd]] = SERVICE_COMMAND[cmd];
        lastReq = temp;
        m_timer.start();
    }
}

void CProtocolTransmitter::readyRead()
{
    if (m_device == nullptr)
        return;

    /// отделяем всю служебную информацию (все, что не в секции) и узнаем тип следующей секции
    for (ESectionType type = separateNoSection(); type != eNoSection;
         type = separateNoSection()) { ///пока есть, что читать

        SSection sect = SSection();
        /// читаем секцию
        auto rawSection = readRawSection(getStr(type));
        /// нечего читать, секция не найдена
        if (rawSection.size() < 1)
            return;
        /// заполняем
        switch (type) {
        case eCommon: {
            SCommonSection cs;
            fillSection(rawSection, cs);
            sect = cs;
            break;
        }
        case eSerial: {
            SSerialSection ss;
            fillSection(rawSection, ss);
            sect = ss;
            break;
        }
        case eDebug: {
            SDebugSection ds;
            fillSection(rawSection, ds);
            sect = ds;
            break;
        }
        case eStat: {
            SStatSection sts;
            fillSection(rawSection, sts);
            sect = sts;
            break;
        }
        }
        QDebug dbg = qDebug();
        dbg << sect;
        /// разделяем подтверждения и ответы на запросы
        if (sect.isConfirmation()) {
            processConfirmation(sect);
        } else {
            emit s_sectionRead(sect);
        }
    }
}

void CProtocolTransmitter::requestTimeout()
{
    if (lastReq.getType() != eDebug)
        emit s_requestFailed(lastReq);
    else {
        for (auto it = lastReq.fields.begin(); it != lastReq.fields.end(); it++) {
            if (!it.value().isEmpty())
                emit s_serviceRequestFailed(it.key());
        }
    }
    lastReq = SSection();
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
            emit s_noSectionRead(QString(m_device->readAll()));
            return eNoSection;
        }

        ///перед началом что-то есть=>кидаем все, что до начала в отладку и начинаем поиск заново
        if (posHeader > 0) {
            emit s_noSectionRead(m_device->read(posHeader));
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

template<typename SSectionClass>
void CProtocolTransmitter::fillSection(const QByteArray &rawSection, SSectionClass &sect)
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
}

void CProtocolTransmitter::processConfirmation(const SSection &sect)
{
    QDebug dbg = qDebug();
    if (!sect.isConfirmation() || lastReq.getType() == eNoSection)
        return;

    auto listErrors = sect.getErrorFields();
    if (listErrors.size() < 1 && lastReq.getType() == sect.getType()) {
        emit s_requestConfirmed(lastReq);
        dbg << "Last req confirmed" << lastReq;
    } else {
        emit s_requestError(listErrors);
        dbg << "Error list: " << endl;
        foreach (auto err, listErrors) {
            dbg << err << endl;
        }
    }

    lastReq = SSection();
}

//// ESectionType CProtocolTransmitter::nextSectionType() const
////{
////    QByteArray data = m_device->peek(m_device->size());
////    int posInterruptor = data.indexOf(SECTION_INTERRUPTOR);

////    if (posInterruptor == -1)
////        return eNoSection;

////    while (posInterruptor > -1) {
////        /// находим заголовок секции среди существующих имен
////        for (auto it = SECTION_TYPES.begin(); it != SECTION_TYPES.end(); it++) {
////            int posHeader = data.indexOf(it.value());
////            /// нашли заголовок и название секции идет перед окончанием секции
////            if (posHeader != -1 && posHeader < posInterruptor) {
////                return it.key();
////            }
////        }
////        ///не нашли заголовков существующих секций перед окончанием секции, переходим к
///следующему /        ///окончанию секции /        posInterruptor =
/// data.indexOf(SECTION_INTERRUPTOR, posInterruptor + 1); /    } /    return eNoSection;
////}

// QList<SSettings> CCmdTransmitter::readSettings(QIODevice *dev)
//{
//    QList<SSettings> res;
//    SSettings temp;
//    temp = readNextSettings(dev);
//    while (temp.getType() != ESettingsType::eNoSection) {
//        res.append(temp);
//        temp = readNextSettings(dev);
//    }
//    return res;
//}

// SSettings CCmdTransmitter::readNextSettings(QIODevice *dev)
//{
//    SSettings res;
//    SSettingsSerial settSerial;
//    SCommonSettings settCommon;

//    QByteArray data = dev->peek(dev->size());
//    //выясняем,какой тип настроек следующий и есть ли там вообще настройки
//    ESettingsType nextType = readNextSectionType(data);
//    //настроек не оказалось
//    if (nextType == ESettingsType::eNoSection)
//        return res;

//    //вычленяем "сырые" настройки от SETTINGS_TYPE до SETTINGS_INTERRUPTOR (Пример: от
//    [common] до
//    //[end])
//    int posBegin = data.indexOf(getTypeStr(nextType)) + getTypeStr(nextType).size();
//    int posEnd = data.indexOf(SETTINGS_INTERRUPTOR, posBegin);
//    QByteArray settData = dev->read(posEnd + SETTINGS_INTERRUPTOR.size());
//    // До секции идут критические ошибки
//    QByteArray err = settData.mid(0, posBegin);

//    settData.remove(0, posBegin);

//    switch (nextType) {
//    case ESettingsType::eCommon:
//        fillSettings(settData, settCommon);
//        if (!err.isEmpty())
//            settCommon.fields.insert(SETTINGS_ERROR, err);
//        return settCommon;
//        break;
//    case ESettingsType::eSerial:
//        fillSettings(settData, settSerial);
//        if (!err.isEmpty())
//            settCommon.fields.insert(SETTINGS_ERROR, err);
//        return settSerial;
//        break;
//    default:
//        return res;
//    }
//}

// template<typename SSettingsClass>
// void CCmdTransmitter::fillSettings(const QByteArray &rawSett, SSettingsClass &sett)
//{
//}

// ESettingsType CCmdTransmitter::readNextSectionType(const QByteArray &data)
//{
//    int posInterruptor = data.indexOf(SETTINGS_INTERRUPTOR);
//    if (posInterruptor == -1)
//        return ESettingsType::eNoSection;

//    while (posInterruptor > -1) {
//        // Находим первое вхождение SETTINGS_INTERRUPTOR. Если нет SETTINGS_INTERRUPTOR,
//        значит, нет
//        // целых настроек

//        // проверяем вхождение в data заголовков существующих секций
//        for (auto it = SETTINGS_TYPES.begin(); it != SETTINGS_TYPES.end(); it++) {
//            int posHeader = data.indexOf(it.value());
//            // нашли совпадение и секция идет перед SETTINGS_INTERRUPTOR
//            if (posHeader != -1 && posHeader < posInterruptor) {
//                return it.key();
//            }
//        }
//        //не нашли вхождение в data заголовков существующих секций, переходим к следующему
//        // SETTINGS_INTERRUPTOR
//        posInterruptor = data.indexOf(SETTINGS_INTERRUPTOR, posInterruptor + 1);
//    }
//    return ESettingsType::eNoSection;
//}

// void CCmdTransmitter::requestSettings(QIODevice *dev, ESettingsType type,
//                                      const QPair<QString, QString> &parameter)
//{
//    QString str;
//    str += SETTINGS_REQUEST;
//    str += "\ntype: ";
//    QString strType = getTypeStr(type);
//    strType.replace("[", "");
//    strType.replace("]", "");
//    str += strType;
//    str += "\n";
//    if (parameter.first.size() > 1 && parameter.second.size() > 1) {
//        str = str + parameter.first + ": " + parameter.second + "\n";
//    }
//    str += SETTINGS_INTERRUPTOR;
//    str += "\n";
//    dev->write(str.toStdString().c_str());
//}

// void CCmdTransmitter::sendSettings(QIODevice *dev, const SSettings &sett)
//{
//    QString str;
//    str += getTypeStr(sett.getType());
//    str += "\n";
//    for (auto it = sett.fields.begin(); it != sett.fields.end(); it++) {
//        if (!it.value().isEmpty())
//            str = str + it.key() + ": " + it.value() + "\n";
//    }
//    dev->write(str.toStdString().c_str());
//}
