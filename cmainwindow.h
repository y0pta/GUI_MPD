#ifndef CMAINWINDOW_H
#define CMAINWINDOW_H

#include "cserialport.h"
#include "ssettings.h"
#include <QMainWindow>

const QString DIALOG_FINISH_EDITING = "Вы действительно хотите завершить настройку МПД?";
const QString STATUS_NO_PORTS = "Нет доступных устройств для подключения";

QT_BEGIN_NAMESPACE namespace Ui
{
    class CMainWindow;
}
QT_END_NAMESPACE

class CMainWindow : public QMainWindow {
    Q_OBJECT
    enum EPage { eStartPage,
        eMainPage,
        eLoading };
    enum ESettingsMode { eHidden,
        eViewMode,
        eEditMode };
    Q_PROPERTY(EPage page READ getPage WRITE setPage)
    Q_PROPERTY(ESettingsMode mode READ getMode WRITE setMode)

public:
    CMainWindow(QWidget* parent = nullptr);
    ~CMainWindow();

protected:
    ///!устанавливает вид страницы
    void setPage(EPage page);
    ///! устанавливает вид настроек (только для eMainPage)
    void setMode(ESettingsMode mode);
    EPage getPage() { return page; }
    ESettingsMode getMode() { return mode; }
    ///! обновляет комбо-бокс с доступными портами (только для eStartPage)
    void updateAvaliablePorts();
    ///! заполняет поля настроек для заданного интерфейса (берет их из  m_setts)
    void loadSettingsFields(QString nameElement);
    ///! Подсвечивает непринятые настройки, обесцвечивает принятые
    void loadSettingStatus(const QList<QString>& errorFields);

    ///!
    void prepareProtocol();

    // GUI - functions
private slots:
    ///   Обработчики нажатий на графику
    /// моделька МПД
    void showSettings(const QString& nameEl);
    ///
    //   void on_pb_radio_clicked();
    /// подключить МПД
    void on_pb_startConfigure_clicked();
    void on_pb_closeSettings_clicked();
    void on_pb_finishConfigure_clicked();
    void on_pb_cancel_clicked();
    void on_pb_accept_clicked();
    void on_pb_edit_clicked();
    void on_pb_refreshStat_clicked();
    void on_pb_resetStat_clicked();
    void on_pb_applyMode_clicked();
    void on_cb_dumpOn_stateChanged(int arg1);
    void on_pb_clearErrors_clicked();
    void on_pb_clearFlash_clicked();
    void on_pb_clearDebug_clicked();
    void enableSettings(bool st);

    //! пользователь захотел подключить/отключить интерфейс
    void wantChangeStateReceived(QString ifaceName, EState st);
    //! порт отключили
    void serialRemoved();
    //! ошибка в порте
    void setError(QString errorStr);
    //! заполняет поля настроек для serial-интерфейсов
    void fillSerialFieds(const SSection& sect);
    //! выделяет неподтвержденные поля настроек для serial-интерфейсов
    void colorFields(const QList<QString>& errorNames);
    ///! изменяет статус интерфейса на интерактивной модели
    void setIfaceState(const QString& nameEl, const QString& state);
    void setIfaceState(const QString& nameEl, EState state);
    //! обрабатывает пришедщую с МПД serial-настройку
    void processSerial(const SSection& sect, const QList<QString>& errorNames = {});
    //! обрабатывает пришедщую с МПД common-настройку
    void processCommon(const SSection& sect);

    ///***слоты сообщений от порта и протокола
    ///! обработка ответа на get-запрос
    void setSection(const SSection& sect);
    ///! обработка полученной отладочной информации
    void setDebugInfo(const QString& str);
    ///! обработка подтверждения set-запроса или отладочного запроса
    void requestConfirmed(const SSection& sect);
    ///! время ожидания запроса превышено
    void requestFailed(const SSection& sect);
    ///! запрос получен МПД, но не подтвержден из-за ошибочных значений в полях
    void requestError(const SSection& answer);
    ///! отладочный запрос не выполнен
    void serviceRequestFailed(QString name);
    ///! отладочный запрос выполнен
    void serviceRequestConfirmed(QString name);

    void on_pb_startUpdate_clicked();

private:
private:
    Ui::CMainWindow* ui;

    EPage page { eStartPage };
    ESettingsMode mode { eHidden };
    ///секции с текущими настройкамми
    QVector<SSection> m_sects;

    //!*** Fields for external connections
    CSerialPort m_serial;
    CProtocolTransmitter* m_protocol { nullptr };
};
#endif // CMAINWINDOW_H
