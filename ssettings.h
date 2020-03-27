#ifndef SSETTINGS_H
#define SSETTINGS_H
#include <QSerialPort>
#include <QMap>

const char FIELD_NAME[] = "name";
enum EState { eConnected, eDisconnected, eError, eDisabled };

enum ESettingsType { eNoSection = -1, eSerial, eCommon };
const QMap<ESettingsType, QString> SETTINGS_TYPES = {
    { ESettingsType::eSerial, "[serial]" },
    { ESettingsType::eCommon, "[common]" },
};

static QString getTypeStr(ESettingsType type)
{
    return SETTINGS_TYPES[type];
}

const QString SETTINGS_INTERRUPTOR = "[end]";
const QString SETTINGS_REQUEST = "[get]";
const QString SETTINGS_CONFIRMED = "<ok>";
const QString SETTINGS_ERROR = "error";

struct SSettings {
    QMap<QString, QString> fields;
    ESettingsType getType() const { return m_type; }
    bool isConfirmation() const
    {
        for (auto field : fields) {
            if (field.contains("<") && field.contains(">"))
                return true;
        }
        return false;
    }
    QString getErrorsStr() const
    {
        if (fields.contains(SETTINGS_ERROR))
            return fields[SETTINGS_ERROR];
        else
            return QString();
    }
    QList<QString> getErrorFields() const
    {
        QList<QString> res;
        for (auto it = fields.begin(); it != fields.end(); it++) {
            if (!it.value().contains(SETTINGS_CONFIRMED))
                res.push_back(it.key());
        }
        return res;
    }

protected:
    ESettingsType m_type { eNoSection };
};

const QString SERIAL_IFACE = "iface";
const QString SERIAL_STATUS = "status";
const QString SERIAL_BAUDRATE = "baudRate";
const QString SERIAL_DATABITS = "dataBits";
const QString SERIAL_PARITY = "parity";
const QString SERIAL_STOPBITS = "stopBits";
const QString SERIAL_WRITEDELAY = "delay";
const QString SERIAL_WAITPACKETTIME = "waitTime";

const QString SERIAL_IFACE_RS232 = "RS-232";
const QString SERIAL_IFACE_RS485 = "RS-485";
const QString SERIAL_IFACE_RADIO = "Radio";

struct SSettingsSerial : public SSettings {
    SSettingsSerial()
    {
        m_type = eSerial;
        fields = { { SERIAL_IFACE, "" },      { SERIAL_STATUS, "" },        { SERIAL_BAUDRATE, "" },
                   { SERIAL_DATABITS, "" },   { SERIAL_PARITY, "" },        { SERIAL_STOPBITS, "" },
                   { SERIAL_WRITEDELAY, "" }, { SERIAL_WAITPACKETTIME, "" } };
    }
    static SSettingsSerial getSerialDefault(QString ifaceName = QString())
    {
        SSettingsSerial sett;
        sett.fields[SERIAL_IFACE] = ifaceName;
        sett.fields[SERIAL_STATUS] = "on";
        sett.fields[SERIAL_BAUDRATE] = "115200";
        sett.fields[SERIAL_DATABITS] = "8";
        sett.fields[SERIAL_PARITY] = "no";
        sett.fields[SERIAL_STOPBITS] = "1";
        sett.fields[SERIAL_WRITEDELAY] = "200";
        sett.fields[SERIAL_WAITPACKETTIME] = "200";
        return sett;
    }
};

const QString COMMON_MODE = "mode";

struct SCommonSettings : public SSettings {
    SCommonSettings()
    {
        m_type = eCommon;
        fields = { { COMMON_MODE, "" } };
    }
};

// const QString SECTION_INTERRUPTOR = "[end]";
// enum EState { eConnected, eDisconnected, eError };
// enum ESettingsType { eSerial, eCommon, eUnknown };
// const QMap<ESettingsType, QString> sectionTypes = { { ESettingsType::eSerial, "[serial]"
// },
//                                                    { ESettingsType::eCommon, "[common]" }
//                                                    };

// enum EMode { eClarity, eSms, eUnrecognized };
// const QMap<EMode, QString> modes = { { EMode::eClarity, "sms" }, { EMode::eSms, "clarity"
// } };

// enum EIface { eUndefined, eRs232, eRs485, eRadio };
// const QMap<EIface, QString> ifaces = { { EIface::eRs232, "RS-232" },
//                                       { EIface::eRs485, "RS-485" },
//                                       { EIface::eRadio, "Radio" } };
// struct SSettingsSerial {
//    EIface iface;
//    bool enabled { false };
//    int baudRate { 115200 };
//    int dataBits { 8 };
//    QSerialPort::Parity parity { QSerialPort::NoParity };
//    float stopBits { 1 };
//    // time interval between packet transtittion (ms)
//    int writeDelay { 0 };
//    // max wait time for new info before packet unit will be ended (ms)
//    int waitPacketTime { 0 };
//};

// struct SCommonSettings {
//    EMode mode { eClarity };
//};

#endif // SSETTINGS_H
