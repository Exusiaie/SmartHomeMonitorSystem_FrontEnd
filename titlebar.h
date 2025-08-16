#ifndef TITLEBAR_H
#define TITLEBAR_H

#include <QWidget>
#include <QPushButton>
#include <QLabel>

class TitleBar : public QWidget
{
    Q_OBJECT

public:
    explicit TitleBar(QWidget *parent = nullptr);
    ~TitleBar();

signals:
    void monitorButtonClicked();
    void playbackButtonClicked();
    void logButtonClicked();
    void systemButtonClicked();
    void loginButtonClicked();

    void minimizeClicked();
    void maximizeClicked();
    void closeClicked();

private:
    // 图标与标题
    QLabel *m_iconLabel;
    QLabel *m_titleLabel;

    // 常用功能
    QPushButton *m_monitorButton;
    QPushButton *m_playbackButton;
    QPushButton *m_logButton;
    QPushButton *m_systemButton;
    QPushButton *m_loginButton;

    // 窗口尺寸
    QPushButton *m_minimizeButton;
    QPushButton *m_maximizeButton;
    QPushButton *m_closeButton;
};

#endif // TITLEBAR_H
