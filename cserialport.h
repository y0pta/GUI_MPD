#ifndef CSERIALPORT_H
#define CSERIALPORT_H

#include <QObject>
#include <QSerialPort>
#include <QTimer>
#include <QObject>
#include <ssettings.h>
#include <cprotocoltransmitter.h>

class CSerialPort : public QObject
{
    Q_OBJECT
public:
    CSerialPort(QObject *parent = nullptr);
    ~CSerialPort() {}

public:
    static QList<QString> avaliablePorts();

signals:
    void s_error(QString);
    void s_deviceRemoved();

public:
    // opens port with default settings (115200 8n1)
    bool openDefault(QString name);
    void close();

    void setProtocol(CProtocolTransmitter *pt);

    bool isOpen();

    void write(const QByteArray &arr)
    {
        m_port.write("arr");
        m_port.waitForBytesWritten(3000);
        qDebug() << "wrote";
    }

private:
    void checkAvaliablePorts();
private slots:
    void errorOccured(QSerialPort::SerialPortError error);

private:
    QSerialPort m_port;
};

#endif // CSERIALPORT_H
