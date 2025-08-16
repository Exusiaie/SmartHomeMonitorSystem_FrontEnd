#include "FFmpeg_Player.h"
#include <QDebug>
#include <QIcon>
#include <QStyle>
#include <QFileDialog>

FFmpeg_Player::FFmpeg_Player(QWidget *parent)
    : QWidget(parent)
    , formatContext(nullptr)
    , videoCodecContext(nullptr)
    , audioCodecContext(nullptr)
    , videoStreamIndex(-1)
    , audioStreamIndex(-1)
    , isPlaying(false)
{

    initFFmpeg();   // 初始化FFmpeg
    initUI();       // 初始化UI
}

FFmpeg_Player::~FFmpeg_Player()
{
    // 释放FFmpeg资源
    if (videoCodecContext) {
        avcodec_free_context(&videoCodecContext);
    }
    if (audioCodecContext) {
        avcodec_free_context(&audioCodecContext);
    }
    if (formatContext) {
        avformat_close_input(&formatContext);
    }
}

void FFmpeg_Player::initFFmpeg()
{
    // 注册FFmpeg组件
    av_register_all();
    avcodec_register_all();
    avformat_network_init();

    // 设置日志级别
    av_log_set_level(AV_LOG_INFO);

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
    createTitleBar();
    mainLayout->addWidget(titleBar);

    // 创建中间内容区域布局
    QHBoxLayout *contentLayout = new QHBoxLayout();
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(0);

    // 创建左边栏
    createLeftSideBar();
    contentLayout->addWidget(leftSideBar, 1);

    // 创建中央播放区域
    createCentralWidget();
    contentLayout->addWidget(centralWidget, 5);

    // 创建右边栏
    createRightSideBar();
    contentLayout->addWidget(rightSideBar, 1);

    mainLayout->addLayout(contentLayout);

    // 创建底部栏
    createBottomBar();
    mainLayout->addWidget(bottomBar);

    setLayout(mainLayout);
}

void FFmpeg_Player::createTitleBar()
{
    titleBar = new QWidget(this);
    titleBar->setStyleSheet("background-color: #333; color: white;");
    titleBar->setFixedHeight(30);

    // 创建水平布局
    QHBoxLayout *titleLayout = new QHBoxLayout(titleBar);
    titleLayout->setContentsMargins(5, 0, 5, 0);
    titleLayout->setSpacing(5);

    // 添加图标和标题
    QLabel *iconLabel = new QLabel();
    iconLabel->setPixmap(QPixmap("://icons/player_icon.png").scaled(24, 24));
    titleLabel = new QLabel("FFmpeg视频播放器");
    titleLabel->setStyleSheet("color: white;");

    // 添加弹簧布局
    QWidget *spacer = new QWidget();
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    // 添加常用功能按钮
    monitorButton = new QPushButton("监控");
    monitorButton->setStyleSheet("color: white; background-color: #555;");
    playbackButton = new QPushButton("回放");
    playbackButton->setStyleSheet("color: white; background-color: #555;");
    logButton = new QPushButton("日志");
    logButton->setStyleSheet("color: white; background-color: #555;");
    systemButton = new QPushButton("系统");
    systemButton->setStyleSheet("color: white; background-color: #555;");

    // 添加窗口控制按钮
    minimizeButton = new QPushButton("-");
    minimizeButton->setStyleSheet("color: white; background-color: #555;");
    maximizeButton = new QPushButton("□");
    maximizeButton->setStyleSheet("color: white; background-color: #555;");
    closeButton = new QPushButton("×");
    closeButton->setStyleSheet("color: white; background-color: #e74c3c;");

    // 设置按钮大小
    QList<QPushButton*> buttons = {monitorButton, playbackButton, logButton, systemButton,
                                  minimizeButton, maximizeButton, closeButton};
    for (auto button : buttons) {
        button->setFixedSize(40, 25);
    }

    // 添加到布局
    titleLayout->addWidget(iconLabel);
    titleLayout->addWidget(titleLabel);
    titleLayout->addWidget(spacer);
    titleLayout->addWidget(monitorButton);
    titleLayout->addWidget(playbackButton);
    titleLayout->addWidget(logButton);
    titleLayout->addWidget(systemButton);
    titleLayout->addWidget(minimizeButton);
    titleLayout->addWidget(maximizeButton);
    titleLayout->addWidget(closeButton);

    // 连接信号槽
    connect(minimizeButton, &QPushButton::clicked, this, &QWidget::showMinimized);
    connect(closeButton, &QPushButton::clicked, this, &QWidget::close);
    connect(monitorButton, &QPushButton::clicked, this, &FFmpeg_Player::onMonitorActionTriggered);
    connect(playbackButton, &QPushButton::clicked, this, &FFmpeg_Player::onPlaybackActionTriggered);
    connect(logButton, &QPushButton::clicked, this, &FFmpeg_Player::onLogActionTriggered);
    connect(systemButton, &QPushButton::clicked, this, &FFmpeg_Player::onSystemActionTriggered);
}

void FFmpeg_Player::createLeftSideBar()
{
    leftSideBar = new QWidget(this);
    leftSideBar->setMinimumWidth(200);
    leftSideBar->setMaximumWidth(250);
    leftSideBar->setStyleSheet("background-color: #f0f0f0;");

    // 创建左侧选项卡
    leftTabWidget = new QTabWidget(leftSideBar);
    leftTabWidget->setStyleSheet("QTabWidget::pane { border: 1px solid #ddd; }");

    // 窗口信息列表
    windowInfoList = new QListWidget();
    windowInfoList->addItem("窗口 1");
    windowInfoList->addItem("窗口 2");
    leftTabWidget->addTab(windowInfoList, "窗口信息");

    // 播放列表
    playlist = new QListWidget();
    playlist->addItem("视频 1");
    playlist->addItem("视频 2");
    leftTabWidget->addTab(playlist, "播放列表");

    // 添加布局
    QVBoxLayout *layout = new QVBoxLayout(leftSideBar);
    layout->addWidget(leftTabWidget);
    leftSideBar->setLayout(layout);
}

void FFmpeg_Player::createRightSideBar()
{
    rightSideBar = new QWidget(this);
    rightSideBar->setMinimumWidth(200);
    rightSideBar->setMaximumWidth(250);
    rightSideBar->setStyleSheet("background-color: #f0f0f0;");

    // 创建右侧选项卡
    rightTabWidget = new QTabWidget(rightSideBar);
    rightTabWidget->setStyleSheet("QTabWidget::pane { border: 1px solid #ddd; }");

    // 设备列表
    deviceList = new QListWidget();
    deviceList->addItem("设备 1");
    deviceList->addItem("设备 2");
    rightTabWidget->addTab(deviceList, "设备列表");

    // 系统信息
    systemInfoWidget = new QWidget();
    QVBoxLayout *systemLayout = new QVBoxLayout(systemInfoWidget);
    systemLayout->addWidget(new QLabel("CPU: Intel i5"));
    systemLayout->addWidget(new QLabel("内存: 8GB"));
    systemLayout->addWidget(new QLabel("系统: Windows 10"));
    rightTabWidget->addTab(systemInfoWidget, "系统信息");

    // 添加布局
    QVBoxLayout *layout = new QVBoxLayout(rightSideBar);
    layout->addWidget(rightTabWidget);
    rightSideBar->setLayout(layout);
}

// 实现createBottomBar函数
void FFmpeg_Player::createBottomBar()
{
    bottomBar = new QWidget(this);
    bottomBar->setFixedHeight(60);
    bottomBar->setStyleSheet("background-color: #333; color: white;");

    // 创建底部布局
    QHBoxLayout *bottomLayout = new QHBoxLayout(bottomBar);
    bottomLayout->setContentsMargins(10, 5, 10, 5);

    // 播放控制按钮
    playButton = new QPushButton();
    playButton->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
    playButton->setStyleSheet("background-color: #555; color: white;");
    playButton->setFixedSize(40, 40);

    pauseButton = new QPushButton();
    pauseButton->setIcon(style()->standardIcon(QStyle::SP_MediaPause));
    pauseButton->setStyleSheet("background-color: #555; color: white;");
    pauseButton->setFixedSize(40, 40);
    pauseButton->setEnabled(false);

    stopButton = new QPushButton();
    stopButton->setIcon(style()->standardIcon(QStyle::SP_MediaStop));
    stopButton->setStyleSheet("background-color: #555; color: white;");
    stopButton->setFixedSize(40, 40);
    stopButton->setEnabled(false);

    // 时间显示
    currentTimeLabel = new QLabel("00:00");
    currentTimeLabel->setStyleSheet("color: white;");

    durationLabel = new QLabel("/ 00:00");
    durationLabel->setStyleSheet("color: white;");

    // 进度条
    progressBar = new QSlider(Qt::Horizontal);
    progressBar->setStyleSheet("QSlider::groove:horizontal {background: #555; height: 8px; border-radius: 4px;}"
                               "QSlider::handle:horizontal {background: white; width: 16px; margin: -4px 0; border-radius: 8px;}");
    progressBar->setRange(0, 100);
    progressBar->setValue(0);

    // 音量控制
    volumeButton = new QPushButton();
    volumeButton->setIcon(style()->standardIcon(QStyle::SP_MediaVolume));
    volumeButton->setStyleSheet("background-color: #555; color: white;");
    volumeButton->setFixedSize(30, 30);

    volumeSlider = new QSlider(Qt::Horizontal);
    volumeSlider->setStyleSheet("QSlider::groove:horizontal {background: #555; height: 8px; border-radius: 4px;}"
                                "QSlider::handle:horizontal {background: white; width: 16px; margin: -4px 0; border-radius: 8px;}");
    volumeSlider->setRange(0, 100);
    volumeSlider->setValue(80);
    volumeSlider->setFixedWidth(80);

    // 截图按钮
    screenshotButton = new QPushButton("截图");
    screenshotButton->setStyleSheet("background-color: #555; color: white;");
    screenshotButton->setFixedSize(60, 30);

    // 布局切换
    layoutComboBox = new QComboBox();
    layoutComboBox->addItem("单画面");
    layoutComboBox->addItem("2个竖切");
    layoutComboBox->addItem("2个横切");
    layoutComboBox->setStyleSheet("background-color: #555; color: white;");
    layoutComboBox->setFixedSize(100, 30);

    // 添加到布局
    bottomLayout->addWidget(playButton);
    bottomLayout->addWidget(pauseButton);
    bottomLayout->addWidget(stopButton);
    bottomLayout->addSpacing(10);
    bottomLayout->addWidget(currentTimeLabel);
    bottomLayout->addWidget(durationLabel);
    bottomLayout->addWidget(progressBar, 1);
    bottomLayout->addWidget(volumeButton);
    bottomLayout->addWidget(volumeSlider);
    bottomLayout->addSpacing(10);
    bottomLayout->addWidget(screenshotButton);
    bottomLayout->addSpacing(10);
    bottomLayout->addWidget(layoutComboBox);

    // 连接信号槽
    connect(playButton, &QPushButton::clicked, this, &FFmpeg_Player::onPlayButtonClicked);
    connect(pauseButton, &QPushButton::clicked, this, &FFmpeg_Player::onPauseButtonClicked);
    connect(stopButton, &QPushButton::clicked, this, &FFmpeg_Player::onStopButtonClicked);
    connect(progressBar, &QSlider::valueChanged, this, &FFmpeg_Player::onProgressBarValueChanged);
    connect(volumeSlider, &QSlider::valueChanged, this, &FFmpeg_Player::onVolumeSliderValueChanged);
    connect(screenshotButton, &QPushButton::clicked, this, &FFmpeg_Player::onScreenshotButtonClicked);
    connect(layoutComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &FFmpeg_Player::onLayoutComboBoxCurrentIndexChanged);
}

// 实现createCentralWidget函数
void FFmpeg_Player::createCentralWidget()
{
    centralWidget = new QWidget(this);
    centralWidget->setStyleSheet("background-color: black;");

    // 创建播放窗口
    videoWidget = new QWidget(centralWidget);
    videoWidget->setStyleSheet("background-color: black;");
    videoWidget->setMinimumSize(640, 360);

    // 创建中央布局
    QVBoxLayout *centralLayout = new QVBoxLayout(centralWidget);
    centralLayout->addWidget(videoWidget);

    // 添加文件打开按钮
    openFileButton = new QPushButton("打开文件");
    openFileButton->setStyleSheet("background-color: #555; color: white;");
    openFileButton->setFixedSize(100, 40);

    // 添加按钮布局
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(openFileButton, 0, Qt::AlignCenter);
    centralLayout->addLayout(buttonLayout);

    // 连接信号槽
    connect(openFileButton, &QPushButton::clicked, this, &FFmpeg_Player::onOpenFileButtonClicked);
}

// 实现播放控制槽函数
void FFmpeg_Player::onPlayButtonClicked()
{
    play();
    playButton->setEnabled(false);
    pauseButton->setEnabled(true);
    stopButton->setEnabled(true);
}

void FFmpeg_Player::onPauseButtonClicked()
{
    pause();
    pauseButton->setEnabled(false);
    playButton->setEnabled(true);
}

void FFmpeg_Player::onStopButtonClicked()
{
    stop();
    stopButton->setEnabled(false);
    playButton->setEnabled(true);
    pauseButton->setEnabled(false);
}

void FFmpeg_Player::onVolumeSliderValueChanged(int value)
{
    setVolume(value);
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
    seek(value);
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
        openVideoFile(fileName.toUtf8().constData());
    }
}

// 实现视频文件打开函数
void FFmpeg_Player::openVideoFile(const char *fileName)
{
    // 关闭之前的文件
    if (formatContext) {
        avformat_close_input(&formatContext);
        formatContext = nullptr;
    }

    // 打开文件
    int ret = avformat_open_input(&formatContext, fileName, nullptr, nullptr);
    if (ret != 0) {
        char errbuf[1024] = {0};
        av_strerror(ret, errbuf, sizeof(errbuf));
        qDebug() << "无法打开文件: " << errbuf;
        return;
    }

    // 获取流信息
    ret = avformat_find_stream_info(formatContext, nullptr);
    if (ret < 0) {
        char errbuf[1024] = {0};
        av_strerror(ret, errbuf, sizeof(errbuf));
        qDebug() << "无法获取流信息: " << errbuf;
        avformat_close_input(&formatContext);
        formatContext = nullptr;
        return;
    }

    // 查找视频流和音频流
    videoStreamIndex = -1;
    audioStreamIndex = -1;
    for (int i = 0; i < formatContext->nb_streams; i++) {
        if (formatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoStreamIndex = i;
        } else if (formatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            audioStreamIndex = i;
        }
    }

    // 打印流信息
    qDebug() << "视频流索引: " << videoStreamIndex;
    qDebug() << "音频流索引: " << audioStreamIndex;

    // 设置进度条
    int64_t duration = formatContext->duration / AV_TIME_BASE;
    progressBar->setRange(0, duration);
    durationLabel->setText(QString("/ %1:%2").arg(duration / 60).arg(duration % 60, 2, 10, QLatin1Char('0')));

    // 初始化解码器等后续工作将在play函数中完成
}

// 实现播放功能
void FFmpeg_Player::play()
{
    if (!formatContext || isPlaying) {
        return;
    }

    isPlaying = true;

    // 初始化视频解码器
    if (videoStreamIndex >= 0) {
        AVCodecParameters *codecParams = formatContext->streams[videoStreamIndex]->codecpar;
        const AVCodec *codec = avcodec_find_decoder(codecParams->codec_id);
        if (!codec) {
            qDebug() << "无法找到视频解码器";
            return;
        }

        videoCodecContext = avcodec_alloc_context3(codec);
        if (!videoCodecContext) {
            qDebug() << "无法分配视频解码器上下文";
            return;
        }

        if (avcodec_parameters_to_context(videoCodecContext, codecParams) < 0) {
            qDebug() << "无法复制 codec 参数到上下文";
            return;
        }

        if (avcodec_open2(videoCodecContext, codec, nullptr) < 0) {
            qDebug() << "无法打开视频解码器";
            return;
        }
    }

    // 初始化音频解码器 (类似视频解码器初始化)
    // ...

    // 启动解码线程
    // 这里应该启动一个线程来进行视频解码和播放
    // 简化版本中，我们可以在这里添加解码循环

    qDebug() << "开始播放";
}

void FFmpeg_Player::pause()
{
    if (!isPlaying) {
        return;
    }

    isPlaying = false;
    qDebug() << "暂停播放";
}

void FFmpeg_Player::stop()
{
    if (!formatContext) {
        return;
    }

    isPlaying = false;

    // 释放解码器资源
    if (videoCodecContext) {
        avcodec_free_context(&videoCodecContext);
        videoCodecContext = nullptr;
    }
    if (audioCodecContext) {
        avcodec_free_context(&audioCodecContext);
        audioCodecContext = nullptr;
    }

    // 重置进度条
    progressBar->setValue(0);
    currentTimeLabel->setText("00:00");

    qDebug() << "停止播放";
}

void FFmpeg_Player::seek(qint64 position)
{
    if (!formatContext) {
        return;
    }

    int64_t seekPos = position * AV_TIME_BASE;
    av_seek_frame(formatContext, -1, seekPos, AVSEEK_FLAG_BACKWARD);
    currentTimeLabel->setText((QString("%1:%2")).arg(position / 60).arg(position % 60, 2, 10, QLatin1Char('0')));
}

void FFmpeg_Player::setVolume(int volume)
{
    // 实现音量控制
    qDebug() << "设置音量: " << volume;
}

void FFmpeg_Player::takeScreenshot()
{
    // 实现截图功能
    qDebug() << "截图";
}

void FFmpeg_Player::switchLayout(int layoutType)
{
    // 实现布局切换
    qDebug() << "切换布局: " << layoutType;
}
