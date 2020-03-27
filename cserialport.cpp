#include "cserialport.h"
#include <QSerialPortInfo>
#include <ccmdtransmitter.h>

CSerialPort::CSerialPort(QObject *parent)
{
    m_avaliablePorts = avaliablePorts();
    m_timeToCheckPorts.start(2000);
    connect(&m_timeToCheckPorts, &QTimer::timeout, this, &CSerialPort::checkAvaliablePorts);
}

QList<QString> CSerialPort::avaliablePorts()
{
    auto ports = QSerialPortInfo::availablePorts();
    QList<QString> res;
    for (auto port : ports)
        if (!port.isBusy())
            res.append(port.portName());
    return res;
}

void CSerialPort::checkAvaliablePorts()
{
    auto curPorts = avaliablePorts();
    for (auto prevPort : m_avaliablePorts) {
        if (curPorts.indexOf(prevPort) == -1) {
            m_avaliablePorts = curPorts;
            break;
        }
    }
}

void CSerialPort::errorOccured(QSerialPort::SerialPortError error)
{
    if (error == QSerialPort::ResourceError)
        emit s_deviceRemoved();
    else
        emit s_error(m_port.errorString());
}

bool CSerialPort::openDefault(QString name)
{
    m_port.setPortName(name);
    m_port.setBaudRate(QSerialPort::Baud115200);
    m_port.setParity(QSerialPort::NoParity);
    m_port.setDataBits(QSerialPort::Data8);

    if (!m_port.open(QIODevice::ReadWrite)) {
        emit s_error(m_port.errorString());
        return false;
    }
    connect(&m_port, &QSerialPort::readyRead, this, &CSerialPort::s_readyRead);
    connect(&m_port, &QSerialPort::errorOccurred, this, &CSerialPort::errorOccured);
    return true;
}

void CSerialPort::close()
{
    m_port.close();
}

QList<SSettings> CSerialPort::readAllSettings()
{
    return CCmdTransmitter::readSettings(&m_port);
}

QByteArray CSerialPort::readAllRaw()
{
    return m_port.readAll();
}

void CSerialPort::sendData(const SSettings &data)
{
    CCmdTransmitter::sendSettings(&m_port, data);
}

void CSerialPort::sendData(const QByteArray &data)
{
    m_port.write(data);
}

void CSerialPort::requestData(ESettingsType type, const QPair<QString, QString> &parameter)
{
    CCmdTransmitter::requestSettings(&m_port, type, parameter);
}
