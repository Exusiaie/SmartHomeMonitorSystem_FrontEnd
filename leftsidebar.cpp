#include "leftsidebar.h"

LeftSideBar::LeftSideBar(QWidget *parent) : QWidget(parent)
{
    // 设置样式和大小
    setMinimumWidth(200);
    setMaximumWidth(250);
    setStyleSheet("background-color: #f0f0f0;");

    // 创建选项卡
    tabWidget = new QTabWidget(this);
    tabWidget->setStyleSheet("QTabWidget::pane { border: 1px solid #ddd; }");

    // 窗口信息列表
    windowInfoList = new QListWidget();
    windowInfoList->addItem("窗口 1");
    windowInfoList->addItem("窗口 2");
    tabWidget->addTab(windowInfoList, "窗口信息");

    // 播放列表
    playlist = new QListWidget();
    playlist->addItem("视频 1");
    playlist->addItem("视频 2");
    tabWidget->addTab(playlist, "播放列表");

    // 添加布局
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(tabWidget);
    setLayout(layout);
}

LeftSideBar::~LeftSideBar()
{
}

QListWidget* LeftSideBar::getPlaylist() const
{
    return playlist;
}

QListWidget* LeftSideBar::getWindowInfoList() const
{
    return windowInfoList;
}
