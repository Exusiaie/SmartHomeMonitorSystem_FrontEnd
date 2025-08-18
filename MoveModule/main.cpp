#include "PanTiltControlWidget.h"
#include <QApplication>


int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    PanTiltControlWidget w;
    w.show();
    return a.exec();
}
