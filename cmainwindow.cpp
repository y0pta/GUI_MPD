#include "cmainwindow.h"
#include "ui_cmainwindow.h"
#include <QMessageBox>
#include <QSignalMapper>

CMainWindow::CMainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::CMainWindow)
{
    ui->setupUi(this);

    ///Настраиваем виджет с моделью
    ui->wgt_model->addElements(3, { SERIAL_IFACE_RADIO, SERIAL_IFACE_RS232, SERIAL_IFACE_RS485 });
    connect(ui->wgt_model, &CMpdWidget::s_clicked, this, &CMainWindow::showSettings);
    connect(ui->wgt_model, &CMpdWidget::s_wantChangeState, this,
        &CMainWindow::wantChangeStateReceived);
    /// настройка порта для настройки МПД
    updateAvaliablePorts();
    setPage(eStartPage);

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
    setSection(sett);
    setSection(SSerialSection::getSerialDefault(SERIAL_IFACE_RS232));
    setSection(SSerialSection::getSerialDefault(SERIAL_IFACE_RS485));

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

void CMainWindow::setPage(EPage p)
{
    page = p;
    ui->gb_settings->setVisible(false);
    ui->lb_status->setVisible(true);
    ui->lb_status->clear();

    if (p == eMainPage) {
        ui->gb_start->setVisible(false);
        ui->gb_mpdModel->setVisible(true);
        ui->pb_finishConfigure->setVisible(true);
        ui->left_tab->setVisible(true);
        ui->lb_mode->setVisible(true);
        ui->lb_status->setAlignment(Qt::AlignLeft);
    }
    if (p == eStartPage) {
        ui->gb_start->setVisible(true);
        ui->left_tab->setVisible(false);
        ui->gb_mpdModel->setVisible(false);
        ui->pb_finishConfigure->setVisible(false);
        ui->lb_mode->setVisible(false);
        ui->lb_status->setAlignment(Qt::AlignCenter);
    }
    if (p == eLoading) {
        ui->lb_status->setText("Загрузка...");
        ui->gb_start->setVisible(false);
        ui->left_tab->setVisible(false);
        ui->gb_mpdModel->setVisible(false);
        ui->pb_finishConfigure->setVisible(false);
        ui->lb_mode->setVisible(false);
        ui->lb_status->setAlignment(Qt::AlignLeft);
    }
}

void CMainWindow::setMode(ESettingsMode m)
{
    if (getPage() == eMainPage) {
        mode = m;
        if (m == eViewMode) {
            ui->pb_edit->setVisible(true);
            ui->pb_accept->setVisible(false);
            ui->pb_cancel->setVisible(false);
            ui->wgt_model->setFreeze(false);

            ui->gb_settings->setVisible(true);
            ui->pb_closeSettings->setEnabled(true);

            enableSettings(false);
        } else if (m == eEditMode) {
            ui->pb_edit->setVisible(false);
            ui->pb_accept->setVisible(true);
            ui->pb_cancel->setVisible(true);
            ui->wgt_model->setFreeze(true);

            ui->gb_settings->setVisible(true);
            ui->pb_closeSettings->setEnabled(true);

            enableSettings(true);
        } else if (m == eHidden) {
            ui->wgt_model->setFreeze(false);
            ui->gb_settings->setVisible(false);
        }
    }
}

void CMainWindow::updateAvaliablePorts()
{
    if (getPage() == eStartPage) {
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

        m_protocol->getRequest(eCommon);
        m_protocol->getRequest(eStat);
        m_protocol->getRequest(eSerial);

        setPage(eLoading);
    }
}

void CMainWindow::on_pb_closeSettings_clicked()
{
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

void CMainWindow::fillSerialFieds(const SSection& sect)
{
    if (sect.getType() != eSerial)
        return;

    if (!sect.fields[SERIAL_STATUS].isEmpty())
        ui->wgt_model->changeState(sect.fields[SERIAL_IFACE], STATE_TYPES.key(sect.fields[SERIAL_STATUS]));
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

void CMainWindow::setIfaceState(const QString& nameEl, const QString& state)
{
    EState status = eError;
    if (state == STATUS_ON)
        status = eConnected;
    if (state == STATUS_OFF)
        status = eDisconnected;
    ui->wgt_model->changeState(nameEl, status);
}

void CMainWindow::setIfaceState(const QString& nameEl, EState state)
{
    ui->wgt_model->changeState(nameEl, state);
}

void CMainWindow::colorFields(const QList<QString>& errorNames)
{
    /// Идем по всем полям формы с настройками, подсвечиваем те, которые ошибочные. Обесцвечиваем не
    /// ошибочные
    for (int i = 0; i < ui->fl_unitSettings->rowCount(); i++) {
        auto label = ui->fl_unitSettings->itemAt(i, QFormLayout::ItemRole::LabelRole)->widget();
        auto fieldName = label->property(FIELD_NAME).toString();
        qobject_cast<QLabel*>(label)->setStyleSheet("QLabel { color: black }; ");

        for (auto errorField : errorNames) {
            if (fieldName == errorField)
                qobject_cast<QLabel*>(label)->setStyleSheet("QLabel { color: red }; }");
        }
    }
}

void CMainWindow::processSerial(const SSection& sect, const QList<QString>& errorNames)
{
    //если уже существует уже такая настройка, находим
    int i = 0;
    for (; i < m_sects.size(); i++) {
        if (m_sects.at(i).fields[SERIAL_IFACE] == sect.fields[SERIAL_IFACE]) {
            m_sects[i] = sect;
            break;
        }
    }
    if (i == m_sects.size())
        m_sects.append(sect);

    colorFields(errorNames);
    fillSerialFieds(sect);

    setIfaceState(sect.fields[SERIAL_IFACE], sect.fields[SERIAL_STATUS]);
}

void CMainWindow::processCommon(const SSection& sect)
{
    ui->lb_mode->setText(sect.fields[COMMON_MODE]);

    //если уже существует уже такая настройка, находим
    auto i = 0;
    for (; i < m_sects.size(); i++)
        if (m_sects.at(i).getType() == eCommon)
            break;
    //если нет, добавляем новую
    if (i == m_sects.size()) {
        m_sects.push_back(SCommonSection());
    }

    //добавили или обновили поля
    if (!sect.fields[COMMON_MODE].isEmpty())
        m_sects[i].fields[COMMON_MODE] = sect.fields[COMMON_MODE];
    if (!sect.fields[COMMON_DUMPSTATUS].isEmpty())
        m_sects[i].fields[COMMON_DUMPSTATUS] = sect.fields[COMMON_DUMPSTATUS];

    // обновляем вид
    /// для режима
    ui->lb_mode->setText(m_sects[i].fields[COMMON_MODE]);
    if (m_sects[i].fields[COMMON_MODE] == COMMON_MODE_VAL[eSms])
        ui->rb_sms->setChecked(true);
    if (m_sects[i].fields[COMMON_MODE] == COMMON_MODE_VAL[eClarity])
        ui->rb_clarity->setChecked(true);
    /// для дампа
    if (m_sects[i].fields[COMMON_DUMPSTATUS] == STATUS_ON)
        ui->cb_dumpOn->setChecked(true);
    if (m_sects[i].fields[COMMON_DUMPSTATUS] == STATUS_OFF)
        ui->cb_dumpOn->setChecked(false);
}

void CMainWindow::setSection(const SSection& sect)
{
    if (page == eLoading)
        setPage(eMainPage);
    switch (sect.getType()) {
    case eStat:
        ui->txt_statistics->append(sect.fields[STAT_TEXT]);
        break;
    case eSerial:
        processSerial(sect);
        break;
    case eCommon:
        processCommon(sect);
    default:
        break;
    }
}

void CMainWindow::setDebugInfo(const QString& str)
{
    ui->txt_debug->append(str);
}

void CMainWindow::requestConfirmed(const SSection& sect)
{
    setSection(sect);
    ui->txt_debug->append("   Request approved: " + getStr(sect.getType()));
}

void CMainWindow::requestFailed(const SSection& sect)
{
    if (page == eLoading)
        setPage(eStartPage);
    if (sect.getType() == eStat) {
        ui->txt_statistics->append("Время ожидания подтверждения превышено");
    } else
        ui->lb_status->setText("Время ожидания подтверждения превышено");
}

void CMainWindow::serviceRequestFailed(QString name)
{
    ui->txt_debug->append("  Запрос " + name + " не выполнен");
    ui->lb_status->setText("Запрос " + name + " не выполнен");
}

void CMainWindow::serviceRequestConfirmed(QString name)
{
    ui->txt_debug->append("  Запрос " + name + " выполнен");
    ui->lb_status->setText("Запрос " + name + " выполнен");
}

void CMainWindow::requestError(const SSection& answer)
{
    ui->txt_debug->append("   Request failed: ");
    colorFields(SSection::getErrorFields(answer));
    setIfaceState(answer.fields[SERIAL_IFACE], eError);
}

void CMainWindow::on_pb_finishConfigure_clicked()
{
    QMessageBox* box = new QMessageBox(QMessageBox::Warning, "Завершить настройку МПД", DIALOG_FINISH_EDITING);
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

void CMainWindow::loadSettingStatus(const QList<QString>& errorFields)
{
    // Идем по всем полям формы, подсвечиваем те, которые ошибочные
    for (int i = 0; i < ui->fl_unitSettings->rowCount(); i++) {
        auto label = ui->fl_unitSettings->itemAt(i, QFormLayout::ItemRole::LabelRole)->widget();
        auto fieldName = label->property(FIELD_NAME).toString();
        qobject_cast<QLabel*>(label)->setStyleSheet("QLabel { color: black }; ");

        for (auto errorField : errorFields) {
            if (fieldName == errorField)
                qobject_cast<QLabel*>(label)->setStyleSheet("QLabel { color: red }; }");
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

void CMainWindow::showSettings(const QString& nameEl)
{
    if (getMode() != eEditMode) {
        loadSettingsFields(nameEl);
        setMode(eViewMode);
    }
}

void CMainWindow::on_pb_accept_clicked()
{
    if (!m_protocol || !m_serial.isOpen())
        return;

    setMode(eViewMode);

    SSerialSection sett;
    QString iface = ui->lb_ifaceName->text();
    sett.fields[SERIAL_IFACE] = iface;
    sett.fields[SERIAL_STATUS] = STATUS_ON;
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

void CMainWindow::on_pb_startUpdate_clicked()
{
    m_serial.avaliablePorts();
    updateAvaliablePorts();
}
