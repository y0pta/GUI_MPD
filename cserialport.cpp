#include "cserialport.h"
#include <QSerialPortInfo>

CSerialPort::CSerialPort(QObject *parent)
{
    m_avaliablePorts = getAvaliable();
    connect(&m_timeToCheckPorts, &QTimer::timeout, this, &CSerialPort::checkAvaliablePorts);
}

QList<QString> CSerialPort::getAvaliable()
{
    auto ports = QSerialPortInfo::availablePorts();
    QList<QString> res;
    for (auto port : ports)
        res.append(port.portName());
    return res;
}

void CSerialPort::checkAvaliablePorts()
{
    auto curPorts = getAvaliable();
    for (auto prevPort : m_avaliablePorts) {
        if (curPorts.indexOf(prevPort) == -1) {
            m_avaliablePorts = curPorts;
            emit s_avaliablePortsChanged(curPorts);
            break;
        }
    }
}

bool CSerialPort::openDefault(QString name) {}

QByteArray CSerialPort::readyRead()
{
    auto data = m_port.readAll();
    emit s_readyRead(data);
    return data;
}
