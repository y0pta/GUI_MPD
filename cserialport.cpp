#include "cserialport.h"
#include <QSerialPortInfo>
#include <cprotocoltransmitter.h>

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
    connect(&m_port, &QSerialPort::errorOccurred, this, &CSerialPort::errorOccured);

    return true;
}

void CSerialPort::close()
{
    m_port.close();
}

void CSerialPort::setProtocol(CProtocolTransmitter *pt)
{
    pt->setDevice(&m_port);
}

bool CSerialPort::isOpen()
{
    return m_port.isOpen();
}
