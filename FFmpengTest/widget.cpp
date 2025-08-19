#include "widget.h"
#include <QVBoxLayout>

Widget::Widget(QWidget *parent)
    : QWidget{parent}
    , m_formatContext(nullptr)
    , m_videoCodec(nullptr)
    , m_videoCodecContext(nullptr)
    , m_videoStreamIndex(-1)
    , m_swsContext(nullptr)
    , m_decodedFrame(nullptr)
    , m_packet(nullptr)
{
    initUI();

    // 设置QWidget的背景色
    setAutoFillBackground(true);
    QPalette palette = this->palette();
    palette.setColor(QPalette::Window, Qt::black);
    setPalette(palette);
}

Widget::~Widget()
{
    m_readTimer.stop();
    cleanupFFmpeg();
}

void Widget::initUI()
{
    m_connectButton = new QPushButton("连接并播放", this);
    m_statusLabel = new QLabel("未连接", this);
    m_statusLabel->setAlignment(Qt::AlignCenter);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(m_statusLabel);
    layout->addWidget(m_connectButton);
    layout->addStretch(); // 视频区域

    // 按钮按下 ---> 发起连接
    connect(m_connectButton, &QPushButton::clicked, this, &Widget::onConnectButtonClicked);
    // 定时器超时 --->
    connect(&m_readTimer, &QTimer::timeout, this, &Widget::onReadFrame);
}

void Widget::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);

    if (!m_videoImage.isNull()) {
        // 计算目标绘制区域，保持宽高比
        int image_width = m_videoImage.width();
        int image_height = m_videoImage.height();
        int widget_width = this->width();
        int widget_height = this->height();

        if (image_width > 0 && image_height > 0) {
            QRect targetRect = this->rect();
            float scale_x = (float)widget_width / image_width;
            float scale_y = (float)widget_height / image_height;
            float scale = std::min(scale_x, scale_y);

            int draw_width = image_width * scale;
            int draw_height = image_height * scale;
            int x = (widget_width - draw_width) / 2;
            int y = (widget_height - draw_height) / 2;

            painter.drawImage(QRect(x, y, draw_width, draw_height), m_videoImage);
        }
    }
}

// 初始化FFmpeg
bool Widget::initFFmpeg()
{
    // 分配 FFmpeg 上下文给数据成员
    m_formatContext = avformat_alloc_context();
    if (!m_formatContext) {
        qDebug() << "错误: 无法分配 AVFormatContext";
        return false;
    }

    // 打开RTMP流地址
    const char* rtmpUrl = "rtmp://192.168.5.222:1935/hlsram/live0";
    int ret = avformat_open_input(&m_formatContext, rtmpUrl, nullptr, nullptr); // 发送了RTMP连接请求
    if (ret != 0) {
        char errbuf[AV_ERROR_MAX_STRING_SIZE] = {0};
        av_strerror(ret, errbuf, sizeof(errbuf));
        qDebug() << "错误: 无法打开输入流." << errbuf;
        cleanupFFmpeg();
        return false;
    }

    // 使用FFmpeg上下文, 调用函数获取这个流的元数据
    ret = avformat_find_stream_info(m_formatContext, nullptr);
    if (ret < 0) {
        qDebug() << "错误: 无法找到流信息.";
        cleanupFFmpeg();
        return false;
    }

    // 获取最佳视频流的上下文索引(int)
    m_videoStreamIndex = av_find_best_stream(m_formatContext, AVMEDIA_TYPE_VIDEO, -1, -1, &m_videoCodec, 0);
    if (m_videoStreamIndex < 0) {
        qDebug() << "错误: 无法找到视频流.";
        cleanupFFmpeg();
        return false;
    }

    // 获取解码器上下文
    m_videoCodecContext = avcodec_alloc_context3(m_videoCodec);
    if (!m_videoCodecContext) {
        qDebug() << "错误: 无法分配 AVCodecContext.";
        cleanupFFmpeg();
        return false;
    }

    ret = avcodec_parameters_to_context(m_videoCodecContext, m_formatContext->streams[m_videoStreamIndex]->codecpar);
    if (ret < 0) {
        qDebug() << "错误: 无法将参数复制到解码器上下文.";
        cleanupFFmpeg();
        return false;
    }

    // 解码
    ret = avcodec_open2(m_videoCodecContext, m_videoCodec, nullptr);
    if (ret < 0) {
        qDebug() << "错误: 无法打开解码器.";
        cleanupFFmpeg();
        return false;
    }

    // 分配AVFrame和AVPacket
    m_decodedFrame = av_frame_alloc();
    m_packet = av_packet_alloc();

    if (!m_decodedFrame || !m_packet) {
        qDebug() << "错误: 无法分配 AVFrame 或 AVPacket.";
        cleanupFFmpeg();
        return false;
    }

    return true;
}

// 释放FFmpeg资源逻辑
void Widget::cleanupFFmpeg()
{
    if (m_packet) {
          av_packet_free(&m_packet);
          m_packet = nullptr;
      }
      if (m_decodedFrame) {
          av_frame_free(&m_decodedFrame);
          m_decodedFrame = nullptr;
      }
      if (m_videoCodecContext) {
          avcodec_close(m_videoCodecContext);
          avcodec_free_context(&m_videoCodecContext);
          m_videoCodecContext = nullptr;
      }
      if (m_formatContext) {
          avformat_close_input(&m_formatContext);
          avformat_free_context(m_formatContext);
          m_formatContext = nullptr;
      }


      // 注释掉会导致内存泄漏。
      // if (m_swsContext) {
      //     sws_free_context(m_swsContext);
      //     m_swsContext = nullptr;
      // }

      m_swsContext = nullptr; // 直接将指针置空，不调用释放函数
}

void Widget::onConnectButtonClicked()
{
    m_connectButton->setEnabled(false);
    m_statusLabel->setText("正在连接...");

    cleanupFFmpeg();    // 清理剩余数据

    if (initFFmpeg()) {
        m_statusLabel->setText("连接成功，开始播放...");
        // 启动定时器，每30毫秒读取一帧, 视频最高33帧
        m_readTimer.start(30);
    } else {
        m_statusLabel->setText("连接失败");
        m_connectButton->setEnabled(true);
    }
}

void Widget::onReadFrame()
{
    if (av_read_frame(m_formatContext, m_packet) < 0) {         // 从流中读取一个数据包 AVPacket
        m_readTimer.stop();
        m_statusLabel->setText("流已结束或读取失败");
        qDebug() << "警告: 无法从流中读取帧.";
        cleanupFFmpeg();
        return;
    }

    // 之前已经获取到了一个最佳的视频流索引, 我们要确定获取到的AVPacket就是这个视频流, 从而忽略别的流(音频数据包也被忽略了)
    if (m_packet->stream_index == m_videoStreamIndex) {
        int ret = avcodec_send_packet(m_videoCodecContext, m_packet);   // 把AVPacket发给解码器解码
        if (ret < 0) {
            qDebug() << "警告: 无法发送数据包到解码器.";
            av_packet_unref(m_packet);
            return;
        }

        while (ret >= 0) {
            ret = avcodec_receive_frame(m_videoCodecContext, m_decodedFrame);   // 循环获取原始视频帧
            if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                break;
            } else if (ret < 0) {
                qDebug() << "错误: 解码帧失败.";
                break;
            }

            // 确保SWS上下文已初始化或更新 , 将AVFrame格式转为Qt使用的QImage格式
            m_swsContext = sws_getCachedContext(m_swsContext,
                                            m_decodedFrame->width, m_decodedFrame->height, m_videoCodecContext->pix_fmt,
                                            m_decodedFrame->width, m_decodedFrame->height, AV_PIX_FMT_RGB32,
                                            SWS_BILINEAR, nullptr, nullptr, nullptr);
            if (!m_swsContext) {
                qDebug() << "错误: 无法初始化SWS上下文.";
                return;
            }

            // 创建QImage并将AVFrame数据转换进去
            QImage tempImage(m_decodedFrame->width, m_decodedFrame->height, QImage::Format_RGB32);
            uint8_t *dst_data[4] = {tempImage.bits(), nullptr, nullptr, nullptr};
            int dst_linesize[4] = {static_cast<int>(tempImage.bytesPerLine()), 0, 0, 0};

            sws_scale(m_swsContext, m_decodedFrame->data, m_decodedFrame->linesize,
                      0, m_decodedFrame->height, dst_data, dst_linesize);

            // 将转换后的QImage赋值给成员变量
            m_videoImage = tempImage;
            update(); // 触发paintEvent进行绘制
        }
    }

    av_packet_unref(m_packet);
}
