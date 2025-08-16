#include "playercore.h"
#include <QDebug>

PlayerCore::PlayerCore(QObject *parent) : QObject(parent)
{
    // 初始化FFmpeg
    av_register_all();
    avcodec_register_all();
    avformat_network_init();
    av_log_set_level(AV_LOG_INFO);
}

PlayerCore::~PlayerCore()
{
    releaseResources();
}

bool PlayerCore::openFile(const QString &fileName)
{
    // 释放之前的资源
    releaseResources();

    // 打开文件
    int ret = avformat_open_input(&m_formatContext, fileName.toUtf8().constData(), nullptr, nullptr);
    if (ret != 0) {
        char errbuf[1024] = {0};
        av_strerror(ret, errbuf, sizeof(errbuf));
        emit errorOccurred(QString("无法打开文件: %1").arg(errbuf));
        return false;
    }

    // 获取流信息
    ret = avformat_find_stream_info(m_formatContext, nullptr);
    if (ret < 0) {
        char errbuf[1024] = {0};
        av_strerror(ret, errbuf, sizeof(errbuf));
        emit errorOccurred(QString("无法获取流信息: %1").arg(errbuf));
        avformat_close_input(&m_formatContext);
        m_formatContext = nullptr;
        return false;
    }

    // 查找视频流和音频流
    m_videoStreamIndex = -1;
    m_audioStreamIndex = -1;
    for (int i = 0; i < m_formatContext->nb_streams; i++) {
        if (m_formatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            m_videoStreamIndex = i;
        } else if (m_formatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            m_audioStreamIndex = i;
        }
    }

    // 打印流信息
    qDebug() << "视频流索引: " << m_videoStreamIndex;
    qDebug() << "音频流索引: " << m_audioStreamIndex;

    // 设置总时长
    m_duration = m_formatContext->duration / AV_TIME_BASE;
    emit durationChanged(m_duration);

    return true;
}

void PlayerCore::play()
{
    if (!m_formatContext || m_isPlaying) {
        return;
    }

    // 初始化解码器
    initializeDecoder();

    m_isPlaying = true;
    emit playStateChanged(true);
    qDebug() << "开始播放";

    // 这里应该启动一个线程来进行视频解码和播放
    // 简化版本中，我们可以在这里添加解码循环
}

void PlayerCore::pause()
{
    if (!m_isPlaying) {
        return;
    }

    m_isPlaying = false;
    emit playStateChanged(false);
    qDebug() << "暂停播放";
}

void PlayerCore::stop()
{
    if (!m_formatContext) {
        return;
    }

    m_isPlaying = false;

    // 释放解码器资源
    if (m_videoCodecContext) {
        avcodec_free_context(&m_videoCodecContext);
        m_videoCodecContext = nullptr;
    }
    if (m_audioCodecContext) {
        avcodec_free_context(&m_audioCodecContext);
        m_audioCodecContext = nullptr;
    }

    emit stopStateChanged();
    qDebug() << "停止播放";
}

void PlayerCore::seek(qint64 position)
{
    if (!m_formatContext) {
        return;
    }

    int64_t seekPos = position * AV_TIME_BASE;
    av_seek_frame(m_formatContext, -1, seekPos, AVSEEK_FLAG_BACKWARD);
    emit positionChanged(position);
}

void PlayerCore::setVolume(int volume)
{
    m_volume = volume;
    qDebug() << "设置音量: " << volume;
}

void PlayerCore::initializeDecoder()
{
    // 初始化视频解码器
    if (m_videoStreamIndex >= 0) {
        AVCodecParameters *codecParams = m_formatContext->streams[m_videoStreamIndex]->codecpar;
        const AVCodec *codec = avcodec_find_decoder(codecParams->codec_id);
        if (!codec) {
            emit errorOccurred("无法找到视频解码器");
            return;
        }

        m_videoCodecContext = avcodec_alloc_context3(codec);
        if (!m_videoCodecContext) {
            emit errorOccurred("无法分配视频解码器上下文");
            return;
        }

        if (avcodec_parameters_to_context(m_videoCodecContext, codecParams) < 0) {
            emit errorOccurred("无法复制 codec 参数到上下文");
            return;
        }

        if (avcodec_open2(m_videoCodecContext, codec, nullptr) < 0) {
            emit errorOccurred("无法打开视频解码器");
            return;
        }
    }

    // 初始化音频解码器 (类似视频解码器初始化)
    // ...
}

void PlayerCore::releaseResources()
{
    // 释放解码器资源
    if (m_videoCodecContext) {
        avcodec_free_context(&m_videoCodecContext);
        m_videoCodecContext = nullptr;
    }
    if (m_audioCodecContext) {
        avcodec_free_context(&m_audioCodecContext);
        m_audioCodecContext = nullptr;
    }

    // 关闭文件
    if (m_formatContext) {
        avformat_close_input(&m_formatContext);
        m_formatContext = nullptr;
    }

    m_videoStreamIndex = -1;
    m_audioStreamIndex = -1;
    m_isPlaying = false;
    m_duration = 0;
}
