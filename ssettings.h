#ifndef SSETTINGS_H
#define SSETTINGS_H
#include <QSerialPort>
#include <QMap>
enum EState { eConnected, eDisconnected, eError };

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

struct SSettings {
    QMap<QString, QString> fields;
    ESettingsType getType() const { return m_type; }

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
