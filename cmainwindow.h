#ifndef CMAINWINDOW_H
#define CMAINWINDOW_H

#include <QMainWindow>
#include "cserialport.h"
#include "ssettings.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class CMainWindow;
}
QT_END_NAMESPACE

class CMainWindow : public QMainWindow
{
    Q_OBJECT
    enum EPage { eStartPage, eMainPage };
    enum ESettingsMode { eNone, eViewMode, eEditMode, eWaitMode };

public:
    CMainWindow(QWidget *parent = nullptr);
    ~CMainWindow();

    // GUI - functions
private slots:
    //   void changeSerialState(QString textName, EState st);
    void on_wgt_model_clicked(QString nameEl);
    void on_pb_radio_clicked();
    void on_pb_startConfigure_clicked();
    void on_pb_closeSettings_clicked();

    void enableSettings(bool st);

    void on_pb_edit_clicked();
    // пользователь захотел подключить/отключить интерфейс
    void wantChangeStateReceived(QString ifaceName, EState st);

private:
    //
    void setView(EPage page);
    void setMode(ESettingsMode mode);
    void updateAvaliablePorts(QList<QString>);

private:
    Ui::CMainWindow *ui;
    //*** Fields for GUI
    ESettingsMode m_mode { eNone };
    // Selected unit
    SSettingsSerial selectedUnit;
    QList<SSettingsSerial> units;

    //*** Fields for external connections
    CSerialPort m_serial;
};
#endif // CMAINWINDOW_H
