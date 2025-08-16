#ifndef TITLEBAR_H
#define TITLEBAR_H

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QStackedWidget>

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

private slots:
    void onLoginButtonClicked();
    void onLoginSubmitButtonClicked();
    void onRegisterButtonClicked();
    void onExitLoginClicked();

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

    // 登录界面相关
    QWidget *m_loginWidget;          // 登录界面容器
    QLabel *m_loginPromptLabel;      // 登录提示
    QLabel *m_usernameLabel;         // 账号标签
    QLineEdit *m_usernameLineEdit;   // 账号输入框
    QLabel *m_passwordLabel;         // 密码标签
    QLineEdit *m_passwordLineEdit;   // 密码输入框
    QPushButton *m_loginSubmitButton;// 登录按钮
    QPushButton *m_registerButton;   // 注册按钮
    QPushButton *m_exitLoginButton;  // 退出登录界面按钮
};

#endif // TITLEBAR_H
