#include "bottombar.h"
#include <QHBoxLayout>
#include <QStyle>
#include <QDebug>

BottomBar::BottomBar(QWidget *parent)
    : QWidget(parent)
{
    setFixedHeight(60);
    setStyleSheet("background-color: #333; color: white;");

    // 创建底部布局
    QHBoxLayout *bottomLayout = new QHBoxLayout(this);
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
//    connect(playButton, &QPushButton::clicked, this, &BottomBar::playButtonClicked);
//    connect(pauseButton, &QPushButton::clicked, this, &BottomBar::pauseButtonClicked);
//    connect(stopButton, &QPushButton::clicked, this, &BottomBar::stopButtonClicked);
//    connect(progressBar, &QSlider::valueChanged, this, &BottomBar::progressValueChanged);
//    connect(volumeSlider, &QSlider::valueChanged, this, &BottomBar::volumeValueChanged);
//    connect(screenshotButton, &QPushButton::clicked, this, &BottomBar::screenshotButtonClicked);
//    connect(layoutComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &BottomBar::layoutIndexChanged);

    // - 播放功能类信号槽
    connect(playButton, &QPushButton::clicked, this, &BottomBar::onPlayButtonClicked);
    connect(pauseButton, &QPushButton::clicked, this, &BottomBar::onPauseButtonClicked);
    connect(stopButton, &QPushButton::clicked, this, &BottomBar::onStopButtonClicked);

    // - 菜单工具类信号槽
    connect(volumeSlider, &QSlider::valueChanged, this, &BottomBar::onVolumeSliderValueChanged);
    connect(screenshotButton, &QPushButton::clicked, this, &BottomBar::onScreenshotButtonClicked);
    connect(layoutComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &BottomBar::onLayoutComboBoxCurrentIndexChanged);
    connect(progressBar, &QSlider::valueChanged, this, &BottomBar::onProgressBarValueChanged);
}

BottomBar::~BottomBar()
{
}

void BottomBar::setDuration(int64_t duration)
{
    progressBar->setRange(0, duration);
    durationLabel->setText(QString("/ %1:%2").arg(duration / 60).arg(duration % 60, 2, 10, QLatin1Char('0')));
}

void BottomBar::setPosition(qint64 position)
{
    progressBar->setValue(position);
    currentTimeLabel->setText(QString("%1:%2").arg(position / 60).arg(position % 60, 2, 10, QLatin1Char('0')));
}

void BottomBar::updatePlayState(bool isPlaying)
{
    playButton->setEnabled(!isPlaying);
    pauseButton->setEnabled(isPlaying);
    stopButton->setEnabled(true);
}

void BottomBar::updateStopState()
{
    playButton->setEnabled(true);
    pauseButton->setEnabled(false);
    stopButton->setEnabled(false);
}

void BottomBar::onPlayButtonClicked()
{
    if (m_playerCore) {
        m_playerCore->play();
    }
    emit playClicked();
}

void BottomBar::onPauseButtonClicked()
{
    if (m_playerCore) {
        m_playerCore->pause();
    }
    emit pauseClicked();
}

void BottomBar::onStopButtonClicked()
{
    if (m_playerCore) {
        m_playerCore->stop();
    }
    emit stopClicked();
}

void BottomBar::onVolumeSliderValueChanged(int value)
{
    if (m_playerCore) {
        m_playerCore->setVolume(value);
    }
    emit volumeValueChanged(value);
}

void BottomBar::onScreenshotButtonClicked()
{
    emit screenshotButtonClicked();
}

void BottomBar::onLayoutComboBoxCurrentIndexChanged(int index)
{
    if (m_playerTools) {
        m_playerTools->switchLayout(index);
    }
    emit layoutIndexChanged(index);
}

void BottomBar::onProgressBarValueChanged(int value)
{
    if (m_playerCore) {
        m_playerCore->seek(value);
    }
    emit progressValueChanged(value);
}
