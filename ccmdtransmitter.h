#ifndef CCMDCONVENTER_H
#define CCMDCONVENTER_H

#include <QObject>
#include "ssettings.h"

class CCmdTransmitter
{
public:
    CCmdTransmitter() {}
    ~CCmdTransmitter() {}

    static QList<SSettings> readSettings(QIODevice &dev);
    static void sendSettings(QIODevice &dev, const SSettings &sett);
    static void
    requestSettings(QIODevice &dev, ESettingsType type,
                    const QPair<QString, QString> &parameter = QPair<QString, QString>());

protected:
    template<typename SSettingsClass>
    static void fillSettings(const QByteArray &rawSett, SSettingsClass &sett);

private:
    static ESettingsType readNextSectionType(const QByteArray &data);
    static SSettings readNextSettings(QIODevice &dev);
};

// class CCmdTransmitter
//{

// public:
//    CCmdTransmitter() {}
//    ~CCmdTransmitter() {}
//    static ESettingsType getNextSectionType(QIODevice &dev);
//    SSettingsSerial &getSettings(QIODevice &dev, SSettingsSerial &sett);
//    SCommonSettings &getSettings(QIODevice &dev, SCommonSettings &sett);
//    static void sendSettings(QIODevice &dev, const SSettingsSerial &sett);
//    static void sendSettings(QIODevice &dev, const SCommonSettings &sett);
//    static void sendSettingsRequest();

//    QString getLastError();
// signals:
//    void s_error();

// private:
//    QString m_error;
//};

#endif // CCMDCONVENTER_H
