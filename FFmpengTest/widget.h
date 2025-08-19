#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QTimer>
#include <QImage>
#include <QDebug>
#include <QPainter>

// ffmpeg是用C语言实现的，在C++中使用，必须要加上 extern "C"{}
extern "C" {
#include <libavutil/avutil.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavdevice/avdevice.h>
#include <libavfilter/avfilter.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
#include <libpostproc/postprocess.h>
}


class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

protected:
    // 重写paintEvent以绘制视频帧
    void paintEvent(QPaintEvent* event) override;

private slots:
    void onConnectButtonClicked();  // 在按钮按下时被调用, 并调用initFFmpeg进行连接
    void onReadFrame();             // 使用av_read_frame() 从流中读取一个数据包（AVPacket）。

private:
    void initUI();
    bool initFFmpeg();
    void cleanupFFmpeg();

    QPushButton *m_connectButton;   // 按钮
    QLabel *m_statusLabel;          // 状态标签
    QImage m_videoImage;            // 用于存储视频帧的QImage, paintEvent 函数会从这个对象中获取图像数据，并将其绘制到窗口上

    // FFmpeg 相关成员
    AVFormatContext *m_formatContext;       // FFmpeg 的格式上下文, 用于打开 RTMP 连接
    AVCodec *m_videoCodec;                  // 访问具体的解码器（如H.264、H.265）
    AVCodecContext *m_videoCodecContext;    // 解码器上下文
    int m_videoStreamIndex;                 // 在FFmpeg格式的上下文索引
    SwsContext *m_swsContext;               //缩放和像素格式转换上下文
    AVFrame *m_decodedFrame;                // 解码后未压缩的原始帧
    AVPacket *m_packet;                     // 压缩数据包

    QTimer m_readTimer;         // 用于在主线程中定时读取帧, 实现非阻塞式的视频帧读取和播放
};
#endif // WIDGET_H
