#ifndef SSETTINGS_H
#define SSETTINGS_H
#include <QMap>
#include <QDebug>
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

const QString COMMON_MODE = "mode";
const QString COMMON_DUMPSTATUS = "dumpStatus";
enum eCommonMode { eSms, eClarity };
const QMap<eCommonMode, QString> COMMON_MODE_VAL { { eSms, "sms" }, { eClarity, "clarity" } };

const QString DEBUG_CLEARERRORS = "clearErrors";
const QString DEBUG_CLEARSTAT = "clearStat";
const QString DEBUG_CLEARFLASH = "clearFlash";

const QString STAT_TEXT = "text";

const QString STATUS_ON = "on";
const QString STATUS_OFF = "off";

const char FIELD_NAME[] = "name";

enum EState { eConnected, eDisconnected, eError, eDisabled };
const QMap<EState, QString> STATE_TYPES = {
    { eConnected, "on" },
    { eDisabled, "off" },
};
enum ESectionType { eNoSection = -1, eSerial, eCommon, eStat, eDebug };
const QMap<ESectionType, QString> SECTION_TYPES = { { ESectionType::eSerial, "[serial]" },
                                                    { ESectionType::eCommon, "[common]" },
                                                    { ESectionType::eStat, "[stat]" },
                                                    { ESectionType::eDebug, "[debug]" } };

static QString getStr(ESectionType type)
{
    return SECTION_TYPES[type];
}

static ESectionType getSectionType(QString nameSection)
{
    for (auto it = SECTION_TYPES.begin(); it != SECTION_TYPES.end(); it++) {
        if (it.value() == nameSection)
            return it.key();
    }
    return eNoSection;
}

enum EServiceCommand { eClearErrors, eClearStat, eClearFlash };

const QMap<EServiceCommand, QString> SERVICE_COMMAND = {
    { eClearErrors, DEBUG_CLEARERRORS },
    { eClearStat, DEBUG_CLEARSTAT },
    { eClearFlash, DEBUG_CLEARFLASH },
};

const QString SECTION_INTERRUPTOR = "[end]";
const QString SECTIONVAL_CONFIRMED = "<ok>";
const QString SECTIONVAL_ERROR = "error";

struct SSection {
    QMap<QString, QString> fields;

    ///определяет тип настроек
    static ESectionType getType(const SSection &sect) { return sect.getType(); }
    /// определяет, является подтверждением
    static bool isConfirmation(const SSection &sect);
    /// возвращает поля ошибок
    static QList<QString> getErrorFields(const SSection &sect);
    /// если значения всех полей пусты, возвращает true
    static bool isEmpty(const SSection &sect);
    ESectionType getType() const { return m_type; }

protected:
    ESectionType m_type { eNoSection };
};

struct SSerialSection : public SSection {
    SSerialSection()
    {
        m_type = eSerial;
        fields = { { SERIAL_IFACE, "" },      { SERIAL_STATUS, "" },        { SERIAL_BAUDRATE, "" },
                   { SERIAL_DATABITS, "" },   { SERIAL_PARITY, "" },        { SERIAL_STOPBITS, "" },
                   { SERIAL_WRITEDELAY, "" }, { SERIAL_WAITPACKETTIME, "" } };
    }
    static SSerialSection getSerialDefault(QString ifaceName = QString())
    {
        SSerialSection sett;
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

struct SCommonSection : public SSection {
    SCommonSection()
    {
        m_type = eCommon;
        fields = { { COMMON_MODE, "" }, { COMMON_DUMPSTATUS, "" } };
    }
};

struct SDebugSection : public SSection {
    SDebugSection()
    {
        m_type = eDebug;
        fields = { { DEBUG_CLEARSTAT, "" }, { DEBUG_CLEARFLASH, "" }, { DEBUG_CLEARERRORS, "" } };
    }
};

struct SStatSection : public SSection {
    SStatSection()
    {
        m_type = eStat;
        fields = { { STAT_TEXT, "" } };
    }
};

static QDebug &operator<<(QDebug &debug, const SSection &sect)
{
    debug << getStr(sect.getType()) << endl;
    for (auto it = sect.fields.begin(); it != sect.fields.end(); it++) {
        if (it.value().size() > 0)
            debug << it.key() << ": " << it.value() << endl;
    }
    return debug;
}

#endif // SSETTINGS_H
