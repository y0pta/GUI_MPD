#ifndef CCMDCONVENTER_H
#define CCMDCONVENTER_H

#include <QObject>
#include "ssettings.h"

class CCmdTransmitter
{
public:
    CCmdTransmitter() {}
    ~CCmdTransmitter() {}

    static QList<SSettings> readSettings(QIODevice *dev);
    static void sendSettings(QIODevice *dev, const SSettings &sett);
    static void
    requestSettings(QIODevice *dev, ESettingsType type,
                    const QPair<QString, QString> &parameter = QPair<QString, QString>());

protected:
    template<typename SSettingsClass>
    static void fillSettings(const QByteArray &rawSett, SSettingsClass &sett);

private:
    static ESettingsType readNextSectionType(const QByteArray &data);
    static SSettings readNextSettings(QIODevice *dev);
};

#endif // CCMDCONVENTER_H
