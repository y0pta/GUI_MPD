#ifndef CCMDCONVENTER_H
#define CCMDCONVENTER_H

#include <QObject>
#include "ssettings.h"

class CCmdConventer
{
public:
    CCmdConventer();
    static SCmdTemp fromSerial(QByteArray data);
    static QByteArray toSerial(SCmdTemp data);
};

#endif // CCMDCONVENTER_H
