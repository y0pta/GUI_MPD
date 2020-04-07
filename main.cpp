#include "cmainwindow.h"

#include <QApplication>
#include "cprotocoltransmitter.h"
#include <QDebug>
#include <QFile>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    CMainWindow w;
    w.show();
    return a.exec();
}
