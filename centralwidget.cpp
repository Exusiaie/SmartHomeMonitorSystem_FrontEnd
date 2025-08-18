#include "centralwidget.h"

CentralWidget::CentralWidget(QWidget *parent) : QWidget(parent)
{
    // 设置样式
    setStyleSheet("background-color: black;");

    // 创建播放窗口
    m_videoWidget = new  VideoWidget(this);  // 使用VideoWidget
    m_videoWidget->setStyleSheet("background-color: black;");
    m_videoWidget->setMinimumSize(640, 360);

    // 创建中央布局
    QVBoxLayout *centralLayout = new QVBoxLayout(this);
    centralLayout->addWidget(m_videoWidget);

    // 添加文件打开按钮
    m_openFileButton = new QPushButton("打开文件");
    m_openFileButton->setStyleSheet("background-color: #555; color: white;");
    m_openFileButton->setFixedSize(100, 40);

    // 添加按钮布局
    QHBoxLayout *buttonLayout = new QHBoxLayout(m_videoWidget);
    buttonLayout->addWidget(m_openFileButton, 0, Qt::AlignCenter);

    // 连接信号槽
    connect(m_openFileButton, &QPushButton::clicked, this, &CentralWidget::openFileButtonClicked);
    setLayout(centralLayout);
}

CentralWidget::~CentralWidget()
{
}

VideoWidget* CentralWidget::getVideoWidget() const
{
    return m_videoWidget;
}
