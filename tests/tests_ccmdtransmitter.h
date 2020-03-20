#include <ccmdtransmitter.h>
#include <QFile>

void testRequestSettings()
{
    QFile file("/home/liza/test");
    file.open(QIODevice::WriteOnly);
    CCmdTransmitter::requestSettings(&file, ESettingsType::eSerial,
                                     QPair<QString, QString>("iface", "RS-232"));
    CCmdTransmitter::requestSettings(&file, ESettingsType::eCommon);
    file.close();
}

void testWriteSettings()
{
    QFile file("/home/liza/test");
    file.open(QIODevice::ReadWrite);

    SCommonSettings gsett;
    gsett.fields[COMMON_MODE] = "sms";
    SSettingsSerial sett;
    sett.fields[SERIAL_IFACE] = SERIAL_IFACE_RADIO;
    sett.fields[SERIAL_PARITY] = "odd";
    sett.fields[SERIAL_BAUDRATE] = "115200";
    CCmdTransmitter::sendSettings(&file, gsett);
    CCmdTransmitter::sendSettings(&file, sett);
    file.close();
}

void testReadSettings()
{
    CCmdTransmitter cmd;
    QFile file("/home/liza/test");
    file.open(QIODevice::WriteOnly);
    file.write("[end]\n"
               "[serial]\n "
               " iface: RS - 232\n "
               "status: on\n"
               "baudRate: 9600\n"
               "dataBits: 8\n"
               "parity: odd\n"
               "stopBits: 1.0\n"
               "delay: 5\n"
               "[end]\n"
               "[common]\n"
               "mode: sms\n[end]");
    file.close();
    file.open(QIODevice::ReadOnly);
    auto res = cmd.readSettings(&file);
    file.close();
}
