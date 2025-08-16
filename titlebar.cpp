#include "titlebar.h"
#include "register.h"

#include <QHBoxLayout>
#include <QStyle>
#include <QIcon>

TitleBar::TitleBar(QWidget *parent)
    : QWidget(parent)
{
    setStyleSheet("background-color: #333; color: white;");
    setMinimumHeight(60);  // 增加高度以容纳两层

    // 创建垂直布局作为主布局
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(5, 0, 5, 0);
    mainLayout->setSpacing(5);

    // ================ 第一层：现有功能 ================
    QHBoxLayout *titleLayout = new QHBoxLayout();
    titleLayout->setContentsMargins(0, 0, 0, 0);
    titleLayout->setSpacing(5);

    // 添加图标和标题
    m_iconLabel = new QLabel();
    m_iconLabel->setPixmap(QPixmap("://icons/player_icon.png").scaled(24, 24));
    m_titleLabel = new QLabel("FFmpeg视频播放器");
    m_titleLabel->setStyleSheet("color: white;");

    // 添加弹簧布局
    QWidget *spacer = new QWidget();
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    // 添加常用功能按钮
    m_monitorButton = new QPushButton("监控");
    m_playbackButton = new QPushButton("回放");
    m_logButton = new QPushButton("日志");
    m_systemButton = new QPushButton("系统");
    m_loginButton = new QPushButton("登录");

    // 添加窗口控制按钮
    m_minimizeButton = new QPushButton("-");
    m_maximizeButton = new QPushButton("□");
    m_closeButton = new QPushButton("×");

    // 设置按钮样式
    QList<QPushButton*> buttons = {m_monitorButton, m_playbackButton, m_logButton, m_systemButton,
                                  m_minimizeButton, m_maximizeButton, m_closeButton, m_loginButton};
    for (auto button : buttons) {
        button->setFixedSize(40, 25);
        button->setStyleSheet("color: white; background-color: #555;");
    }

    // 添加到第一层布局
    titleLayout->addWidget(m_iconLabel);
    titleLayout->addWidget(m_titleLabel);
    titleLayout->addWidget(spacer);

    titleLayout->addWidget(m_monitorButton);
    titleLayout->addWidget(m_playbackButton);
    titleLayout->addWidget(m_logButton);
    titleLayout->addWidget(m_systemButton);
    titleLayout->addWidget(m_loginButton);

    titleLayout->addWidget(m_minimizeButton);
    titleLayout->addWidget(m_maximizeButton);
    titleLayout->addWidget(m_closeButton);

    // ================ 第二层：登录界面 ================
    m_loginWidget = new QWidget();
    m_loginWidget->setStyleSheet("background-color: black;");
    m_loginWidget->setVisible(true);  // 默认隐藏

    QHBoxLayout *loginLayout = new QHBoxLayout(m_loginWidget);
    loginLayout->setContentsMargins(10, 5, 10, 5);
    loginLayout->setSpacing(5);

    // 登录提示
    m_loginPromptLabel = new QLabel("登录界面享受联网功能");
    m_loginPromptLabel->setStyleSheet("color: white;");

    // 账号
    m_usernameLabel = new QLabel("账号：");
    m_usernameLabel->setStyleSheet("color: white;");
    m_usernameLineEdit = new QLineEdit();
    m_usernameLineEdit->setFixedWidth(100);
    m_usernameLineEdit->setStyleSheet("background-color: #666; color: white;");

    // 密码
    m_passwordLabel = new QLabel("密码：");
    m_passwordLabel->setStyleSheet("color: white;");
    m_passwordLineEdit = new QLineEdit();
    m_passwordLineEdit->setEchoMode(QLineEdit::Password);  // 隐藏密码
    m_passwordLineEdit->setFixedWidth(100);
    m_passwordLineEdit->setStyleSheet("background-color: #666; color: white;");

    // 登录按钮
    m_loginSubmitButton = new QPushButton("登录");
    m_loginSubmitButton->setFixedSize(60, 25);
    m_loginSubmitButton->setStyleSheet("color: white; background-color: #555;");

    // 注册按钮
    m_registerButton = new QPushButton("注册");
    m_registerButton->setFixedSize(60, 25);
    m_registerButton->setStyleSheet("color: white; background-color: #555;");

    // 弹簧
    QWidget *loginSpacer = new QWidget();
    loginSpacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    // 退出按钮
    m_exitLoginButton = new QPushButton("×");
    m_exitLoginButton->setFixedSize(25, 25);
    m_exitLoginButton->setStyleSheet("color: white; background-color: #777;");

    // 添加到登录布局
    loginLayout->addWidget(m_loginPromptLabel);
    loginLayout->addSpacing(10);
    loginLayout->addWidget(m_usernameLabel);
    loginLayout->addWidget(m_usernameLineEdit);
    loginLayout->addWidget(m_passwordLabel);
    loginLayout->addWidget(m_passwordLineEdit);
    loginLayout->addSpacing(10);
    loginLayout->addWidget(m_loginSubmitButton);
    loginLayout->addWidget(m_registerButton);
    loginLayout->addWidget(loginSpacer);
    loginLayout->addWidget(m_exitLoginButton);

    // 添加两层到主布局
    mainLayout->addLayout(titleLayout);
    mainLayout->addWidget(m_loginWidget);

    // 连接信号槽
    // - 第一层：基础功能
    connect(m_monitorButton, &QPushButton::clicked, this, &TitleBar::monitorButtonClicked);
    connect(m_playbackButton, &QPushButton::clicked, this, &TitleBar::playbackButtonClicked);
    connect(m_logButton, &QPushButton::clicked, this, &TitleBar::logButtonClicked);
    connect(m_systemButton, &QPushButton::clicked, this, &TitleBar::systemButtonClicked);
    connect(m_loginButton, &QPushButton::clicked, this, &TitleBar::loginButtonClicked);

    // - 第一层：窗口尺寸
    connect(m_minimizeButton, &QPushButton::clicked, this, &TitleBar::minimizeClicked);
    connect(m_maximizeButton, &QPushButton::clicked, this, &TitleBar::maximizeClicked);
    connect(m_closeButton, &QPushButton::clicked, this, &TitleBar::closeClicked);

    // - 第二层：登录相关
    connect(m_loginSubmitButton, &QPushButton::clicked, this, &TitleBar::onLoginSubmitButtonClicked);
    connect(m_exitLoginButton, &QPushButton::clicked, this, &TitleBar::onExitLoginClicked);
    connect(m_registerButton, &QPushButton::clicked, this, &TitleBar::onRegisterButtonClicked);
}

TitleBar::~TitleBar()
{
}

void TitleBar::onLoginButtonClicked()
{
    // 显示登录界面
    qDebug() << "登录按钮被点击";
    m_loginWidget->setVisible(true);
    // 调整标题栏高度
    setMinimumHeight(60);
    setFixedHeight(60);
}

void TitleBar::onLoginSubmitButtonClicked()
{
    // 显示登录界面
    qDebug() << "登录提交按钮被点击";
    m_loginWidget->setVisible(true);
    // 调整标题栏高度
    setMinimumHeight(60);
    setFixedHeight(60);
}

void TitleBar::onRegisterButtonClicked()
{
    // 创建并显示注册对话框
    Register *registerDialog = new Register(this);
    registerDialog->exec();

    // 注册完成后关闭登录界面
    onExitLoginClicked();
}

void TitleBar::onExitLoginClicked()
{
    // 隐藏登录界面
    m_loginWidget->setVisible(false);
    // 恢复标题栏高度
    setMinimumHeight(30);
    setFixedHeight(30);
}
