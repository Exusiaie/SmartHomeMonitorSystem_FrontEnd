#include "FFmpeg_Player.h"
#include "leftsidebar.h"
#include "rightsidebar.h"
#include "centralwidget.h"

#include <QDebug>
#include <QIcon>
#include <QStyle>
#include <QFileDialog>

FFmpeg_Player::FFmpeg_Player(QWidget *parent)
    : QWidget(parent)
{
    initFFmpeg();   // 初始化FFmpeg
    initUI();       // 初始化UI
}

FFmpeg_Player::~FFmpeg_Player()
{
}

void FFmpeg_Player::initFFmpeg()
{

    // 创建播放器核心和工具
    playerCore = new PlayerCore(this);
    playerTools = new PlayerTools(this);

    qDebug() << "FFmpeg initialized successfully";
}

void FFmpeg_Player::initUI()
{
    // 设置窗口标题和大小
    setWindowTitle("FFmpeg视频播放器");
    resize(1280, 720);

    // 创建主布局
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // 创建标题栏
    titleBar = new TitleBar(this);
    mainLayout->addWidget(titleBar);

    // 连接标题栏信号
    connect(titleBar, &TitleBar::minimizeClicked, this, &FFmpeg_Player::onMinimizeClicked);
    connect(titleBar, &TitleBar::closeClicked, this, &FFmpeg_Player::onCloseClicked);
    connect(titleBar, &TitleBar::monitorButtonClicked, this, &FFmpeg_Player::onMonitorActionTriggered);
    connect(titleBar, &TitleBar::playbackButtonClicked, this, &FFmpeg_Player::onPlaybackActionTriggered);
    connect(titleBar, &TitleBar::logButtonClicked, this, &FFmpeg_Player::onLogActionTriggered);
    connect(titleBar, &TitleBar::systemButtonClicked, this, &FFmpeg_Player::onSystemActionTriggered);

    // 创建中间内容区域布局
    QHBoxLayout *contentLayout = new QHBoxLayout();
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(0);

    // 创建左边栏
    leftSideBar = new LeftSideBar(this);
    contentLayout->addWidget(leftSideBar, 1);

    // 创建中央播放区域
    centralWidget = new CentralWidget(this);
    contentLayout->addWidget(centralWidget, 5);
    connect(centralWidget, &CentralWidget::openFileButtonClicked, this, &FFmpeg_Player::onOpenFileButtonClicked);

    // 创建右边栏
    rightSideBar = new RightSideBar(this);
    contentLayout->addWidget(rightSideBar, 1);
    mainLayout->addLayout(contentLayout);

    // 创建底部栏
    bottomBar = new BottomBar(this);
    mainLayout->addWidget(bottomBar);

    // 设置底部栏的核心和工具指针
    bottomBar->setPlayerCore(playerCore);
    bottomBar->setPlayerTools(playerTools);

    // 连接底部栏信号
    connect(bottomBar, &BottomBar::volumeValueChanged, this, &FFmpeg_Player::onVolumeSliderValueChanged);
    connect(bottomBar, &BottomBar::screenshotButtonClicked, this, &FFmpeg_Player::onScreenshotButtonClicked);
    connect(bottomBar, &BottomBar::layoutIndexChanged, this, &FFmpeg_Player::onLayoutComboBoxCurrentIndexChanged);
    connect(bottomBar, &BottomBar::progressValueChanged, this, &FFmpeg_Player::onProgressBarValueChanged);

    // 连接PlayerCore信号
    connect(playerCore, &PlayerCore::playStateChanged, this, &FFmpeg_Player::onPlayStateChanged);
    connect(playerCore, &PlayerCore::stopStateChanged, this, &FFmpeg_Player::onStopStateChanged);
    connect(playerCore, &PlayerCore::durationChanged, this, &FFmpeg_Player::onDurationChanged);
    connect(playerCore, &PlayerCore::positionChanged, this, &FFmpeg_Player::onPositionChanged);
    connect(playerCore, &PlayerCore::errorOccurred, this, &FFmpeg_Player::onErrorOccurred);

    // 连接PlayerTools信号
    connect(playerTools, &PlayerTools::layoutChanged, this, &FFmpeg_Player::switchLayout);

    setLayout(mainLayout);
}

void FFmpeg_Player::onVolumeSliderValueChanged(int value)
{
    // 这里可以添加额外的音量处理逻辑
    qDebug() << "音量改变: " << value;
}

void FFmpeg_Player::onScreenshotButtonClicked()
{
    takeScreenshot();
}

void FFmpeg_Player::onLayoutComboBoxCurrentIndexChanged(int index)
{
    switchLayout(index);
}

void FFmpeg_Player::onProgressBarValueChanged(int value)
{
    // 这里可以添加额外的进度处理逻辑
    qDebug() << "进度改变: " << value;
}

// 实现菜单动作槽函数
void FFmpeg_Player::onMonitorActionTriggered()
{
    qDebug() << "监控按钮被点击";
    // 实现监控功能
}

void FFmpeg_Player::onPlaybackActionTriggered()
{
    qDebug() << "回放按钮被点击";
    // 实现回放功能
}

void FFmpeg_Player::onLogActionTriggered()
{
    qDebug() << "日志按钮被点击";
    // 实现日志功能
}

void FFmpeg_Player::onSystemActionTriggered()
{
    qDebug() << "系统按钮被点击";
    // 实现系统功能
}

// 实现文件打开槽函数
void FFmpeg_Player::onOpenFileButtonClicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, "打开视频文件", "", "视频文件 (*.mp4 *.avi *.mkv *.mov);;所有文件 (*.*)");
    if (!fileName.isEmpty()) {
        // 打开视频文件
        playerCore->openFile(fileName);
    }
}

void FFmpeg_Player::takeScreenshot()
{
    if (playerTools) {
        playerTools->takeScreenshot(centralWidget->getVideoWidget());
    }
}

void FFmpeg_Player::switchLayout(int layoutType)
{
    // 实现布局切换
    qDebug() << "切换布局: " << layoutType;
    // 这里可以根据layoutType改变视频窗口的布局
}

void FFmpeg_Player::onMinimizeClicked()
{
    showMinimized();
}

void FFmpeg_Player::onCloseClicked()
{
    close();
}

void FFmpeg_Player::onPlayStateChanged(bool isPlaying)
{
    bottomBar->updatePlayState(isPlaying);
}

void FFmpeg_Player::onStopStateChanged()
{
    bottomBar->updateStopState();
}

void FFmpeg_Player::onDurationChanged(int64_t duration)
{
    bottomBar->setDuration(duration);
}

void FFmpeg_Player::onPositionChanged(qint64 position)
{
    bottomBar->setPosition(position);
}

void FFmpeg_Player::onErrorOccurred(const QString &errorString)
{
    qDebug() << "错误: " << errorString;
    // 这里可以添加错误处理逻辑，如显示错误对话框等
}
