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
    enum EMode { eNone, eEditMode, eWaitMode };

public:
    CMainWindow(QWidget *parent = nullptr);
    ~CMainWindow();

    // GUI - functions
private slots:
    void on_pb_radio_clicked();

    void on_pb_startConfigure_clicked();

private:
    //
    void setView(EPage page);
    void setView(EMode mode);
    void updateAvaliablePorts(QList<QString>);

private:
    Ui::CMainWindow *ui;
    //*** Fields for GUI
    EMode m_mode { eNone };
    // Selected unit
    SSettingsSerial selectedUnit;
    QList<SSettingsSerial> units;

    //*** Fields for external connections
    CSerialPort m_serial;
};
#endif // CMAINWINDOW_H
