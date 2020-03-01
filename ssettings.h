#ifndef SSETTINGS_H
#define SSETTINGS_H
#include <QSerialPort>

struct SCmdTemp {
    enum ESection { eNone, eRs232, eRs485, eRadio };
    enum EMode { eClarity, eSms };
    ESection section { eNone };
    EMode mode { eClarity };
};

struct SSettingsSerial : SCmdTemp {
    bool enabled { false };
    QSerialPort::BaudRate baudRate { QSerialPort::Baud115200 };
    QSerialPort::DataBits bataBits { QSerialPort::Data8 };
    QSerialPort::Parity parity { QSerialPort::NoParity };
    QSerialPort::StopBits stopBits { QSerialPort::OneStop };
    // time interval between packet transtittion (ms)
    int writeDelay;
    // max wait time for new info before packet unit will be ended (ms)
    int waitPacketTime;
};

struct SGetSettings : SCmdTemp {
};

#endif // SSETTINGS_H
