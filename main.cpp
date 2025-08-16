#include "FFmpeg_Player.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    FFmpeg_Player w;
    w.show();
    return a.exec();
}
