#include "MainWindow.h"

#include <QApplication>
#include <QSizePolicy>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    //w.setFixedSize(w.width(), w.height());
    w.show();
    return a.exec();
}
