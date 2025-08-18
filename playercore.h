#ifndef PLAYERCORE_H
#define PLAYERCORE_H

// 由于 FFmpeg 是 C 语言实现的，在 C++ 中使用需要用 extern "C" 包装
extern "C"
{
#include <libavutil/log.h>
#include <libavutil/avutil.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>  // 添加缩放转换头文件
#include <libavutil/imgutils.h>  // 添加图像工具头文件
}

#include <set>
#include <mutex>

#include <QObject>
#include <QString>
#include <QImage>
#include <QTimer>
#include <QThread>

class PlayerCore : public QObject
{
    Q_OBJECT
public:
    explicit PlayerCore(QObject *parent = nullptr);
    ~PlayerCore();

    // 播放控制功能
    void play();
    void pause();
    void stop();
    void seek(qint64 position);
    void setVolume(int volume);

    // 文件操作
    bool openFile(const QString &fileName);

    // 获取播放状态
    bool isPlaying() const { return m_isPlaying; }
    int64_t getDuration() const { return m_duration; }
    int getVideoStreamIndex() const { return m_videoStreamIndex; }
    int getAudioStreamIndex() const { return m_audioStreamIndex; }

signals:
    void playStateChanged(bool isPlaying);
    void stopStateChanged();
    void durationChanged(int64_t duration);
    void positionChanged(qint64 position);
    void errorOccurred(const QString &errorString);
    void frameDecoded(const QImage &image);         // 添加帧解码完成信号
    void stopDecoding();

private:
    void decodeLoop();                              // 添加解码循环函数
    void decodeNextFrame();
    void initializeDecoder();
    void releaseResources();

private:
    // 帧结构（按PTS排序）
    struct VideoFrame {
        qint64 pts;      // 显示时间戳（毫秒）
        QImage image;    // 解码后的图像
        bool operator<(const VideoFrame& other) const {
            return pts < other.pts;
        }
    };

    // 存储帧的数据结构
    std::set<VideoFrame>   m_frameSet;           // 有序去重帧集合
    std::mutex             m_setMutex;           // 集合锁
    const int              MAX_FRAME_COUNT = 15; // 缓存15帧（约0.6秒

    // 运行线程
    QThread         *m_decodeThread = nullptr;      // 独立解码线程
    std::mutex      m_formatMutex;                  // 成员变量

    // 播放器核心数据
    AVFormatContext *m_formatContext = nullptr;     //音视频封装格式上下文结构体
    AVCodecContext  *m_videoCodecContext = nullptr; //音视频编码器上下文结构体
    AVCodecContext  *m_audioCodecContext = nullptr;
    AVCodec         *m_codec = nullptr;             // 音视频编码器结构体(注意，老师代码中有，而我没有)
    AVFrame         *m_frame = nullptr;             // 存储解码后帧数据
    AVFrame         *m_rgbFrame = nullptr;          // 存储转换后RGB帧
    AVPacket        *m_packet = nullptr;            // 存储编码数据包
    SwsContext      *m_swsContext = nullptr;        // 图像转换上下文

    QTimer          *m_timer;
    uint8_t         *m_outBuffer = nullptr;         // RGB帧缓冲区

    // 播放控制变量
    bool            m_isPlaying = false;
    bool            m_isEOF = false;                // 标记是否解析完所有帧
    int             m_volume = 20;

    int             m_videoStreamIndex = -1;
    int             m_audioStreamIndex = -1;

    double          m_fps = 0;                      // 新增：视频帧率
    qint64          m_maxPts = -1;                  // 当前最大PTS（过滤旧帧用）
    int64_t         m_duration = 0;
    int             m_frameDuration = 0;            // 新增: 每帧持续时间（毫秒）

    qint64          m_lastValidPosition = -1;       // 记录上一次有效位置
};

#endif // PLAYERCORE_H
