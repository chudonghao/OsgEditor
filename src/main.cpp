#include <QApplication>

#include "ui/MainWindow.h"

int main(int argc, char *argv[]) {
  QApplication app(argc, argv);

  MainWindow *w = new MainWindow;
  w->show();

  return QApplication::exec();
}
