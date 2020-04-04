#include "cmainwindow.h"

#include <QApplication>
#include "cprotocoltransmitter.h"
#include <QDebug>
#include <QFile>
#include <tests/tests_ccmdtransmitter.h>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    testRequestSettings();
    // CMainWindow w;
    // w.show();
    return a.exec();
}
