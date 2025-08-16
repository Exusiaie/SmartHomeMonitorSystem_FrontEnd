#include "centralwidget.h"

CentralWidget::CentralWidget(QWidget *parent) : QWidget(parent)
{
    // 设置样式
    setStyleSheet("background-color: black;");

    // 创建播放窗口
    videoWidget = new QWidget(this);
    videoWidget->setStyleSheet("background-color: black;");
    videoWidget->setMinimumSize(640, 360);

    // 创建中央布局
    QVBoxLayout *centralLayout = new QVBoxLayout(this);
    centralLayout->addWidget(videoWidget);

    // 添加文件打开按钮
    openFileButton = new QPushButton("打开文件");
    openFileButton->setStyleSheet("background-color: #555; color: white;");
    openFileButton->setFixedSize(100, 40);

    // 添加按钮布局
    QHBoxLayout *buttonLayout = new QHBoxLayout(videoWidget);
    buttonLayout->addWidget(openFileButton, 0, Qt::AlignCenter);

    // 连接信号槽
    connect(openFileButton, &QPushButton::clicked, this, &CentralWidget::openFileButtonClicked);
    setLayout(centralLayout);
}

CentralWidget::~CentralWidget()
{
}

QWidget* CentralWidget::getVideoWidget() const
{
    return videoWidget;
}
