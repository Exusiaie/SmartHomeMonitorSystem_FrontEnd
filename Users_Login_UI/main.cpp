#include "MainWindow.h"
#include "TitleBar.h"
#include <QApplication>
#include <QMainWindow>



int test2(QApplication & a){

    MainWindow loginDlg;
    if (loginDlg.exec() == QDialog::Accepted) { // 如果用户登录成功,调用了 accept()，就进入主窗口并运行事件循环
        QMainWindow w;
        w.setWindowTitle("主窗口");
        w.show();
        return a.exec();
    }else return -1;
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    test2(a);

    return 0;
}
