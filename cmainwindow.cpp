#include "cmainwindow.h"
#include "ui_cmainwindow.h"

CMainWindow::CMainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::CMainWindow)
{
    ui->setupUi(this);
    ui->wgt_model->addElements(3, QList<QString>() = { "Radio", "Rs-232", "Rs-484" });
    connect(ui->wgt_model, &CMpdWidget::s_clicked, this, &CMainWindow::on_wgt_model_clicked);
    connect(ui->wgt_model, &CMpdWidget::s_wantChangeState, this,
            &CMainWindow::wantChangeStateReceived);

    updateAvaliablePorts(CSerialPort::getAvaliable());
    connect(&m_serial, &CSerialPort::s_avaliablePortsChanged, this,
            &CMainWindow::updateAvaliablePorts);
    setView(eMainPage);

    ui->wgt_model->s_clicked("Rs-232");
    setMode(ESettingsMode::eViewMode);
}

CMainWindow::~CMainWindow()
{
    delete ui;
}

void CMainWindow::on_wgt_model_clicked(QString nameEl)
{
    //    ui->gb_settings->setVisible(true);
    //    ui->pb_edit->setVisible(true);
    //    ui->pb_accept->setVisible(false);
    //    ui->pb_cancel->setVisible(false);

    //    enableSettings(false);
}

void CMainWindow::setView(EPage page)
{
    ui->gb_settings->setVisible(false);
    ui->lb_status->setVisible(true);
    ui->lb_status->clear();

    if (page == eMainPage) {
        ui->gb_start->setVisible(false);
        ui->gb_mpdModel->setVisible(true);
        ui->pb_finishConfigure->setVisible(true);
    }
    if (page == eStartPage) {
        ui->gb_start->setVisible(true);
        ui->gb_mpdModel->setVisible(false);
        ui->pb_finishConfigure->setVisible(false);
    }
}

void CMainWindow::setMode(ESettingsMode mode)
{
    if (mode == eViewMode) {
        ui->pb_edit->setVisible(true);
        ui->pb_accept->setVisible(false);
        ui->pb_cancel->setVisible(false);

        ui->wgt_model->setEnabled(true);
        ui->gb_settings->setVisible(true);
        ui->pb_closeSettings->setEnabled(true);

        enableSettings(false);
        ui->wgt_model->setEnabled(true);
        setCursor(Qt::ArrowCursor);
    } else if (mode == eEditMode) {
        ui->pb_edit->setVisible(false);
        ui->pb_accept->setVisible(true);
        ui->pb_cancel->setVisible(true);

        ui->wgt_model->setEnabled(false);
        ui->gb_settings->setVisible(true);
        ui->pb_closeSettings->setEnabled(true);

        enableSettings(true);
        ui->wgt_model->setEnabled(false);
        setCursor(Qt::ArrowCursor);

    } else if (mode == eWaitMode) {
        ui->pb_edit->setVisible(false);
        ui->pb_accept->setVisible(false);
        ui->pb_cancel->setVisible(false);

        ui->wgt_model->setEnabled(false);
        ui->gb_settings->setVisible(true);
        ui->pb_closeSettings->setEnabled(false);

        enableSettings(false);
        ui->wgt_model->setEnabled(false);
        setCursor(Qt::WaitCursor);
    }
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

void CMainWindow::on_pb_closeSettings_clicked()
{
    ui->gb_settings->setVisible(false);
}

void CMainWindow::enableSettings(bool st)
{
    ui->lb_parity->setEnabled(st);
    ui->lb_baudRate->setEnabled(st);
    ui->lb_dataBits->setEnabled(st);
    ui->lb_stopBits->setEnabled(st);
    ui->lb_writeDelay->setEnabled(st);
    ui->lb_waitPacketTime->setEnabled(st);

    ui->cb_parity->setEnabled(st);
    ui->cb_baudRate->setEnabled(st);
    ui->cb_dataBits->setEnabled(st);
    ui->cb_stopBits->setEnabled(st);
    ui->ln_writeDelay->setEnabled(st);
    ui->ln_waitPacketTime->setEnabled(st);
}

void CMainWindow::on_pb_edit_clicked()
{
    ui->pb_edit->setVisible(false);
    ui->pb_accept->setVisible(true);
    ui->pb_cancel->setVisible(true);

    enableSettings(true);
}

void CMainWindow::wantChangeStateReceived(QString ifaceName, EState st)
{
    //    if (ifaceName == "RS-232") {
    //    } else if (ifaceName == "RS-485") {
    //    }
    ui->gb_settings->setVisible(true);
    ui->pb_edit->setVisible(false);
    ui->pb_accept->setVisible(true);
    ui->pb_cancel->setVisible(true);

    enableSettings(true);
}
