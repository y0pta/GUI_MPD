#include "cmainwindow.h"
#include "ui_cmainwindow.h"
#include <QMessageBox>
#include <QSignalMapper>

CMainWindow::CMainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::CMainWindow)
{
    ui->setupUi(this);

    ///Настраиваем виджет с моделью
    ui->wgt_model->addElements(3, { SERIAL_IFACE_RADIO, SERIAL_IFACE_RS232, SERIAL_IFACE_RS485 });
    connect(ui->wgt_model, &CMpdWidget::s_clicked, this, &CMainWindow::showSettings);
    connect(ui->wgt_model, &CMpdWidget::s_wantChangeState, this,
            &CMainWindow::wantChangeStateReceived);
    /// настройка порта для настройки
    updateAvaliablePorts();
    connect(&m_serial, &CSerialPort::s_avaliablePortsChanged, this,
            &CMainWindow::updateAvaliablePorts);
    m_page = eMainPage;
    setPage(eMainPage);

    /// Назначаем label'ам форм property, чтобы было удобнее заполнять
    ui->lb_parity->setProperty(FIELD_NAME, QString(SERIAL_PARITY));
    ui->lb_baudRate->setProperty(FIELD_NAME, QString(SERIAL_BAUDRATE));
    ui->lb_dataBits->setProperty(FIELD_NAME, QString(SERIAL_DATABITS));
    ui->lb_stopBits->setProperty(FIELD_NAME, QString(SERIAL_STOPBITS));
    ui->lb_writeDelay->setProperty(FIELD_NAME, QString(SERIAL_WRITEDELAY));
    ui->lb_waitPacketTime->setProperty(FIELD_NAME, QString(SERIAL_WAITPACKETTIME));

    /// Внешний вид для теста
    //    ui->wgt_model->changeState(SERIAL_IFACE_RADIO, eConnected);
    //    ui->wgt_model->changeState(SERIAL_IFACE_RS232, eError);

    auto sett = SSerialSection::getSerialDefault(SERIAL_IFACE_RADIO);
    //    sett.fields[SERIAL_BAUDRATE] = "9600";
    //    sett.fields[SERIAL_PARITY] = "odd";
    addSetting(sett);
    addSetting(SSerialSection::getSerialDefault(SERIAL_IFACE_RS232));
    addSetting(SSerialSection::getSerialDefault(SERIAL_IFACE_RS485));

    //    SCommonSection cs;
    //    cs.fields[COMMON_MODE] = COMMON_MODE_VAL[eSms];
    //    addSetting(cs);

    //    SStatSection ss;
    //    ss.fields[STAT_TEXT] = "swdsdasdsad\nsdasdsadsad\n";
    //    addSetting(ss);

    //    setMode(eViewMode);
    //    sett.fields[SERIAL_BAUDRATE] = "1200";
    //    sett.fields[SERIAL_PARITY] = "fgdfg";

    //    loadSettingStatus({ SERIAL_BAUDRATE, SERIAL_DATABITS });
}

CMainWindow::~CMainWindow()
{
    delete ui;
}

void CMainWindow::setPage(EPage page)
{
    ui->gb_settings->setVisible(false);
    ui->lb_status->setVisible(true);
    ui->lb_status->clear();

    if (page == eMainPage) {
        ui->gb_start->setVisible(false);
        ui->gb_mpdModel->setVisible(true);
        ui->pb_finishConfigure->setVisible(true);
        ui->left_tab->setVisible(true);
        ui->lb_mode->setVisible(true);
    }
    if (page == eStartPage) {
        ui->gb_start->setVisible(true);
        ui->left_tab->setVisible(false);
        ui->gb_mpdModel->setVisible(false);
        ui->pb_finishConfigure->setVisible(false);
        ui->lb_mode->setVisible(false);
        showSettings("RS-232");
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
            setCursor(Qt::ArrowCursor);
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

void CMainWindow::on_pb_startConfigure_clicked()
{
    if (!m_serial.openDefault(ui->cb_avaliablePorts->currentText()))
        ui->lb_status->setText("Не удалось открыть порт");
    else {
        connect(&m_serial, &CSerialPort::s_deviceRemoved, this, &CMainWindow::serialRemoved);
        connect(&m_serial, &CSerialPort::s_error, this, &CMainWindow::setError);
        prepareProtocol();
        m_serial.setProtocol(m_protocol);
        m_protocol->getRequest(eSerial);
        m_protocol->getRequest(eCommon);
        m_protocol->getRequest(eStat);

        setPage(eMainPage);
    }
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
    loadSettingsFields(ifaceName);
    if (st == eConnected) {
        setMode(eEditMode);
    } else if (st == eDisconnected) {
        if (m_protocol && m_serial.isOpen()) {
            SSerialSection sett;
            sett.fields[SERIAL_IFACE] = ifaceName;
            sett.fields[SERIAL_STATUS] = "off";

            m_protocol->setRequest(sett);
        }
    }
}

void CMainWindow::addSetting(const SSection &sett)
{
    for (auto it = m_sects.begin(); it != m_sects.end(); it++) {
        if (it->fields.contains(SERIAL_IFACE))
            if (it->fields[SERIAL_IFACE] == sett.fields[SERIAL_IFACE]) {
                m_sects.erase(it);
            }
    }
    m_sects.push_back(sett);
    processSetting(sett);
}

void CMainWindow::processSetting(const SSection &sect)
{
    switch (sect.getType()) {
    case eSerial:
        colorFields({});
        fillSerialFieds(sect);
        break;
    case eCommon:
        ui->lb_mode->setText(sect.fields[COMMON_MODE]);
        if (sect.fields[COMMON_MODE] == COMMON_MODE_VAL[eSms])
            ui->rb_sms->setChecked(true);
        else
            ui->rb_clarity->setChecked(true);
        if (sect.fields[COMMON_DUMPSTATUS] == STATUS_ON)
            ui->cb_dumpOn->setChecked(true);
        else
            ui->cb_dumpOn->setChecked(false);
        break;
    case eStat:
        ui->txt_statistics->append(sect.fields[STAT_TEXT]);
    default:
        break;
    }
}

void CMainWindow::serialRemoved()
{
    m_serial.close();
    setError("Устройство было извлечено!");
    setPage(eStartPage);
    disconnect(&m_serial, &CSerialPort::s_deviceRemoved, this, &CMainWindow::serialRemoved);
    disconnect(&m_serial, &CSerialPort::s_error, this, &CMainWindow::setError);
}

void CMainWindow::setError(QString errorStr)
{
    ui->lb_status->setText(errorStr);
}

void CMainWindow::fillSerialFieds(const SSection &sect)
{
    if (sect.getType() != eSerial)
        return;

    if (!sect.fields[SERIAL_PARITY].isEmpty())
        ui->cb_parity->setCurrentText(sect.fields[SERIAL_PARITY]);
    if (!sect.fields[SERIAL_BAUDRATE].isEmpty())
        ui->cb_baudRate->setCurrentText(sect.fields[SERIAL_BAUDRATE]);
    if (!sect.fields[SERIAL_DATABITS].isEmpty())
        ui->cb_dataBits->setCurrentText(sect.fields[SERIAL_DATABITS]);
    if (!sect.fields[SERIAL_STOPBITS].isEmpty())
        ui->cb_stopBits->setCurrentText(sect.fields[SERIAL_STOPBITS]);
    if (!sect.fields[SERIAL_WRITEDELAY].isEmpty())
        ui->ln_writeDelay->setText(sect.fields[SERIAL_WRITEDELAY]);
    if (!sect.fields[SERIAL_WAITPACKETTIME].isEmpty())
        ui->ln_waitPacketTime->setText(sect.fields[SERIAL_WAITPACKETTIME]);
}

void CMainWindow::colorFields(const QList<QString> &errorNames)
{
    /// Идем по всем полям формы с настройками, подсвечиваем те, которые ошибочные. Обесцвечиваем не
    /// ошибочные
    for (int i = 0; i < ui->fl_unitSettings->rowCount(); i++) {
        auto label = ui->fl_unitSettings->itemAt(i, QFormLayout::ItemRole::LabelRole)->widget();
        auto fieldName = label->property(FIELD_NAME).toString();
        qobject_cast<QLabel *>(label)->setStyleSheet("QLabel { color: black }; ");

        for (auto errorField : errorNames) {
            if (fieldName == errorField)
                qobject_cast<QLabel *>(label)->setStyleSheet("QLabel { color: red }; }");
        }
    }
}

void CMainWindow::setSection(const SSection &sect)
{
    addSetting(sect);
    setMode(m_mode);
}

void CMainWindow::setDebugInfo(const QString &str)
{
    ui->txt_debug->append(str);
}

void CMainWindow::requestConfirmed(const SSection &sect)
{
    addSetting(sect);
    setMode(m_mode);
    if (sect.getType() == eSerial)
        ui->wgt_model->changeState(sect.fields[SERIAL_IFACE],
                                   STATE_TYPES.key(sect.fields[SERIAL_STATUS]));
}

void CMainWindow::requestFailed(const SSection &sect)
{
    if (sect.getType() == eStat) {
        ui->txt_statistics->setText("Время ожидания подтверждения превышено");
    } else
        ui->lb_status->setText("Время ожидания подтверждения превышено");
}

void CMainWindow::serviceRequestFailed(QString name)
{
    // ui->lb_status->setText("Запрос " + sect.fields[] + " не выполнен");
}

void CMainWindow::serviceRequestConfirmed(QString name)
{
    ui->lb_status->setText("Запрос " + name + " выполнен");
}

void CMainWindow::requestError(const QList<QString> &errorFields)
{
    colorFields(errorFields);
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
        setPage(eStartPage);

        disconnect(&m_serial, &CSerialPort::s_deviceRemoved, this, &CMainWindow::serialRemoved);
        disconnect(&m_serial, &CSerialPort::s_error, this, &CMainWindow::setError);
    }
    delete box;
}

void CMainWindow::on_pb_cancel_clicked()
{
    ui->wgt_model->unfreezeAll();
    setMode(eHidden);
}

void CMainWindow::loadSettingsFields(QString nameElement)
{
    SSection sett = SSerialSection::getSerialDefault();
    for (auto el : m_sects) {
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

void CMainWindow::loadSettingStatus(const QList<QString> &errorFields)
{
    // Идем по всем полям формы, подсвечиваем те, которые ошибочные
    for (int i = 0; i < ui->fl_unitSettings->rowCount(); i++) {
        auto label = ui->fl_unitSettings->itemAt(i, QFormLayout::ItemRole::LabelRole)->widget();
        auto fieldName = label->property(FIELD_NAME).toString();
        qobject_cast<QLabel *>(label)->setStyleSheet("QLabel { color: black }; ");

        for (auto errorField : errorFields) {
            if (fieldName == errorField)
                qobject_cast<QLabel *>(label)->setStyleSheet("QLabel { color: red }; }");
        }
    }
}

void CMainWindow::prepareProtocol()
{
    if (!m_protocol)
        m_protocol = new CProtocolTransmitter(this);

    connect(m_protocol, &CProtocolTransmitter::s_sectionRead, this, &CMainWindow::setSection);
    connect(m_protocol, &CProtocolTransmitter::s_debugInfo, this, &CMainWindow::setDebugInfo);
    connect(m_protocol, &CProtocolTransmitter::s_requestError, this, &CMainWindow::requestError);
    connect(m_protocol, &CProtocolTransmitter::s_requestFailed, this, &CMainWindow::requestFailed);
    connect(m_protocol, &CProtocolTransmitter::s_requestConfirmed, this,
            &CMainWindow::requestConfirmed);
    connect(m_protocol, &CProtocolTransmitter::s_serviceRequestFailed, this,
            &CMainWindow::serviceRequestFailed);
    connect(m_protocol, &CProtocolTransmitter::s_serviceRequestConfirmed, this,
            &CMainWindow::serviceRequestConfirmed);
}

void CMainWindow::showSettings(const QString &nameEl)
{
    if (m_mode != eEditMode) {
        loadSettingsFields(nameEl);
        setMode(eViewMode);
        colorFields({});
    }
}

void CMainWindow::on_pb_accept_clicked()
{
    if (!m_protocol || !m_serial.isOpen())
        return;

    setMode(eWaitMode);
    ui->wgt_model->freezeExcept();
    SSerialSection sett;
    QString iface = ui->lb_ifaceName->text();
    sett.fields[SERIAL_IFACE] = iface;
    sett.fields[SERIAL_PARITY] = ui->cb_parity->currentText();
    sett.fields[SERIAL_BAUDRATE] = ui->cb_baudRate->currentText();
    sett.fields[SERIAL_DATABITS] = ui->cb_dataBits->currentText();
    sett.fields[SERIAL_STOPBITS] = ui->cb_stopBits->currentText();
    sett.fields[SERIAL_WRITEDELAY] = ui->ln_writeDelay->text();
    sett.fields[SERIAL_WAITPACKETTIME] = ui->ln_waitPacketTime->text();

    m_protocol->setRequest(sett);
}

void CMainWindow::on_pb_refreshStat_clicked()
{
    if (m_protocol && m_serial.isOpen())
        m_protocol->getRequest(eStat);
    ui->txt_statistics->clear();
}

void CMainWindow::on_pb_resetStat_clicked()
{
    if (m_protocol && m_serial.isOpen())
        m_protocol->serviceRequest(eClearStat);
    ui->txt_statistics->clear();
}

void CMainWindow::on_pb_applyMode_clicked()
{
    if (m_protocol && m_serial.isOpen()) {
        SCommonSection sect;
        if (ui->rb_sms->isChecked())
            sect.fields[COMMON_MODE] = COMMON_MODE_VAL[eSms];
        if (ui->rb_clarity->isChecked())
            sect.fields[COMMON_MODE] = COMMON_MODE_VAL[eClarity];
        m_protocol->setRequest(sect);
    }
}

void CMainWindow::on_cb_dumpOn_stateChanged(int arg1)
{
    if (m_protocol && m_serial.isOpen()) {
        SCommonSection sect;
        if (ui->cb_dumpOn->checkState() == Qt::Checked)
            sect.fields[COMMON_DUMPSTATUS] = STATUS_ON;
        else
            sect.fields[COMMON_DUMPSTATUS] = STATUS_OFF;
        m_protocol->setRequest(sect);
    }
}

void CMainWindow::on_pb_clearErrors_clicked()
{
    if (m_protocol && m_serial.isOpen())
        m_protocol->serviceRequest(eClearErrors);
}

void CMainWindow::on_pb_clearFlash_clicked()
{
    qDebug() << "Clear Flash clicked";
    if (m_protocol && m_serial.isOpen()) {
        m_protocol->serviceRequest(eClearFlash);
        qDebug() << "Inside clear flash";
    }
}

void CMainWindow::on_pb_clearDebug_clicked()
{
    ui->txt_debug->clear();
}
