#include "centralwidget.h"
#include "qss.h"
#include "log.h"

CentralWidget::CentralWidget(QWidget *parent) : QWidget(parent)
{
    // 设置样式
//    setStyleSheet("background-color: black;");

    // 创建播放窗口
    m_videoWidget = new  VideoWidget(this);  // 使用VideoWidget
//    m_videoWidget->setStyleSheet("background-color: black;");
    m_videoWidget->setMinimumSize(640, 360);

    // 创建中央布局
    QVBoxLayout *centralLayout = new QVBoxLayout(this);
    centralLayout->addWidget(m_videoWidget);
    // 消除中央布局的间距和边距
    centralLayout->setSpacing(0);
    centralLayout->setContentsMargins(0, 0, 0, 0);

    // 创建按钮容器widget
    QWidget *buttonWidget = new QWidget(this);
    buttonWidget->setStyleSheet("background-color: transparent;");
    QVBoxLayout *buttonLayout = new QVBoxLayout(buttonWidget);
    // 消除按钮布局的间距
    buttonLayout->setSpacing(60);
//    buttonLayout->setContentsMargins(0, 0, 0, 0);

    // 添加文件打开按钮
    m_openFileButton = new QPushButton("打开本地录像");
    m_openFileButton->setStyleSheet("background-color: #555; color: white;");
    m_openFileButton->setFixedSize(100, 40);
    buttonLayout->addWidget(m_openFileButton);

    // 新增：添加URL打开按钮
    m_openUrlButton = new QPushButton("连接远程摄像头");
    m_openUrlButton->setStyleSheet("background-color: #555; color: white;");
    m_openUrlButton->setFixedSize(100, 40);
    buttonLayout->addWidget(m_openUrlButton);

    // 添加按钮布局
    buttonLayout->setAlignment(Qt::AlignCenter);
    buttonWidget->setLayout(buttonLayout);

    // 将按钮widget添加到视频窗口
    QVBoxLayout *videoLayout = new QVBoxLayout(m_videoWidget);
    videoLayout->addWidget(buttonWidget, 0, Qt::AlignCenter);
    // 消除视频布局的间距和边距
    videoLayout->setSpacing(0);
    videoLayout->setContentsMargins(0, 0, 0, 0);

    // 连接信号槽
    connect(m_openFileButton, &QPushButton::clicked, this, &CentralWidget::openFileButtonClicked);
    connect(m_openUrlButton, &QPushButton::clicked, this, &CentralWidget::openUrlButtonClicked);
    setLayout(centralLayout);

    this->setStyleSheet(AppStyle::MAIN_STYLE);
}

CentralWidget::~CentralWidget()
{
}

// 新增：实现显示/隐藏按钮方法
void CentralWidget::showButtons(bool show)
{
    m_openFileButton->setVisible(show);
    m_openUrlButton->setVisible(show);
}


VideoWidget* CentralWidget::getVideoWidget() const
{
    return m_videoWidget;
}
