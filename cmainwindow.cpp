#include "cmainwindow.h"
#include "ui_cmainwindow.h"

CMainWindow::CMainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::CMainWindow)
{
    ui->setupUi(this);
    updateAvaliablePorts(CSerialPort::getAvaliable());
    connect(&m_serial, &CSerialPort::s_avaliablePortsChanged, this,
            &CMainWindow::updateAvaliablePorts);
    setView(eStartPage);
}

CMainWindow::~CMainWindow()
{
    delete ui;
}

void CMainWindow::setView(EPage page)
{
    ui->gb_settings->setVisible(false);
    ui->lb_status->setVisible(true);
    ui->lb_status->clear();

    if (page == eMainPage) {
        ui->gb_start->setVisible(false);
        ui->gb_model->setVisible(true);
        ui->pb_finishConfigure->setVisible(true);
    }
    if (page == eStartPage) {
        ui->gb_start->setVisible(true);
        ui->gb_model->setVisible(false);
        ui->pb_finishConfigure->setVisible(false);
    }
}

void CMainWindow::setView(EMode mode)
{
    switch (mode) {

        break;
    case eEditMode:
        break;
    case eWaitMode:
        break;
    default:
        break;
    }
    ui->gb_start->setVisible(true);

    ui->gb_settings->setVisible(false);
    ui->lb_status->setVisible(true);

    ui->pb_finishConfigure->setVisible(false);
    ui->pb_mpd->setVisible(false);
    ui->pb_radio->setVisible(false);
    ui->pb_rs232->setVisible(false);
    ui->pb_rs485->setVisible(false);

    ui->lb_baudRate;
    ui->lb_dataBits;
    ui->lb_parity;
    ui->lb_stopBits;
    ui->lb_writeDelay;
    ui->lb_waitPacketTime;

    // ui->lb_avaliablePorts;
}

void CMainWindow::updateAvaliablePorts(QList<QString> portNames)
{
    ui->cb_avaliablePorts->clear();
    for (auto portName : portNames)
        ui->cb_avaliablePorts->addItem(portName);
}

void CMainWindow::on_pb_radio_clicked()
{

    ui->gb_settings->setVisible(true);
}

void CMainWindow::on_pb_startConfigure_clicked()
{
    if (!m_serial.openDefault(ui->cb_avaliablePorts->currentText()))
        ui->lb_status->setText("Не удалось открыть порт");
    else
        setView(eMainPage);
}
