#include "titlebar.h"
#include "register.h"
#include "FFmpeg_Player.h"
#include "log.h"

#include <QDesktopServices>
#include <QHBoxLayout>
#include <QStyle>
#include <QIcon>

TitleBar::TitleBar(QWidget *parent)
    : QWidget(parent)
    , m_isLoggedIn(false)
{
    setStyleSheet("background-color: #555; color: white;");
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
    m_iconLabel->setPixmap(QPixmap(":/Resource/logo.png").scaled(24, 24));
    m_iconLabel->setStyleSheet("color: white; background-color: transparent;");
    m_titleLabel = new QLabel("Circuit Breakers视频播放器");
    m_titleLabel->setStyleSheet("color: white; background-color: transparent;");

    // 添加弹簧布局
    QWidget *spacer = new QWidget();
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    spacer->setStyleSheet("color: white; background-color: transparent;");

    // 添加常用功能按钮
    m_monitorButton = new QPushButton("监控");
    m_playbackButton = new QPushButton("回放");
    m_logButton = new QPushButton("日志");
//    m_systemButton = new QPushButton("系统");
    m_loginButton = new QPushButton("登录");
    m_logoutButton = new QPushButton("注销");
    m_usernameLabel = new QLabel();
    m_usernameLabel->setStyleSheet("color: white; background-color: transparent;");

    // 初始化日志菜单
    m_logMenu = new QMenu(this);
    m_openLogAction = new QAction("打开日志", this);
    m_clearLogAction = new QAction("清空日志", this);
    m_logMenu->addAction(m_openLogAction);
    m_logMenu->addAction(m_clearLogAction);
    m_logButton->setMenu(m_logMenu);

    // 添加窗口控制按钮
    m_minimizeButton = new QPushButton("-");
    m_maximizeButton = new QPushButton("□");
    m_closeButton = new QPushButton("×");

    // 设置按钮样式
    QList<QPushButton*> buttons = {m_monitorButton, m_playbackButton, m_logButton, m_loginButton, m_logoutButton,
                                      m_minimizeButton, m_maximizeButton, m_closeButton};
    for (auto button : buttons)
    {
        button->setFixedSize(40, 25);
        button->setStyleSheet(
            "QPushButton {"
            "    background-color: transparent;"
            "    color: white;"
            "}"
            "QPushButton:hover {"
            "    background-color: rgba(200, 200, 200, 0.2);"
            "}"
        );
    }

    // 添加到第一层布局
    titleLayout->addWidget(m_iconLabel);
    titleLayout->addWidget(m_titleLabel);
    titleLayout->addWidget(spacer);

    titleLayout->addWidget(m_monitorButton);
    titleLayout->addWidget(m_playbackButton);
    titleLayout->addWidget(m_logButton);
//    titleLayout->addWidget(m_systemButton);
    titleLayout->addWidget(m_loginButton);
    titleLayout->addWidget(m_usernameLabel);
    titleLayout->addWidget(m_logoutButton);

    titleLayout->addWidget(m_minimizeButton);
    titleLayout->addWidget(m_maximizeButton);
    titleLayout->addWidget(m_closeButton);

    // ================ 第二层：登录界面 ================
    m_loginWidget = new QWidget();
    m_loginWidget->setStyleSheet("background-color: #333;");
    m_loginWidget->setVisible(true);  // 默认隐藏

    QHBoxLayout *loginLayout = new QHBoxLayout(m_loginWidget);
    loginLayout->setContentsMargins(10, 5, 10, 5);
    loginLayout->setSpacing(5);

    // 登录提示
    m_loginPromptLabel = new QLabel("登录连接摄像头");
    m_loginPromptLabel->setStyleSheet("color: white;");

    // 账号
    m_accountLabel = new QLabel("账号：");
    m_accountLabel->setStyleSheet("color: white;");
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
    m_logoutButton->setVisible(false);  // 初始不显示

    // 弹簧
    QWidget *loginSpacer = new QWidget();
    loginSpacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    // 退出按钮
    m_exitLoginButton_layer1 = new QPushButton("×");
    m_exitLoginButton_layer1->setFixedSize(25, 25);
    m_exitLoginButton_layer1->setStyleSheet("color: white; background-color: #777;");

    // 添加到登录布局
    loginLayout->addWidget(m_loginPromptLabel);
    loginLayout->addSpacing(10);
    loginLayout->addWidget(m_accountLabel);
    loginLayout->addWidget(m_usernameLineEdit);
    loginLayout->addWidget(m_passwordLabel);
    loginLayout->addWidget(m_passwordLineEdit);
    loginLayout->addSpacing(10);
    loginLayout->addWidget(m_loginSubmitButton);
    loginLayout->addWidget(m_registerButton);
    loginLayout->addWidget(loginSpacer);
    loginLayout->addWidget(m_exitLoginButton_layer1);

    // ================ 第三层：已登录界面 ================
    m_loggedInWidget = new QWidget();
    m_loggedInWidget->setStyleSheet("background-color: #333;");
    m_loggedInWidget->setVisible(false);  // 默认隐藏

    QHBoxLayout *loggedInLayout = new QHBoxLayout(m_loggedInWidget);
    loggedInLayout->setContentsMargins(10, 5, 10, 5);
    loggedInLayout->setSpacing(5);

    // 已登录状态下的弹簧
    QWidget *loggedInSpacer = new QWidget();
    loggedInSpacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    // 退出按钮
    m_exitLoginButton_layer2 = new QPushButton("×");
    m_exitLoginButton_layer2->setFixedSize(25, 25);
    m_exitLoginButton_layer2->setStyleSheet("color: white; background-color: #777;");

    // 添加到已登录布局
    loggedInLayout->addWidget(m_usernameLabel);
    loggedInLayout->addWidget(m_logoutButton);
    loggedInLayout->addWidget(loggedInSpacer);
    loggedInLayout->addWidget(m_exitLoginButton_layer2);

    // ================ 整理布局 ================

    // 添加三层到主布局
    mainLayout->addLayout(titleLayout);
    mainLayout->addWidget(m_loginWidget);
    mainLayout->addWidget(m_loggedInWidget);

    // ================ 连接信号槽 ================

    // 连接信号槽
    // - 第一层：基础功能
    connect(m_monitorButton, &QPushButton::clicked, this, &TitleBar::monitorButtonClicked);
    connect(m_playbackButton, &QPushButton::clicked, this, &TitleBar::playbackButtonClicked);
    connect(m_logButton, &QPushButton::clicked, this, &TitleBar::logButtonClicked);
//    connect(m_systemButton, &QPushButton::clicked, this, &TitleBar::systemButtonClicked);
    connect(m_loginButton, &QPushButton::clicked, this, &TitleBar::loginButtonClicked);
    connect(m_logoutButton, &QPushButton::clicked, this, &TitleBar::onLogoutButtonClicked);

    // - 第一层：日志菜单信号
    connect(m_openLogAction, &QAction::triggered, this, &TitleBar::onOpenLogActionTriggered);
    connect(m_clearLogAction, &QAction::triggered, this, &TitleBar::onClearLogActionTriggered);

    // - 第一层：窗口尺寸
    connect(m_minimizeButton, &QPushButton::clicked, this, &TitleBar::minimizeClicked);
    connect(m_maximizeButton, &QPushButton::clicked, this, &TitleBar::maximizeClicked);
    connect(m_closeButton, &QPushButton::clicked, this, &TitleBar::closeClicked);

    // - 第二层：登录相关
    connect(m_loginSubmitButton, &QPushButton::clicked, this, &TitleBar::onLoginSubmit);
    connect(m_exitLoginButton_layer1, &QPushButton::clicked, this, &TitleBar::onExitLoginClicked);
    connect(m_registerButton, &QPushButton::clicked, this, &TitleBar::onRegisterSubmit);

    // - 第三层：登录相关
    connect(m_exitLoginButton_layer2, &QPushButton::clicked, this, &TitleBar::onExitLoginClicked);
}

TitleBar::~TitleBar()
{
}

// 第一层
void TitleBar::setLoginStatus(bool isLoggedIn, const QString& username)
{
    m_isLoggedIn = isLoggedIn;
    m_currentUsername = username;

    if (isLoggedIn) {
        // 已登录状态
        m_loginButton->setVisible(false);      // 始终显示
        m_usernameLabel->setText(QString("欢迎, %1").arg(username));
        m_usernameLabel->setVisible(true);
        m_logoutButton->setVisible(true);
        m_loginWidget->setVisible(false);
        m_loggedInWidget->setVisible(false);  // 默认不显示
        setMinimumHeight(30);
        setFixedHeight(30);
    } else {
        // 未登录状态
        m_loginButton->setVisible(true);
        m_usernameLabel->setVisible(false);
        m_logoutButton->setVisible(false);
        m_loginWidget->setVisible(false);
        m_loggedInWidget->setVisible(false);
        setMinimumHeight(60);
        setFixedHeight(60);
    }
}

void TitleBar::onLoginButtonClicked()
{
    // 显示登录界面
    qDebug() << "登录按钮被点击";
    m_loginWidget->setVisible(true);
    // 调整标题栏高度
    setMinimumHeight(60);
    setFixedHeight(60);

    if (m_isLoggedIn) {
        // 已登录状态：显示已登录界面
        m_loginWidget->setVisible(false);
        m_loggedInWidget->setVisible(true);
    } else {
        // 未登录状态：显示登录表单
        m_loginWidget->setVisible(true);
        m_loggedInWidget->setVisible(false);
    }
}

void TitleBar::onLogoutButtonClicked()
{
    // 退出登录逻辑
    log_info("用户退出登录");
    setLoginStatus(false, "");

    // 更新FFmpeg_Player的登录状态
    FFmpeg_Player *player = qobject_cast<FFmpeg_Player*>(parent());
    if (player) {
        player->setLoggedIn(false);
    }

    emit logoutButtonClicked();
    emit systemMessage("[信息] 已退出登录");
}

void TitleBar::onOpenLogActionTriggered()
{
    // 打开日志文件
    QString logPath = "d:/White_Hat_Course/03_Embedded/CPP_QT_Project/04_Github_Sync_FFmpeg/local/FFmpeg_Player_local/Log/log.txt";
    QFile logFile(logPath);

    if (logFile.exists()) {
        // 使用系统默认程序打开日志文件
        QDesktopServices::openUrl(QUrl::fromLocalFile(logPath));
        log_info("日志文件已打开");
    } else {
        emit systemMessage("[错误] 日志文件不存在");
        log_error("日志文件不存在");
    }
}

void TitleBar::onClearLogActionTriggered()
{
    // 清空日志文件
    QString logPath = "d:/White_Hat_Course/03_Embedded/CPP_QT_Project/04_Github_Sync_FFmpeg/local/FFmpeg_Player_local/Log/log.txt";
    QFile logFile(logPath);

    if (logFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        logFile.close();
        emit systemMessage("[信息] 日志已清空");
        log_info("日志已清空");
    } else {
        emit systemMessage("[错误] 无法清空日志文件");
        log_error("无法清空日志文件");
    }
}

// 第二层
void TitleBar::onLoginSubmit()
{
    // 获取用户名和密码
    QString username = m_usernameLineEdit->text();
    QString password = m_passwordLineEdit->text();

    // 这里添加实际的登录逻辑
    if (!username.isEmpty() && !password.isEmpty()) {
        log_info(QString("登录提交: 用户名 = %1, 密码 = %2").arg(username).arg(password).toUtf8().constData());

        // 测试账户检测 - 用户名和密码都是test时直接登录成功
        if (username == "test" && password == "test") {
            log_info("测试账户登录成功");
            emit systemMessage("[信息] 测试账户登录成功");
//            emit loginButtonClicked();
            setLoginStatus(true, username);
            onExitLoginClicked();

            // 更新FFmpeg_Player的登录状态
            FFmpeg_Player *player = qobject_cast<FFmpeg_Player*>(parent());
            if (player) {
                player->setLoggedIn(true);
            }
            return;  // 直接返回，跳过正常登录流程
        }

//        if(m_login) delete m_login;
        m_login = new Login(this, username, password);

        log_info("登录实例创建完成");

        connect(m_login, &Login::loginSuccess, this, [this, username](){
            emit systemMessage("[信息] 登录成功");
//            emit loginButtonClicked();  // 发送登录成功信号
            setLoginStatus(true, username);
            onExitLoginClicked();       // 登录成功后隐藏登录界面

            // 更新FFmpeg_Player的登录状态
            FFmpeg_Player *player = qobject_cast<FFmpeg_Player*>(parent());
            if (player) {
                player->setLoggedIn(true);
            }
        });

        connect(m_login, &Login::loginFailed, this, [this]() {
            emit systemMessage("[警告] 登录失败，请重新输入用户名和密码");
        });


    } else {
        emit systemMessage("[警告] 提交登录信息失败，用户名或密码为空，请输入用户名和密码");
    }

}

void TitleBar::onRegisterSubmit()
{
//    // 创建并显示注册对话框
//    Register *registerDialog = new Register(this);
//    registerDialog->exec();

    // 获取用户名和密码
    QString username = m_usernameLineEdit->text();
    QString password = m_passwordLineEdit->text();

    // 这里添加实际的登录逻辑
    if (!username.isEmpty() && !password.isEmpty()) {
        log_info(QString("登录提交: 用户名 = %1, 密码 = %2").arg(username).arg(password).toUtf8().constData());

//        if(m_register) delete m_register;
        m_register = new Register(this);

        // 连接信号
        connect(m_register, &Register::systemMessage, this, &TitleBar::systemMessage);
        connect(m_register, &Register::registerSuccess, this, [this](){
            emit systemMessage("[信息] 注册成功！");
            onExitLoginClicked();

            // 更新FFmpeg_Player的登录状态
            // - parent() 返回当前 TitleBar 对象的父对象指针
            // - qobject_cast 尝试将父对象指针安全转换为 FFmpeg_Player* 类型
            // - 只有当父对象确实是 FFmpeg_Player 类型时，转换才会成功并返回有效指针
            // - 代码中的 if (player) 检查确保只有转换成功时才执行后续操作
            FFmpeg_Player *player = qobject_cast<FFmpeg_Player*>(parent());
            if (player) {
                player->setLoggedIn(true);
            }
        });

        connect(m_register, &Register::registerFailed, this, [this](){
            emit systemMessage("[警告] 注册失败，请重试！");
        });

        // 调用注册方法
        m_register->registerUser(username, password);
    } else {
        emit systemMessage("[警告] 提交注册信息失败，用户名或密码为空，请输入用户名和密码");
    }
}

void TitleBar::onExitLoginClicked()
{
    // 隐藏登录和已登录界面
    m_loginWidget->setVisible(false);
    m_loggedInWidget->setVisible(false);
    // 恢复标题栏高度
    setMinimumHeight(30);
    setFixedHeight(30);
}
