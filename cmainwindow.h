#ifndef CMAINWINDOW_H
#define CMAINWINDOW_H

#include <QMainWindow>
#include "cserialport.h"
#include "ssettings.h"

const QString DIALOG_FINISH_EDITING = "Вы действительно хотите завершить настройку МПД?";
const QString STATUS_NO_PORTS = "Нет доступных устройств для подключения";

QT_BEGIN_NAMESPACE namespace Ui
{
    class CMainWindow;
}
QT_END_NAMESPACE

class CMainWindow : public QMainWindow
{
    Q_OBJECT
    enum EPage { eStartPage, eMainPage };
    enum ESettingsMode { eHidden, eViewMode, eEditMode, eWaitMode };

public:
    CMainWindow(QWidget *parent = nullptr);
    ~CMainWindow();

    // GUI - functions
private slots:
    ///   Обработчики нажатий на графику
    /// моделька МПД
    void on_wgt_model_clicked(QString nameEl);
    ///
    void on_pb_radio_clicked();
    /// подключить МПД
    void on_pb_startConfigure_clicked();
    void on_pb_closeSettings_clicked();
    void on_pb_finishConfigure_clicked();
    void on_pb_cancel_clicked();
    void on_pb_accept_clicked();
    void on_pb_stat_clicked();
    void on_pb_edit_clicked();
    void enableSettings(bool st);

    //! пользователь захотел подключить/отключить интерфейс
    void wantChangeStateReceived(QString ifaceName, EState st);
    //! чтение настроек, пришедших с МПД
    void readyRead();
    //! разбор подтверждений (ответов)
    void processConfirmation(const SSection &sect);
    //! добавление текущих настроек (МПД прислал текущие настройки, нужно их обработать)
    void addSetting(const SSection &sect);
    //! порт отключили
    void serialRemoved();
    //! ошибка в порте
    void setError(QString errorStr);

    //! заполняет поля настроек для serial-интерфейсов
    void fillSerialFieds(const SSection &sect);

    ///***слоты сообщений от порта и протокола
    ///! обработка ответа на get-запрос
    void sectionRead(const SSection &sect);
    ///! обработка полученной отладочной информации
    void debugInfo(const QString &str);
    ///! обработка подтверждения set-запроса или отладочного запроса
    void requestConfirmed(const SSection &sect);
    ///! время ожидания запроса превышено
    void requestFailed(const SSection &sect);
    ///! запрос получен МПД, но не подтвержден из-за ошибочных значений в полях
    void requestError(const QList<QString> &errorFields);
    ///! отладочный запрос не выполнен
    void serviceRequestFailed(QString name);
    ///! отладочный запрос выполнен
    void serviceRequestConfirmed(QString name);

private:
    ///!устанавливает вид страницы
    void setPage(EPage page);
    ///! устанавливает вид настроек (только для eMainPage)
    void setMode(ESettingsMode mode);
    ///! обновляет комбо-бокс с доступными портами (только для eStartPage)

    void updateAvaliablePorts();
    ///! заполняет поля настроек для заданного интерфейса (берет их из  m_setts)
    void loadSettingsFields(QString nameElement);
    ///! Подсвечивает непринятые настройки, обесцвечивает принятые
    void loadSettingStatus(const QList<QString> &errorFields);
    ///! добавляет настройку в m_setts или заменяет уже существующую
    void confirmSetting(const SSettings &sett);
    ///! запрашивает от МПД текущие настройки
    void requestCurrentSettings(ESettingsType type = eNoSection);

    ///!
    void prepareProtocol();

private:
    Ui::CMainWindow *ui;
    //!*** Fields for GUI
    ESettingsMode m_mode { eHidden };
    EPage m_page { eStartPage };

    ///секции с текущими настройкамми
    QList<SSection> m_sects;

    //!*** Fields for external connections
    CSerialPort m_serial;
    CProtocolTransmitter *m_protocol { nullptr };
};
#endif // CMAINWINDOW_H
