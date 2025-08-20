#include "FFmpeg_Player.h"
#include "leftsidebar.h"
#include "rightsidebar.h"
#include "centralwidget.h"
#include "qss.h"
#include "log.h"

#include <QDebug>
#include <QIcon>
#include <QStyle>
#include <QFileDialog>

FFmpeg_Player::FFmpeg_Player(QWidget *parent)
    : CFramelessWidget(parent)
    , m_isLoggedIn(false)  // 初始化为未登录
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

    connect(playerCore, &PlayerCore::frameDecoded, this, [this](const QImage &image) {
        if(image.isNull())
        {
            log_info("收到空帧，跳过渲染");
            return;
        }

        // - QPixmap
        // - 此处为了维持正常播放速度
        QImage scaledImage = image.scaled(centralWidget->getVideoWidget()->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        centralWidget->getVideoWidget()->setImage(scaledImage);
    });

    qDebug() << "FFmpeg initialized successfully";
}

void FFmpeg_Player::initUI()
{
    // 设置窗口标题和大小
    setWindowTitle("FFmpeg视频播放器");
    resize(1280, 720);

    // 设置无边框 + 模态
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);

    // 创建主布局
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // 创建标题栏
    titleBar = new TitleBar(this);
    mainLayout->addWidget(titleBar);

    // 连接标题栏信号
    connect(titleBar, &TitleBar::monitorButtonClicked, this, &FFmpeg_Player::onOpenUrlButtonClicked);
    connect(titleBar, &TitleBar::playbackButtonClicked, this, &FFmpeg_Player::onOpenFileButtonClicked);
    connect(titleBar, &TitleBar::logButtonClicked, this, &FFmpeg_Player::onLogActionTriggered);
//    connect(titleBar, &TitleBar::systemButtonClicked, this, &FFmpeg_Player::onSystemActionTriggered);
    connect(titleBar, &TitleBar::logoutButtonClicked, this, [=](){
        m_isLoggedIn = false;
    });

    connect(titleBar, &TitleBar::minimizeClicked, this, &FFmpeg_Player::onMinimizeClicked);
    connect(titleBar, &TitleBar::maximizeClicked, this, &FFmpeg_Player::onMaximizeClicked);
    connect(titleBar, &TitleBar::closeClicked, this, &FFmpeg_Player::onCloseClicked);

    // 连接登录逻辑
    connect(titleBar, &TitleBar::systemMessage, this, [=](const QString& msg){
        onSystemMessageToRightBar(msg);
    });

    // 创建中间内容区域布局
    QHBoxLayout *contentLayout = new QHBoxLayout();
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(0);

    // 创建左边栏
    leftSideBar = new LeftSideBar(this);
    leftSideBar->setMainWindow(this);  // 设置主窗口指针用于截屏
    contentLayout->addWidget(leftSideBar, 1);

    // 创建左侧边栏 -> 截图
    connect(playerCore, &PlayerCore::playbackStarted, leftSideBar, &LeftSideBar::onPlaybackStarted);

    // 创建左侧边栏 -> 播放列表
    connect(leftSideBar, &LeftSideBar::playRequested, this, [this](const QString &path) {
        if (path.startsWith("http") || path.startsWith("rtsp")) {
            playerCore->openUrl(path);
        } else {
            playerCore->openFile(path);
        }
    });

    connect(playerCore, &PlayerCore::playbackFinished,
            leftSideBar, &LeftSideBar::onPlaybackFinished);

    // 创建中央播放区域
    centralWidget = new CentralWidget(this);
    contentLayout->addWidget(centralWidget, 5);
    connect(centralWidget, &CentralWidget::openFileButtonClicked, this, &FFmpeg_Player::onOpenFileButtonClicked);
    connect(centralWidget, &CentralWidget::openUrlButtonClicked, this, &FFmpeg_Player::onOpenUrlButtonClicked);

    // 创建右边栏
    rightSideBar = new RightSideBar(this);
    contentLayout->addWidget(rightSideBar, 1);
    mainLayout->addLayout(contentLayout);

    connect(this, &FFmpeg_Player::systemMessageToRightBar, this, [=](){
        onSystemMessageToRightBar("[警告] 读取摄像头属于联网功能, 请登录后使用.");
    });

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

    // 连接底部栏 -> PlayerCore信号
    connect(playerCore, &PlayerCore::playStateChanged, this, &FFmpeg_Player::onPlayStateChanged);
    connect(playerCore, &PlayerCore::stopStateChanged, this, &FFmpeg_Player::onStopStateChanged);
    connect(playerCore, &PlayerCore::durationChanged, this, &FFmpeg_Player::onDurationChanged);
    connect(playerCore, &PlayerCore::positionChanged, this, &FFmpeg_Player::onPositionChanged);
    connect(playerCore, &PlayerCore::errorOccurred, this, &FFmpeg_Player::onErrorOccurred);
    connect(playerCore, &PlayerCore::clearVideoDisplay, centralWidget->getVideoWidget(), &VideoWidget::clearImage);
    // 连接底部栏 -> PlayerTools信号
    connect(playerTools, &PlayerTools::layoutChanged, this, &FFmpeg_Player::switchLayout);

    // 设置布局
    setLayout(mainLayout);

    // 应用全局样式
     this->setStyleSheet(AppStyle::MAIN_STYLE);
    QPalette palette;
    QPixmap backgroundPixmap(":/Resource/background.png");
    // 缩放图片以适应窗口
    QPixmap scaledPixmap = backgroundPixmap.scaled(this->size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
    palette.setBrush(QPalette::Window, QBrush(scaledPixmap));
    this->setPalette(palette);
}

// 底部栏
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

////// 菜单栏
//void FFmpeg_Player::onMonitorActionTriggered()
//{
//    qDebug() << "监控按钮被点击";
//    // 实现监控功能
//}

//void FFmpeg_Player::onPlaybackActionTriggered()
//{
//    qDebug() << "回放按钮被点击";
//    // 实现回放功能
//}

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

// 中央布局
void FFmpeg_Player::onOpenFileButtonClicked()
{
    // 1. 停止当前播放
    if (playerCore->isPlaying()) {
        playerCore->stop();
        log_info("已停止当前播放");
    }

    // 2. 选择多个文件
    QStringList files = QFileDialog::getOpenFileNames(
        this,
        "选择媒体文件",
        "",
        "视频文件 (*.mp4 *.avi *.mkv *.mov *.flv);;所有文件 (*.*)"
    );

    if (files.isEmpty()) {
        log_info("用户取消选择");
        return;
    }

    // 3. 清空现有播放列表
    leftSideBar->clearPlaylist();
    log_info(QString("已清空播放列表，准备添加 %1 个文件").arg(files.size()));

    // 4. 添加所有文件到播放列表
    for (const auto &file : files) {
        leftSideBar->onAddMediaRequested(file);
    }

    // 5. 自动播放第一个文件
    QString firstFile = files.first();
    if (playerCore->openFile(firstFile)) {
        QFileInfo fileInfo(firstFile);
        leftSideBar->setCurrentMediaName(fileInfo.fileName());
        log_info(QString("开始播放: %1").arg(fileInfo.fileName()));
    } else {
        log_error("打开首文件失败");
        onSystemMessageToRightBar("[错误] 无法播放: " + QFileInfo(firstFile).fileName());
    }
}

void FFmpeg_Player::onOpenUrlButtonClicked()
{
    if (!isLoggedIn()) {
        // 未登录状态：更新系统信息并切换到系统标签页
        log_info("执行未登录状态代码");
        onSystemMessageToRightBar("[警告] 读取摄像头属于联网功能, 请登录后使用.");
    } else {
        // 检查是否正在播放，如果是则停止
        if (playerCore->isPlaying()) {
            log_info("停止当前播放");
            playerCore->close();
        }

        // 已登录状态：执行原有播放逻辑
        log_info("执行登录状态代码");
        if (!playerCore->openUrl("rtsp://admin:admin@192.168.5.222/live/chn=0")) {
            log_error("打开URL失败");
            onSystemMessageToRightBar("[错误] 无法连接到摄像头");
        }

        QString url = "rtsp://admin:admin@192.168.5.222/live/chn=0";
        leftSideBar->setCurrentMediaName(url);
    }
}

// 顶部栏
void FFmpeg_Player::onMinimizeClicked()
{
    log_info("执行onMinimizeClicked()");
    showMinimized();
}

void FFmpeg_Player::onMaximizeClicked()
{
    log_info("执行onMaximizeClicked()");
    toggleFullscreen();
}

void FFmpeg_Player::onCloseClicked()
{
    close();
}

void FFmpeg_Player::toggleFullscreen()
{
    log_info("执行toggleFullscreen()");

    static bool isFullscreen = false;
    isFullscreen = !isFullscreen;

    if (isFullscreen) {
        // 进入全屏模式
        showFullScreen();
        // 隐藏侧边栏和底部栏
//        leftSideBar->hide();
//        rightSideBar->hide();
//        bottomBar->hide();
//        // 最大化视频窗口
//        centralWidget->getVideoWidget()->setMinimumSize(size());
    } else {
        // 退出全屏模式
        showNormal();
        //        // 恢复视频窗口大小
        //        centralWidget->getVideoWidget()->setMinimumSize(640, 360);
        // 显示侧边栏和底部栏
//        leftSideBar->show();
//        rightSideBar->show();
//        bottomBar->show();
    }
}

void FFmpeg_Player::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);

    // 重新缩放背景图片
    QPalette palette = this->palette();
    QPixmap backgroundPixmap(":/Resource/background.png");
    QPixmap scaledPixmap = backgroundPixmap.scaled(this->size(),
                                                  Qt::KeepAspectRatioByExpanding,
                                                  Qt::SmoothTransformation);
    palette.setBrush(QPalette::Window, QBrush(scaledPixmap));
    this->setPalette(palette);
}

bool FFmpeg_Player::isLoggedIn() const
{
    return m_isLoggedIn;
}

void FFmpeg_Player::setLoggedIn(bool loggedIn)
{
    m_isLoggedIn = loggedIn;
    // 同步更新标题栏的登录状态
    if (titleBar) {
        if (loggedIn) {
            // 这里假设我们有一个方法获取当前用户名
            // 实际应用中可能需要从登录管理器或配置中获取
            QString username = "test";
            titleBar->setLoginStatus(true, username);
        } else {
            titleBar->setLoginStatus(false, "");
        }
    }
}

void FFmpeg_Player::onPlayStateChanged(bool isPlaying)
{
    bottomBar->updatePlayState(isPlaying);      // 更改播放状态
    centralWidget->showButtons(!isPlaying);     // 显示/隐藏按钮
}

void FFmpeg_Player::onStopStateChanged()
{
    bottomBar->updateStopState();
    centralWidget->showButtons(true);           // 停止播放显示按钮
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

// 右边栏
// 错误处理函数，又能连接信号槽，又能当作普通函数调用
void FFmpeg_Player::onSystemMessageToRightBar(const QString& msg)
{
    rightSideBar->switchToSystemInfoTab();
    rightSideBar->updateSystemInfo(msg);
}
