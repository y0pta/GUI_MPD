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
    Q_PROPERTY(QList<QString> avaliablePorts MEMBER m_avaliablePorts NOTIFY s_avaliablePortsChanged)
public:
    CSerialPort(QObject *parent = nullptr);
    ~CSerialPort() {}

public:
    static QList<QString> avaliablePorts();

signals:
    void s_readyRead();
    void s_error(QString);
    void s_deviceRemoved();
    void s_disconnected();
    void s_connected();
    void s_avaliablePortsChanged();

public:
    // opens port with default settings (115200 8n1)
    bool openDefault(QString name);
    void close();
    QByteArray readAllRaw();

private:
    void checkAvaliablePorts();
private slots:
    void errorOccured(QSerialPort::SerialPortError error);

private:
    QSerialPort m_port;
    CProtocolTransmitter transmitter;
    QTimer m_timeToCheckPorts;
    QList<QString> m_avaliablePorts;
};

#endif // CSERIALPORT_H
