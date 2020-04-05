#include "cmainwindow.h"
#include "ui_cmainwindow.h"
#include <QMessageBox>

CMainWindow::CMainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::CMainWindow)
{
    ui->setupUi(this);
    //    setStyleSheet(
    //            "QMainWindow { background-color: #9C9C9C; }\n QPushButton { background-color: "
    //            "#979797; }\n QComboBox { color: white; background-color: #676767}\n QLineEdit { "
    //            "background-color: #676767 }\n QMenu {color: white; background-color:
    //            #676767}\n");
    ui->gb_mpdModel->setStyleSheet(
            "QGroupBox {color: white; background-color: #A7A7A7; border: solid; }");
    //Настраиваем виджет с моделью
    ui->wgt_model->addElements(3, { SERIAL_IFACE_RADIO, SERIAL_IFACE_RS232, SERIAL_IFACE_RS485 });
    auto sett = SSerialSection::getSerialDefault(SERIAL_IFACE_RADIO);
    sett.fields[SERIAL_BAUDRATE] = "9600";
    sett.fields[SERIAL_PARITY] = "odd";
    m_sects.append(sett);
    m_sects.append(SSerialSection::getSerialDefault(SERIAL_IFACE_RS232));
    m_sects.append(SSerialSection::getSerialDefault(SERIAL_IFACE_RS485));
    connect(ui->wgt_model, &CMpdWidget::s_clicked, this, &CMainWindow::on_wgt_model_clicked);
    connect(ui->wgt_model, &CMpdWidget::s_wantChangeState, this,
            &CMainWindow::wantChangeStateReceived);

    updateAvaliablePorts();
    m_page = eMainPage;
    setPage(eMainPage);
    ui->wgt_model->changeState(SERIAL_IFACE_RADIO, eConnected);
    ui->wgt_model->changeState(SERIAL_IFACE_RS232, eError);

    connect(&m_serial, &CSerialPort::s_avaliablePortsChanged, this,
            &CMainWindow::updateAvaliablePorts);

    //    ui->wgt_model->s_clicked("Rs-232");
    //    setMode(ESettingsMode::eViewMode);

    // user data
    QString str = SERIAL_PARITY;

    ui->lb_parity->setProperty(FIELD_NAME, QString(SERIAL_PARITY));
    ui->lb_baudRate->setProperty(FIELD_NAME, QString(SERIAL_BAUDRATE));
    ui->lb_dataBits->setProperty(FIELD_NAME, QString(SERIAL_DATABITS));
    ui->lb_stopBits->setProperty(FIELD_NAME, QString(SERIAL_STOPBITS));
    ui->lb_writeDelay->setProperty(FIELD_NAME, QString(SERIAL_WRITEDELAY));
    ui->lb_waitPacketTime->setProperty(FIELD_NAME, QString(SERIAL_WAITPACKETTIME));

    setMode(eEditMode);
    sett.fields[SERIAL_BAUDRATE] = "1200";
    sett.fields[SERIAL_PARITY] = "fgdfg";

    loadSettingStatus({ SERIAL_BAUDRATE, SERIAL_DATABITS });
}

CMainWindow::~CMainWindow()
{
    delete ui;
}

void CMainWindow::on_wgt_model_clicked(QString nameEl)
{
    if (m_mode != eEditMode) {
        loadSettingsFields(nameEl);
        setMode(eViewMode);
    }
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

void CMainWindow::on_pb_radio_clicked()
{
    ui->gb_settings->setVisible(true);
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
        SSerialSection sett;
        sett.fields[SERIAL_IFACE] = ifaceName;
        sett.fields[SERIAL_STATUS] = "off";
        m_serial.sendData(sett);
        ui->wgt_model->changeState(ifaceName, eDisconnected);
    }
}

void CMainWindow::readyRead()
{
    setMode(eViewMode);
    //читаем все настройки и разбираем их на подтвержения и ответы на запросы
    auto setts = m_serial.readAllSettings();
    for (auto sett : setts) {
        //Если подтверждение
        if (sett.isConfirmation())
            processConfirmation(sett);
        //Если пришли текущие настройки МПД
        else
            setCurrentSetting(sett);
    }
}

void CMainWindow::processConfirmation(const SSection &sett)
{
    auto type = sett.getType();
    // если есть ошибки, не связанные с конкретной настройкой, выводим в строку
    setError(sett.getErrorsStr());

    auto listErrorFields = sett.getErrorFields();

    //если нет ошибок, то просто переводим настройку из листа ожидания в основную массу
    if (listErrorFields.size() < 1) {
        confirmSetting(sett);
        return;
    }
    // ошибки common и serial рассматриваем отдельно
    switch (type) {
    case eCommon:
        break;
    case eSerial:
        loadSettingStatus(listErrorFields);
        break;
    default:
        break;
    }
}

void CMainWindow::addSetting(const SSection &sett)
{
    //ВАЖНО! пока нет обработчика для eCommon
    //Добавляем или изменяем m_sects
    for (auto it = m_sects.begin(); it != m_sects.end(); it++) {
        if (it->fields.contains(SERIAL_IFACE))
            if (it->fields[SERIAL_IFACE] == sett.fields[SERIAL_IFACE]) {
                m_sects.erase(it);
            }
    }
    m_sects.push_back(sett);
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

void CMainWindow::sectionRead(const SSection &sect)
{
    switch (sect.getType()) {
    case eSerial:
        fillSerialFieds(sect);
        break;
    case eCommon:
        ui->lb_mode->setText(sect.fields[COMMON_MODE]);
        break;
    default:
        break;
    }
    addSetting(sect);
}

void CMainWindow::debugInfo(const QString &str)
{
    ui->txt_debug->append(str);
}

void CMainWindow::requestConfirmed(const SSection &sect)
{
    if (sect.getType() == eDebug)
        for (auto field : sect.fields) {
            if (!field.isEmpty())
        }

    else
        ui->lb_status->setText("Время ожидания подтверждения превышено");
    addSetting(sect);
}

void CMainWindow::requestFailed(const SSection &sect)
{
    ui->lb_status->setText("Время ожидания подтверждения превышено");
}

void CMainWindow::serviceRequestFailed(QString name)
{
    ui->lb_status->setText("Запрос " + name + " не выполнен");
}

void CMainWindow::serviceRequestConfirmed(QString name)
{
    ui->lb_status->setText("Запрос " + name + " выполнен");
}

void CMainWindow::requestError(const QList<QString> &errorFields)
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

void CMainWindow::confirmSetting(const SSection &sett)
{
    setCurrentSetting(sett);
    // удаляем настройку из листа ожидания подтверждения (m_processingSetts)
    for (auto it = m_processingSetts.begin(); it != m_processingSetts.end(); it++) {
        if (it->fields.contains(SERIAL_IFACE))
            if (it->fields[SERIAL_IFACE] == sett.fields[SERIAL_IFACE]) {
                m_processingSetts.erase(it);
            }
    }
}

void CMainWindow::requestCurrentSettings(ESettingsType type)
{
    m_serial.requestData(ESettingsType::eCommon);
    m_serial.requestData(ESettingsType::eSerial);
}

void CMainWindow::prepareProtocol()
{
    if (!m_protocol)
        m_protocol = new CProtocolTransmitter(this);

    connect(m_protocol, &CProtocolTransmitter::s_sectionRead, this, &CMainWindow::sectionRead);
    connect(m_protocol, &CProtocolTransmitter::s_debugInfo, this, &CMainWindow::debugInfo);
    connect(m_protocol, &CProtocolTransmitter::s_requestError, this, &CMainWindow::requestError);
    connect(m_protocol, &CProtocolTransmitter::s_requestFailed, this, &CMainWindow::debugInfo);
    connect(m_protocol, &CProtocolTransmitter::s_requestConfirmed, this,
            &CMainWindow::requestError);
    connect(m_protocol, &CProtocolTransmitter::s_serviceRequestFailed, this,
            &CMainWindow::serviceRequestFailed);
    connect(m_protocol, &CProtocolTransmitter::s_serviceRequestConfirmed, this,
            &CMainWindow::serviceRequestConfirmed);
}

void CMainWindow::on_pb_accept_clicked()
{
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
    // добавили в лист ожидания
    m_processingSetts.append(sett);

    m_serial.sendData(sett);
    ui->wgt_model->changeState(iface, eConnected);
}

void CMainWindow::on_pb_stat_clicked()
{
    m_protocol->getRequest(eStat);
}
