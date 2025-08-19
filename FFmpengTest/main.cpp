#include "widget.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    avformat_network_init();

    Widget w;
    w.setWindowTitle("RTMP 播放器");
    w.resize(800, 600); // 设置窗口初始大小
    w.show();

    int result = a.exec();

    avformat_network_deinit();

    return result;
}
