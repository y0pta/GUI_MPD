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
    ///   void changeSerialState(QString textName, EState st);
    void on_wgt_model_clicked(QString nameEl);
    void on_pb_radio_clicked();
    void on_pb_startConfigure_clicked();
    void on_pb_closeSettings_clicked();

    void enableSettings(bool st);

    void on_pb_edit_clicked();
    //! пользователь захотел подключить/отключить интерфейс
    void wantChangeStateReceived(QString ifaceName, EState st);
    //! чтение настроек, пришедших с МПД
    void readyRead();
    //! разбор подтверждений (ответов)
    void processConfirmation(const SSettings &sett);
    //! добавление текущих настроек (МПД прислал текущие настройки, нужно их обработать)
    void setCurrentSetting(const SSettings &sett);
    //! порт отключили
    void serialRemoved();
    //! ошибка в порте
    void setError(QString errorStr);

    void on_pb_finishConfigure_clicked();

    void on_pb_cancel_clicked();

    void on_pb_accept_clicked();

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

private:
    Ui::CMainWindow *ui;
    //!*** Fields for GUI
    ESettingsMode m_mode { eHidden };
    EPage m_page { eStartPage };

    QList<SSettings> m_setts;
    QList<SSettings> m_processingSetts;

    //!*** Fields for external connections
    CSerialPort m_serial;
};
#endif // CMAINWINDOW_H
