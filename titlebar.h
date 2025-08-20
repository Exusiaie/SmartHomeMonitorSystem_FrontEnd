#ifndef TITLEBAR_H
#define TITLEBAR_H

#include "login.h"
#include "register.h"

#include <QMenu>
#include <QAction>
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

    void setLoginStatus(bool isLoggedIn, const QString& username);  // 设置登录状态的方法

// 信号转发 -> 信号链式连接
signals:
    void monitorButtonClicked();
    void playbackButtonClicked();
    void logButtonClicked();
//    void systemButtonClicked();    // 删除，实在想不到这个功能应该怎么实现
    void loginButtonClicked();
    void logoutButtonClicked();      // 添加退出登录信号

    void minimizeClicked();
    void maximizeClicked();
    void closeClicked();

    void systemMessage(const QString &message);      // 添加错误信号

private slots:
    void onLoginButtonClicked();     // 点击登录按钮触发
    void onLoginSubmit();            // 提交登录表单触发
    void onRegisterSubmit();
    void onExitLoginClicked();

    void onLogoutButtonClicked();    // 退出登录按钮触发
    void onOpenLogActionTriggered(); // 打开日志
    void onClearLogActionTriggered(); // 清空日志

private:
    // 图标与标题
    QLabel *m_iconLabel;
    QLabel *m_titleLabel;

    // 常用功能
    QPushButton *m_monitorButton;
    QPushButton *m_playbackButton;
    QPushButton *m_logButton;
//    QPushButton *m_systemButton;
    QPushButton *m_loginButton;
    QPushButton *m_logoutButton;     // 添加退出登录按钮
    QLabel      *m_usernameLabel;    // 显示登录用户名
    QLabel      *m_accountLabel;

    // 窗口尺寸
    QPushButton *m_minimizeButton;
    QPushButton *m_maximizeButton;
    QPushButton *m_closeButton;

    // 登录界面相关
    QWidget *m_loginWidget;          // 准备登录界面容器
    QWidget *m_loggedInWidget;       // 已经登录界面容器
    QLabel *m_loginPromptLabel;      // 登录提示
    QLineEdit *m_usernameLineEdit;   // 账号输入框
    QLabel *m_passwordLabel;         // 密码标签
    QLineEdit *m_passwordLineEdit;   // 密码输入框
    QPushButton *m_loginSubmitButton;// 登录按钮
    QPushButton *m_registerButton;   // 注册按钮
    QPushButton *m_exitLoginButton_layer1;  // 退出登录界面按钮

    // 日志菜单相关
    QMenu   *m_logMenu;
    QAction *m_openLogAction;
    QAction *m_clearLogAction;

    // 登录注册逻辑相关
    Login *m_login;                  // 登录
    Register *m_register;            // 注册
    bool m_isLoggedIn;               // 登录状态
    QString m_currentUsername;       // 当前登录用户名
    QPushButton *m_exitLoginButton_layer2;  // 退出登录界面按钮
};

#endif // TITLEBAR_H
