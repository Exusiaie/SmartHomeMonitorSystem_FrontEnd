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

signals:
    // 打开文件按钮点击信号
    void openFileButtonClicked();

private:
    VideoWidget *m_videoWidget;        // 视频播放窗口
    QPushButton *m_openFileButton; // 打开文件按钮
};

#endif // CENTRALWIDGET_H
