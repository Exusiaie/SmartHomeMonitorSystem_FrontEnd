#include "leftsidebar.h"
#include "qss.h"
#include "log.h"
#include <QScreen>
#include <QApplication>
#include <QDateTime>
#include <QFile>
#include <QMessageBox>
#include <QLabel>
#include <QMenu>

LeftSideBar::LeftSideBar(QWidget *parent)
    : QWidget(parent)
    , m_mainWindow(nullptr)
    , m_playlist(new PlaylistManager(this))
{
    // 设置样式和大小
    setMinimumWidth(200);
    setMaximumWidth(250);

    // 创建选项卡
    tabWidget = new QTabWidget(this);

    // 播放列表
    playlist = new QListWidget();
    playlist->addItem("视频 1");
    playlist->addItem("视频 2");
    tabWidget->addTab(playlist, "播放列表");

    // - 播放队列功能初始化
    setupPlaylistTab();
//    addPlaylistItem();

    connect(m_playlist, &PlaylistManager::playlistChanged, this, [this](){
            setupPlaylistTab(); // 播放列表更新时自动刷新
        });

    // 窗口信息列表
    windowInfoList = new QListWidget();
    windowInfoList->setIconSize(QSize(100, 60));  // 设置图标大小
    tabWidget->addTab(windowInfoList, "窗口信息");

    // - 截图初始化定时器（但不在这里打开）
    m_screenshotTimer = new QTimer(this);
    connect(m_screenshotTimer, &QTimer::timeout, this, &LeftSideBar::captureScreenshot);

    // - 创建截图文件夹
    createScreenshotFolder();

    // 添加布局
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(tabWidget);
    setLayout(layout);

    // 设置样式
    this->setStyleSheet(AppStyle::MAIN_STYLE);
}

LeftSideBar::~LeftSideBar()
{
    if (m_screenshotTimer) {
        m_screenshotTimer->stop();
        delete m_screenshotTimer;
    }
}

QListWidget* LeftSideBar::getPlaylist() const
{
    return playlist;
}

QListWidget* LeftSideBar::getWindowInfoList() const
{
    return windowInfoList;
}

void LeftSideBar::setMainWindow(QWidget *mainWindow)
{
    m_mainWindow = mainWindow;
    if (m_mainWindow) {
        log_info("主窗口已设置，等待播放开始信号");
    } else {
        m_screenshotTimer->stop();
        log_info("截图定时器已停止");
    }
}

void LeftSideBar::setCurrentMediaName(const QString& name)
{
    m_currentMediaName = name;
}

void LeftSideBar::captureScreenshot()
{
    if (!m_mainWindow) return;

    // 截取主窗口
    QPixmap screenshot = m_mainWindow->grab();

    // 保存截图到文件
    saveScreenshot(screenshot);

    // 创建列表项
    QListWidgetItem *item = new QListWidgetItem();

    // 设置缩略图
    QPixmap thumbnail = screenshot.scaled(100, 60, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    item->setIcon(QIcon(thumbnail));

    // 设置时间戳
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
    item->setText(timestamp);

    // 头插法插入到列表头部
    windowInfoList->insertItem(0, item);

    // 限制列表大小
    while (windowInfoList->count() > MAX_SCREENSHOTS) {
        delete windowInfoList->takeItem(windowInfoList->count() - 1);
    }

    log_info(QString("已添加新截图，当前截图数量: %1").arg(windowInfoList->count()));
}

//void LeftSideBar::captureScreenshot()
//{
//    if (!m_mainWindow) return;

//    // 截取主窗口
//    QPixmap screenshot = m_mainWindow->grab();

//    // 保存截图到文件
//    saveScreenshot(screenshot);

//    // 创建列表项
//    QListWidgetItem *item = new QListWidgetItem();

//    // 设置缩略图
//    QPixmap thumbnail = screenshot.scaled(100, 60, Qt::KeepAspectRatio, Qt::SmoothTransformation);
//    item->setIcon(QIcon(thumbnail));

//    if(m_currentMediaName != nullptr)
//    {
//        item->setText(m_currentMediaName);
//    }

//    // 设置时间戳
//    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
//    item->setText(timestamp);

//    // 头插法插入到列表头部
//    windowInfoList->insertItem(0, item);

//    // 限制列表大小
//    while (windowInfoList->count() > MAX_SCREENSHOTS) {
//        delete windowInfoList->takeItem(windowInfoList->count() - 1);
//    }

//    log_info(QString("已添加新截图，当前截图数量: %1").arg(windowInfoList->count()));
//}

void LeftSideBar::createScreenshotFolder()
{
    // 获取截图文件夹路径 (FFmpeg_Player_local同级目录下的screenshot)
    QString appDir = QCoreApplication::applicationDirPath();
    QString screenshotDir = appDir + "/../screenshot";  // 向上一级目录

    QDir dir(screenshotDir);
    if (!dir.exists()) {
        if (!dir.mkpath(screenshotDir)) {
            log_error("无法创建截图文件夹");
            QMessageBox::warning(nullptr, "警告", "无法创建截图文件夹，截图将不会保存到文件");
        } else {
            log_info("截图文件夹创建成功: " + screenshotDir);
        }
    }
}

bool LeftSideBar::saveScreenshot(const QPixmap &screenshot)
{
    // 获取截图文件夹路径
    QString appDir = QCoreApplication::applicationDirPath();
    QString screenshotDir = appDir + "/../screenshot";

    // 生成唯一文件名 (时间戳)
    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss");
    QString filename = screenshotDir + "/screenshot_" + timestamp + ".png";

    // 保存截图
    if (!screenshot.save(filename)) {
        log_error("保存截图失败: " + filename);
        return false;
    } else {
        log_info("截图已保存: " + filename);
    }

    return true;
}

// 改名为初始化播放列表标签页
void LeftSideBar::setupPlaylistTab()
{
    // 确保组件已初始化
    if (!playlist) {
        log_error("播放列表未初始化");
        return;
    }

    playlist->clear(); // 清空现有内容

    // 设置列表属性
    playlist->setIconSize(QSize(32, 32));
    playlist->setSelectionMode(QAbstractItemView::ExtendedSelection);
    playlist->setDragDropMode(QAbstractItemView::InternalMove);

    // 添加现有媒体文件
    const QStringList mediaList = m_playlist->all();
    for (const QString &path : mediaList) {
        addPlaylistItem(path);
    }

    // 连接信号
    connect(playlist, &QListWidget::itemDoubleClicked, [this](QListWidgetItem *item) {
        emit playRequested(item->data(Qt::UserRole).toString());
    });

    log_info(QString("播放列表初始化完成，共 %1 个媒体文件").arg(mediaList.size()));
}

// 专用添加播放列表项的函数
void LeftSideBar::addPlaylistItem(const QString &filePath)
{
    QListWidgetItem *item = new QListWidgetItem();
    QFileInfo fi(filePath);

    // 基础信息
    item->setText(fi.fileName());
    item->setData(Qt::UserRole, filePath);

    // 根据文件类型设置不同图标
    static QIcon videoIcon(":/icons/video.png");
    static QIcon audioIcon(":/icons/audio.png");
    item->setIcon(fi.suffix().startsWith("mp3") ? audioIcon : videoIcon);

    playlist->addItem(item);
}

void LeftSideBar::onPlaybackStarted()
{
    // 当播放开始时启动定时器
    if (m_mainWindow && !m_screenshotTimer->isActive()) {
        m_screenshotTimer->start(10000);  // 10秒
        log_info("播放已开始，截图定时器已启动");
    }
}

void LeftSideBar::onPlaybackFinished(bool success)
{
    if (success && m_playlist->count() > 0) {
        QString nextFile = m_playlist->next();
        if (!nextFile.isEmpty()) {
            emit playRequested(nextFile);
        }
    }
}

void LeftSideBar::onAddMediaRequested(const QString &filePath)
{
    m_playlist->addMedia(filePath);
}

void LeftSideBar::onRemoveMediaRequested(int index)
{
    m_playlist->removeMedia(index);
}

void LeftSideBar::clearPlaylist()
{
    m_playlist->clear();
    playlist->clear();
}
