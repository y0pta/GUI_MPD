#include "cmainwindow.h"
#include "ui_cmainwindow.h"
#include <QMessageBox>

CMainWindow::CMainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::CMainWindow)
{
    ui->setupUi(this);
    setStyleSheet("QMainWindow { background-color: #9C9C9C; }\n QPushButton { background-color: "
                  "#979797; }\n ");
    ui->gb_mpdModel->setStyleSheet("QGroupBox {background-color: #A7A7A7; border: solid; }");
    //Настраиваем виджет с моделью
    ui->wgt_model->addElements(3, { SERIAL_IFACE_RADIO, SERIAL_IFACE_RS232, SERIAL_IFACE_RS485 });
    auto sett = SSettingsSerial::getSerialDefault(SERIAL_IFACE_RADIO);
    sett.fields[SERIAL_BAUDRATE] = "9600";
    sett.fields[SERIAL_PARITY] = "odd";
    m_setts.append(sett);
    m_setts.append(SSettingsSerial::getSerialDefault(SERIAL_IFACE_RS232));
    m_setts.append(SSettingsSerial::getSerialDefault(SERIAL_IFACE_RS485));
    connect(ui->wgt_model, &CMpdWidget::s_clicked, this, &CMainWindow::on_wgt_model_clicked);
    connect(ui->wgt_model, &CMpdWidget::s_wantChangeState, this,
            &CMainWindow::wantChangeStateReceived);

    updateAvaliablePorts();
    m_page = eMainPage;
    setView(eMainPage);
    ui->wgt_model->changeState(SERIAL_IFACE_RADIO, eConnected);
    ui->wgt_model->changeState(SERIAL_IFACE_RS232, eError);

    connect(&m_serial, &CSerialPort::s_avaliablePortsChanged, this,
            &CMainWindow::updateAvaliablePorts);

    m_serial.requestData(eSerial);
    //    ui->wgt_model->s_clicked("Rs-232");
    //    setMode(ESettingsMode::eViewMode);
}

CMainWindow::~CMainWindow()
{
    delete ui;
}

void CMainWindow::on_wgt_model_clicked(QString nameEl)
{
    if (m_mode != eEditMode) {
        loadSettings(nameEl);
        setMode(eViewMode);
    }
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
    if (m_page == eMainPage) {
        if (mode == eViewMode) {
            ui->pb_edit->setVisible(true);
            ui->pb_accept->setVisible(false);
            ui->pb_cancel->setVisible(false);

            ui->gb_settings->setVisible(true);
            ui->pb_closeSettings->setEnabled(true);

            enableSettings(false);
            setCursor(Qt::ArrowCursor);
        } else if (mode == eEditMode) {
            ui->pb_edit->setVisible(false);
            ui->pb_accept->setVisible(true);
            ui->pb_cancel->setVisible(true);

            ui->gb_settings->setVisible(true);
            ui->pb_closeSettings->setEnabled(true);

            enableSettings(true);
            setCursor(Qt::ArrowCursor);

        } else if (mode == eWaitMode) {
            ui->pb_edit->setVisible(false);
            ui->pb_accept->setVisible(false);
            ui->pb_cancel->setVisible(false);

            ui->gb_settings->setVisible(true);
            ui->pb_closeSettings->setEnabled(false);

            enableSettings(false);
            setCursor(Qt::WaitCursor);
        } else if (mode == eHidden) {
            ui->gb_settings->setVisible(false);
        }
        m_mode = mode;
    }
}

void CMainWindow::updateAvaliablePorts()
{
    if (m_page == eStartPage) {
        //очищаем все
        ui->cb_avaliablePorts->clear();
        // добавляем новые порты
        for (auto portName : CSerialPort::avaliablePorts())
            ui->cb_avaliablePorts->addItem(portName);

        bool enable = true;
        // если не оказалось доступных портов, замораживаем кнопку подключения и комбо-бокс, выводим
        // подсказку
        if (ui->cb_avaliablePorts->count() == 0) {
            enable = false;
            ui->lb_status->setText("Нет доступных портов");
        }

        ui->pb_startConfigure->setEnabled(enable);
        ui->cb_avaliablePorts->setEnabled(enable);
    }
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
    ui->wgt_model->unfreezeAll();
    setMode(eHidden);
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
    ui->wgt_model->freezeExcept(ui->lb_ifaceName->text());
    setMode(eEditMode);
}

void CMainWindow::wantChangeStateReceived(QString ifaceName, EState st)
{
    loadSettings(ifaceName);
    if (st == eConnected) {
        setMode(eEditMode);
    } else if (st == eDisconnected) {
    }
}

void CMainWindow::readyRead()
{ // TODO: сделать обработчик настроек
    auto setts = m_serial.readAllSettings();
    for (auto sett : setts) {
        if (sett.getType() == eCommon) {

        } else if (sett.getType() == eSerial) {
        }
    }
}

void CMainWindow::on_pb_finishConfigure_clicked()
{
    QMessageBox *box =
            new QMessageBox(QMessageBox::Warning, "Завершить настройку МПД", DIALOG_FINISH_EDITING);
    auto yes = box->addButton("Да", QMessageBox::AcceptRole);
    box->addButton("Отмена", QMessageBox::RejectRole);
    box->exec();

    if (box->clickedButton() == yes) {
        m_serial.close();
        setView(eStartPage);
    }
    delete box;
}

void CMainWindow::on_pb_cancel_clicked()
{
    ui->wgt_model->unfreezeAll();
    setMode(eHidden);
}

void CMainWindow::loadSettings(QString nameElement)
{
    SSettings sett = SSettingsSerial::getSerialDefault();
    for (auto el : m_setts) {
        if (el.fields.count(SERIAL_IFACE))
            if (el.fields[SERIAL_IFACE] == nameElement) {
                sett = el;
                ui->lb_ifaceName->setText(nameElement);
            }
    }
    ui->cb_parity->setCurrentText(sett.fields[SERIAL_PARITY]);
    ui->cb_baudRate->setCurrentText(sett.fields[SERIAL_BAUDRATE]);
    ui->cb_dataBits->setCurrentText(sett.fields[SERIAL_DATABITS]);
    ui->cb_stopBits->setCurrentText(sett.fields[SERIAL_STOPBITS]);
    ui->ln_writeDelay->setText(sett.fields[SERIAL_WRITEDELAY]);
    ui->ln_waitPacketTime->setText(sett.fields[SERIAL_WAITPACKETTIME]);
}

void CMainWindow::on_pb_accept_clicked()
{
    ui->wgt_model->freezeExcept();
    SSettingsSerial sett;
    sett.fields[SERIAL_PARITY] = ui->cb_parity->currentText();
    sett.fields[SERIAL_BAUDRATE] = ui->cb_baudRate->currentText();
    sett.fields[SERIAL_DATABITS] = ui->cb_dataBits->currentText();
    sett.fields[SERIAL_STOPBITS] = ui->cb_stopBits->currentText();
    sett.fields[SERIAL_WRITEDELAY] = ui->ln_writeDelay->text();
    sett.fields[SERIAL_WAITPACKETTIME] = ui->ln_waitPacketTime->text();
    m_serial.sendData(sett);
    setMode(eWaitMode);
}
