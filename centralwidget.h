#ifndef CENTRALWIDGET_H
#define CENTRALWIDGET_H

#include "videowidget.h"  // 添加VideoWidget头文件

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>

class CentralWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CentralWidget(QWidget *parent = nullptr);
    ~CentralWidget();

    // 获取视频播放窗口
    VideoWidget* getVideoWidget() const;

    // 新增：显示/隐藏按钮方法
    void showButtons(bool show);

signals:
    void openFileButtonClicked();      // 打开文件按钮点击信号
    void openUrlButtonClicked();       // 新增：打开URL按钮点击信号

private:
    VideoWidget *m_videoWidget;        // 视频播放窗口
    QPushButton *m_openFileButton;     // 打开文件按钮
    QPushButton *m_openUrlButton;      // 新增：打开URL按钮
};

#endif // CENTRALWIDGET_H
