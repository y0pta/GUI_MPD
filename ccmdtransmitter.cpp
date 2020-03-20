#include "ccmdtransmitter.h"

QList<SSettings> CCmdTransmitter::readSettings(QIODevice *dev)
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

SSettings CCmdTransmitter::readNextSettings(QIODevice *dev)
{
    SSettings res;
    SSettingsSerial settSerial;
    SCommonSettings settCommon;

    QByteArray data = dev->peek(dev->size());
    //выясняем,какой тип настроек следующий и есть ли там вообще настройки
    ESettingsType nextType = readNextSectionType(data);
    //настроек не оказалось
    if (nextType == ESettingsType::eNoSection)
        return res;

    //вычленяем "сырые" настройки от SETTINGS_TYPE до SETTINGS_INTERRUPTOR (Пример: от [common] до
    //[end])
    int posBegin = data.indexOf(getTypeStr(nextType)) + getTypeStr(nextType).size();
    int posEnd = data.indexOf(SETTINGS_INTERRUPTOR, posBegin);
    QByteArray settData = dev->read(posEnd + SETTINGS_INTERRUPTOR.size());
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

void CCmdTransmitter::requestSettings(QIODevice *dev, ESettingsType type,
                                      const QPair<QString, QString> &parameter)
{
    QString str;
    str += SETTINGS_REQUEST;
    str += "\ntype: ";
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
    dev->write(str.toStdString().c_str());
}

bool CCmdTransmitter::isConfirmation(SSettings sett)
{
    for (auto field : sett.fields) {
        if (field == "ok" || field.contains("<"))
            return true;
    }
    return false;
}

void CCmdTransmitter::sendSettings(QIODevice *dev, const SSettings &sett)
{
    QString str;
    str += getTypeStr(sett.getType());
    str += "\n";
    for (auto it = sett.fields.begin(); it != sett.fields.end(); it++) {
        if (!it.value().isEmpty())
            str = str + it.key() + ": " + it.value() + "\n";
    }
    dev->write(str.toStdString().c_str());
}
