#ifndef FFmpeg_Player_H
#define FFmpeg_Player_H

#include "titlebar.h"
#include "bottombar.h"
#include "playercore.h"
#include "playertools.h"
#include "leftsidebar.h"
#include "rightsidebar.h"
#include "centralwidget.h"
#include "cframelesswidget.h"

#include <QWidget>
#include <QListWidget>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QPushButton>

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

class FFmpeg_Player : public CFramelessWidget
{
    Q_OBJECT

public:
    FFmpeg_Player(QWidget *parent = nullptr);
    ~FFmpeg_Player();

    // 初始化
    void initFFmpeg();                // 初始化FFmpeg库
    void initUI();                    // 初始化UI界面

    // 基础UI布局
    void createTitleBar();            // 创建标题栏
    void createBottomBar();           // 创建底部栏

    // 播放控制功能
    void takeScreenshot();
    void switchLayout(int layoutType); // 0:单画面, 1:2个竖切, 2:2个横切

    // 登录状态管理
    bool isLoggedIn() const;          // 检查是否登录
    void setLoggedIn(bool loggedIn);  // 设置登录状态

signals:
    // 向titlebar转发错误信息的信号
    void systemMessageToRightBar(const QString &message);

private slots:
    // 界面按钮槽函数
//    void onPlayButtonClicked();
//    void onPauseButtonClicked();
//    void onStopButtonClicked();
    void onVolumeSliderValueChanged(int value);
    void onScreenshotButtonClicked();
    void onLayoutComboBoxCurrentIndexChanged(int index);
    void onProgressBarValueChanged(int value);

    // 菜单动作槽函数
//    void onMonitorActionTriggered();
//    void onPlaybackActionTriggered();
    void onLogActionTriggered();
    void onSystemActionTriggered();
    void onOpenFileButtonClicked();
    void onOpenUrlButtonClicked();
//    void openVideoFile(const char *fileName);

    // 标题栏信号槽
    void onMinimizeClicked();
    void onMaximizeClicked();
    void onCloseClicked();

    // PlayerCore信号槽
    void onPlayStateChanged(bool isPlaying);
    void onStopStateChanged();
    void onDurationChanged(int64_t duration);
    void onPositionChanged(qint64 position);
    void onErrorOccurred(const QString &errorString);

    // 错误处理信号槽
    void onSystemMessageToRightBar(const QString&);

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    void toggleFullscreen();

private:
    bool m_isLoggedIn;             // 新增：登录状态标志

    // UI组件
    TitleBar *titleBar;          // 标题栏
    BottomBar *bottomBar;        // 底部栏
    LeftSideBar *leftSideBar;    // 左边栏
    RightSideBar *rightSideBar;  // 右边栏
    CentralWidget *centralWidget; // 中央播放区域

    // 播放器核心和工具
    PlayerCore *playerCore;      // 播放核心
    PlayerTools *playerTools;    // 播放工具

};
#endif // WIDGET_H
