#include <QApplication>

#include <thread>
#include <osgViewer/Viewer>
#include <osgFX/Outline>
#include <osgDB/ReadFile>
#include "ui/MainWindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    MainWindow *w = new MainWindow;
    w->show();

    return QApplication::exec();
}
