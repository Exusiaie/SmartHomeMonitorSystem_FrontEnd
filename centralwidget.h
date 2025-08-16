#ifndef CENTRALWIDGET_H
#define CENTRALWIDGET_H

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
    QWidget* getVideoWidget() const;

signals:
    // 打开文件按钮点击信号
    void openFileButtonClicked();

private:
    QWidget *videoWidget;        // 视频播放窗口
    QPushButton *openFileButton; // 打开文件按钮
};

#endif // CENTRALWIDGET_H
