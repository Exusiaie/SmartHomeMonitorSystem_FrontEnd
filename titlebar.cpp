#include "titlebar.h"
#include <QHBoxLayout>
#include <QStyle>
#include <QIcon>

TitleBar::TitleBar(QWidget *parent)
    : QWidget(parent)
{
    setStyleSheet("background-color: #333; color: white;");
    setFixedHeight(30);

    // 创建水平布局
    QHBoxLayout *titleLayout = new QHBoxLayout(this);
    titleLayout->setContentsMargins(5, 0, 5, 0);
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

    // 添加到布局
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

    // 连接信号槽
    // - 基础功能
    connect(m_monitorButton, &QPushButton::clicked, this, &TitleBar::monitorButtonClicked);
    connect(m_playbackButton, &QPushButton::clicked, this, &TitleBar::playbackButtonClicked);
    connect(m_logButton, &QPushButton::clicked, this, &TitleBar::logButtonClicked);
    connect(m_systemButton, &QPushButton::clicked, this, &TitleBar::systemButtonClicked);
    connect(m_loginButton, &QPushButton::clicked, this, &TitleBar::loginButtonClicked);

    // - 窗口尺寸
    connect(m_minimizeButton, &QPushButton::clicked, this, &TitleBar::minimizeClicked);
    connect(m_maximizeButton, &QPushButton::clicked, this, &TitleBar::maximizeClicked);
    connect(m_closeButton, &QPushButton::clicked, this, &TitleBar::closeClicked);
}

TitleBar::~TitleBar()
{
}
