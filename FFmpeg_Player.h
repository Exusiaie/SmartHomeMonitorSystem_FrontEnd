#ifndef FFMPEG_H
#define FFMPEG_H

#include <QWidget>
#include <QPushButton>
#include <QSlider>
#include <QLabel>
#include <QListWidget>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFrame>
#include <QProgressBar>
#include <QMenuBar>
#include <QAction>
#include <QSystemTrayIcon>
#include <QToolButton>
#include <QComboBox>

// 由于 FFmpeg 是 C 语言实现的，在 C++ 中使用需要用 extern "C" 包装
extern "C"
{
#include <libavutil/log.h>
#include <libavutil/avutil.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavdevice/avdevice.h>
#include <libavfilter/avfilter.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
#include <libpostproc/postprocess.h>
}

class FFmpeg_Player : public QWidget
{
    Q_OBJECT

public:
    FFmpeg_Player(QWidget *parent = nullptr);
    ~FFmpeg_Player();

    // 初始化FFmpeg库
    void initFFmpeg();


    void initUI();                    // 初始化UI界面
    void createTitleBar();            // 创建标题栏
    void createLeftSideBar();         // 创建左边栏
    void createRightSideBar();        // 创建右边栏
    void createBottomBar();           // 创建底部栏
    void createCentralWidget();       // 创建播放区域

    // 播放控制功能
    void play();
    void pause();
    void stop();
    void seek(qint64 position);
    void setVolume(int volume);
    void takeScreenshot();
    void switchLayout(int layoutType); // 0:单画面, 1:2个竖切, 2:2个横切

private slots:
    // 界面按钮槽函数
    void onPlayButtonClicked();
    void onPauseButtonClicked();
    void onStopButtonClicked();
    void onVolumeSliderValueChanged(int value);
    void onScreenshotButtonClicked();
    void onLayoutComboBoxCurrentIndexChanged(int index);
    void onProgressBarValueChanged(int value);

    // 菜单动作槽函数
    void onMonitorActionTriggered();
    void onPlaybackActionTriggered();
    void onLogActionTriggered();
    void onSystemActionTriggered();
    void onOpenFileButtonClicked();
    void openVideoFile(const char *fileName);

private:

    // 播放器核心数据
    AVFormatContext *formatContext;    // FFmpeg格式上下文
    AVCodecContext *videoCodecContext; // 视频解码器上下文
    AVCodecContext *audioCodecContext; // 音频解码器上下文
    int videoStreamIndex;        // 视频流索引
    int audioStreamIndex;        // 音频流索引
    bool isPlaying;              // 播放状态

    // UI组件
    QWidget *titleBar;           // 标题栏
    QWidget *leftSideBar;        // 左边栏
    QWidget *rightSideBar;       // 右边栏
    QWidget *bottomBar;          // 底部栏
    QWidget *centralWidget;      // 中央播放区域

    // UI组件 - 标题栏
    QLabel *titleLabel;          // 标题标签
    QPushButton *monitorButton;  // 监控按钮
    QPushButton *playbackButton; // 回放按钮
    QPushButton *logButton;      // 日志按钮
    QPushButton *systemButton;   // 系统按钮
    QPushButton *minimizeButton; // 最小化按钮
    QPushButton *maximizeButton; // 最大化按钮
    QPushButton *closeButton;    // 关闭按钮

    // UI组件 - 侧边栏
    QTabWidget *leftTabWidget;   // 左侧选项卡
    QListWidget *windowInfoList; // 窗口信息列表
    QListWidget *playlist;       // 播放列表
    QTabWidget *rightTabWidget;  // 右侧选项卡
    QListWidget *deviceList;     // 设备列表
    QWidget *systemInfoWidget;   // 系统信息窗口

    // UI组件 - 中央播放区域
    QWidget *videoWidget;        // 视频播放窗口
    QPushButton *openFileButton; // 打开文件按钮

    // UI组件 - 底部栏
    QFrame *videoFrame1;         // 视频帧1
    QFrame *videoFrame2;         // 视频帧2

    QPushButton *playButton;     // 播放按钮
    QPushButton *pauseButton;    // 暂停按钮
    QPushButton *stopButton;     // 停止按钮

    QLabel *currentTimeLabel;    // 当前时间标签
    QLabel *durationLabel;       // 总时长标签
    QSlider *progressBar;        // 进度条

    QPushButton *volumeButton;     // 音量按钮
    QSlider *volumeSlider;         // 音量滑块
    QPushButton *warningButton;    // 警告按钮
    QPushButton *zoomButton;       // 放大按钮
    QPushButton *screenshotButton; // 截图按钮
    QComboBox *layoutComboBox;     // 布局组合框

};
#endif // WIDGET_H
