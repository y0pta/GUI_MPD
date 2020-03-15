#include "ccmdtransmitter.h"

QList<SSettings> CCmdTransmitter::readSettings(QIODevice &dev)
{
    QList<SSettings> res;
    SSettings temp;
    temp = readNextSettings(dev);
    while (temp.getType() != ESettingsType::eNoSection) {
        res.append(temp);
        temp = readNextSettings(dev);
    }
    return res;
}

SSettings CCmdTransmitter::readNextSettings(QIODevice &dev)
{
    SSettings res;
    SSettingsSerial settSerial;
    SCommonSettings settCommon;

    QByteArray data = dev.peek(dev.size());
    //выясняем,какой тип настроек следующий и есть ли там вообще настройки
    ESettingsType nextType = readNextSectionType(data);
    //настроек не оказалось
    if (nextType == ESettingsType::eNoSection)
        return res;

    //вычленяем "сырые" настройки от SETTINGS_TYPE до SETTINGS_INTERRUPTOR (Пример: от [common] до
    //[end])
    int posBegin = data.indexOf(getTypeStr(nextType)) + getTypeStr(nextType).size();
    int posEnd = data.indexOf(SETTINGS_INTERRUPTOR, posBegin);
    QByteArray settData = dev.read(posEnd + SETTINGS_INTERRUPTOR.size());
    settData.remove(0, posBegin);

    switch (nextType) {
    case ESettingsType::eCommon:
        fillSettings(settData, settCommon);
        return settCommon;
        break;
    case ESettingsType::eSerial:
        fillSettings(settData, settSerial);
        return settSerial;
        break;
    default:
        return res;
    }
}

template<typename SSettingsClass>
void CCmdTransmitter::fillSettings(const QByteArray &rawSett, SSettingsClass &sett)
{
    auto strings = rawSett.split('\n');
    for (auto string : strings) {
        for (auto it = sett.fields.begin(); it != sett.fields.end(); it++) {
            if (string.contains(it.key().toStdString().c_str())) {
                string.replace(it.key(), "");
                string.replace(" ", "");
                string.replace(":", "");
                it.value() = string;
            }
        }
    }
}

ESettingsType CCmdTransmitter::readNextSectionType(const QByteArray &data)
{
    int posInterruptor = data.indexOf(SETTINGS_INTERRUPTOR);
    if (posInterruptor == -1)
        return ESettingsType::eNoSection;

    while (posInterruptor > -1) {
        // Находим первое вхождение SETTINGS_INTERRUPTOR. Если нет SETTINGS_INTERRUPTOR, значит, нет
        // целых настроек

        // проверяем вхождение в data заголовков существующих секций
        for (auto it = SETTINGS_TYPES.begin(); it != SETTINGS_TYPES.end(); it++) {
            int posHeader = data.indexOf(it.value());
            // нашли совпадение и секция идет перед SETTINGS_INTERRUPTOR
            if (posHeader != -1 && posHeader < posInterruptor) {
                return it.key();
            }
        }
        //не нашли вхождение в data заголовков существующих секций, переходим к следующему
        // SETTINGS_INTERRUPTOR
        posInterruptor = data.indexOf(SETTINGS_INTERRUPTOR, posInterruptor + 1);
    }
    return ESettingsType::eNoSection;
}

void CCmdTransmitter::requestSettings(QIODevice &dev, ESettingsType type,
                                      const QPair<QString, QString> &parameter)
{
    QString str;
    str += SETTINGS_REQUEST;
    str += "\n";
    QString strType = getTypeStr(type);
    strType.replace("[", "");
    strType.replace("]", "");
    str += strType;
    str += "\n";
    if (parameter.first.size() > 1 && parameter.second.size() > 1) {
        str = str + parameter.first + ": " + parameter.second + "\n";
    }
    str += SETTINGS_INTERRUPTOR;
    str += "\n";
    dev.write(str.toStdString().c_str());
}

void CCmdTransmitter::sendSettings(QIODevice &dev, const SSettings &sett)
{
    QString str;
    str += getTypeStr(sett.getType());
    str += "\n";
    for (auto it = sett.fields.begin(); it != sett.fields.end(); it++) {
        if (!it.value().isEmpty())
            str = str + it.key() + ": " + it.value() + "\n";
    }
    dev.write(str.toStdString().c_str());
}
////находит и возвращает имя первой секции в буфере устройства, если имя секции существует и
////завершается sectionIntterrupter ([end]). Если секции с таким именем нет или нет
//// SECTION_INTERRUPTOR возвращает ESettingsType::eUnknown
// ESettingsType CCmdTransmitter::getNextSectionType(QIODevice &dev)
//{
//    QByteArray data = dev.peek(dev.size());
//    QString nameSection;

//    // удостоверимся, что есть SECTION_INTERRUPTOR
//    int posInterruptor = data.indexOf(SECTION_INTERRUPTOR);
//    if (posInterruptor == -1)
//        return ESettingsType::eUnknown;

//    int i = 0;
//    // ищем имя секции, зная, что оно заключено в []
//    while (i < posInterruptor) {
//        int posBegin = data.indexOf('[', i);
//        int posEnd = data.indexOf(']', i);
//        // вхождение секции не найдено
//        if (posBegin == -1 || posEnd == -1)
//            return ESettingsType::eUnknown;
//        if (posEnd > posBegin) {
//            nameSection = data.mid(posBegin, posEnd - posBegin + 1);
//            i = posInterruptor;
//        } else {
//            i = posEnd + 1;
//        }
//    }
//    // проверяем, что такая секция есть
//    for (auto it = sectionTypes.begin(); it != sectionTypes.end(); it++) {
//        if (it.value() == nameSection)
//            return it.key();
//    }
//    return ESettingsType::eUnknown;
//}

// SSettingsSerial &CCmdTransmitter::getSettings(QIODevice &dev, SSettingsSerial &sett)
//{
//    QByteArray data = dev.peek(dev.size());
//    int posBegin = data.indexOf(sectionTypes[ESettingsType::eSerial])
//            + sectionTypes[ESettingsType::eSerial].size();
//    int posEnd = data.indexOf(SECTION_INTERRUPTOR);
//    QByteArray settStr = dev.read(posEnd + SECTION_INTERRUPTOR.size());
//    //отрезаем часть с настройками
//    settStr = settStr.mid(posBegin, posEnd);

//    auto strings = settStr.split('\n');
//    for ()
//}

// SCommonSettings &CCmdTransmitter::getSettings(QIODevice &dev, SCommonSettings &sett) {}

// void CCmdTransmitter::sendSettings(QIODevice &dev, const SSettingsSerial &sett)
//{
//    QString str = sectionTypes[ESettingsType::eSerial] + "\n";

//    if (sett.iface == EIface::eUndefined)
//        return;
//    str += "iface: " + ifaces[sett.iface] + "\n";

//    if (sett.enabled) {
//        str += "status: on\n";
//        str = str + "baudRate: " + QString::number(sett.baudRate) + "\n";
//        str = str + "dataBits: " + QString::number(sett.dataBits) + "\n";
//        if (sett.parity == QSerialPort::NoParity)
//            str += "parity: no\n";
//        else if (sett.parity == QSerialPort::EvenParity)
//            str += "parity: even\n";
//        else if (sett.parity == QSerialPort::OddParity)
//            str += "parity: odd\n";
//        str = str + "stopBits: " + QString::number(sett.stopBits, 'f', 1) + "\n";
//        if (sett.writeDelay)
//            str = str + "delay: " + QString::number(sett.writeDelay) + "\n";
//        if (sett.waitPacketTime)
//            str = str + "waitTime: " + QString::number(sett.waitPacketTime) + "\n";
//    } else {
//        str += "status: off\n";
//    }

//    str += SECTION_INTERRUPTOR + "\n";
//    dev.write(str.toStdString().c_str());
//}

// void CCmdTransmitter::sendSettings(QIODevice &dev, const SCommonSettings &sett)
//{
//    QString str = sectionTypes[ESettingsType::eCommon] + "\n";
//    str += "mode: " + modes[sett.mode] + "\n";
//    str += SECTION_INTERRUPTOR + "\n";
//    dev.write(str.toStdString().c_str());
//}

// void CCmdTransmitter::sendSettingsRequest()
//{
//    QString str = "[get]\n";
//    str += "mode: \n";
//    str += SECTION_INTERRUPTOR + "\n";
//    // return str.toStdString().c_str();
//}

// static QByteArray sendSettings(QIODevice &dev, const SCommonSettings &sett) {}
