#ifndef BOTTOMBAR_H
#define BOTTOMBAR_H

#include "playercore.h"
#include "playertools.h"

#include <QWidget>
#include <QPushButton>
#include <QSlider>
#include <QLabel>
#include <QComboBox>

class BottomBar : public QWidget
{
    Q_OBJECT

public:
    explicit BottomBar(QWidget *parent = nullptr);
    ~BottomBar();

//    void setProgressRange(int min, int max);     // 设置进度条范围
//    void setProgressValue(int value);            // 设置进度条值
//    void setCurrentTime(const QString &time);    // 设置当前时间
//    void setDuration(const QString &duration);   // 设置总时长
    void setDuration(int64_t duration);
    void setPosition(qint64 position);
    void updatePlayState(bool isPlaying);
    void updateStopState();

    // 获得成员
    void setPlayerCore(PlayerCore *core) { m_playerCore = core; }
    void setPlayerTools(PlayerTools *tools) { m_playerTools = tools; }

signals:
    void playClicked();
    void pauseClicked();
    void stopClicked();
    void volumeValueChanged(int value);
    void screenshotButtonClicked();
    void layoutIndexChanged(int index);
    void progressValueChanged(int value);
//    void playRequested();
//    void pauseRequested();
//    void stopRequested();

//public slots:
//    void enablePlayButton(bool enable);    // 启用/禁用播放按钮
//    void enablePauseButton(bool enable);   // 启用/禁用暂停按钮
//    void enableStopButton(bool enable);    // 启用/禁用停止按钮

private slots:
    void onPlayButtonClicked();
    void onPauseButtonClicked();
    void onStopButtonClicked();

    void onVolumeSliderValueChanged(int value);
    void onScreenshotButtonClicked();
    void onLayoutComboBoxCurrentIndexChanged(int index);
    void onProgressBarValueChanged(int value);

private:
    // 按钮
    QPushButton *playButton;
    QPushButton *pauseButton;
    QPushButton *stopButton;

    // 时间进度条
    QLabel *currentTimeLabel;
    QLabel *durationLabel;
    QSlider *progressBar;
    QPushButton *volumeButton;
    QSlider *volumeSlider;

    // 截图
    QPushButton *screenshotButton;

    // 切换视角
    QComboBox *layoutComboBox;

    // 工具与核心类指针
    PlayerCore *m_playerCore = nullptr;
    PlayerTools *m_playerTools = nullptr;
};

#endif // BOTTOMBAR_H
