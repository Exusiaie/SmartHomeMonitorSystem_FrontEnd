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

#include <mutex>
//#include <condition_variable>
#include <set>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
using std::condition_variable;
using std::lock_guard;
using std::thread;

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
    void close();
    void seek(qint64 position);
    void setVolume(int volume);

    // 文件操作
    bool openFile(const QString &fileName);
    bool openUrl(const QString &url = "rtsp://admin:admin@192.168.5.222/live/chn=0"); // 新增URL播放方法

    // 获取播放状态
    bool isPlaying() const { return m_isPlaying; }
    int64_t getDuration() const { return m_duration; }
    int getVideoStreamIndex() const { return m_videoStreamIndex; }
//    int getAudioStreamIndex() const { return m_audioStreamIndex; }

signals:
    void playStateChanged(bool isPlaying);
    void stopStateChanged();
    void playbackStarted();                         // 添加播放开始信号
    void playbackFinished(bool success);            // 修改后的播放结束信号
    void frameDecoded(const QImage &image);         // 帧解码完成信号
    void stopDecoding();
    void clearVideoDisplay();                       // 清除视频显示信号

    void durationChanged(int64_t duration);
    void positionChanged(qint64 position);

    void errorOccurred(const QString &errorString);

private:
    void decodeLoop();                              // 解码循环函数
    void decodeNextFrame();
    void initializeDecoder();
    void releaseResources();

private:
    // 帧结构（按PTS排序）
    struct VideoFrame {
        qint64 pts;      // 显示时间戳（秒）
        QImage image;    // 解码后的图像

        // 用于set排序
        bool operator<(const VideoFrame& other) const {
            return pts < other.pts;
        }
    };

    // 存储帧的数据结构
    std::set<VideoFrame>   m_frameSet;           // 有序帧集合
    std::mutex             m_setMutex;           // 集合锁
    std::condition_variable m_frameAvailable;    // 帧可用条件变量
    std::condition_variable m_queueNotFull;      // 队列未满条件变量
    const int              MAX_FRAME_COUNT = 50; // 缓存50帧

//    std::priority_queue<VideoFrame> m_frameQueue;  // 优先队列，自动按PTS升序排列
//    std::mutex               *m_frameQueueMutex;    // 队列互斥锁
//    std::condition_variable  *m_frameAvailable;     // 帧可用条件变量
//    std::condition_variable  *m_queueNotFull;       // 队列未满条件变量
//    int                       m_maxQueueSize;       // 队列最大大小

    // 运行线程
    QThread         *m_decodeThread = nullptr;      // 独立解码线程
    std::mutex      m_formatMutex;                  // 成员变量
//    std::thread     m_decodeThread;                 // 添加解码线程成员
    bool            m_threadRunning;                // 添加线程运行标志

    // 播放器核心数据
    AVFormatContext *m_formatContext = nullptr;     //音视频封装格式上下文结构体
    AVCodecContext  *m_videoCodecContext = nullptr; //音视频编码器上下文结构体
//    AVCodecContext  *m_audioCodecContext = nullptr;
//    AVCodec         *m_codec = nullptr;             // 音视频编码器结构体(注意，老师代码中有，而我没有)
    AVFrame         *m_frame = nullptr;             // 存储解码后帧数据
    AVFrame         *m_rgbFrame = nullptr;          // 存储转换后RGB帧
    AVPacket        *m_packet = nullptr;            // 存储编码数据包
    SwsContext      *m_swsContext = nullptr;        // 图像转换上下文

    QTimer          *m_timer;
    uint8_t         *m_outBuffer = nullptr;         // RGB帧缓冲区

    // 播放控制变量
    bool            m_isPlaying = false;
    bool            m_isEOF = false;                // 标记是否解析完所有帧
//    int             m_volume = 20;
    qint64          m_currentPosition = 0;

    int             m_videoStreamIndex = -1;
//    int             m_audioStreamIndex = -1;

    double          m_fps = 25;                     // 视频帧率，默认25
    double          m_frameDuration = 1.0 / 25;     // 每帧持续时间（秒）
    double          m_maxPts = -1;                  // 当前最大PTS（过滤旧帧用）
    double          m_duration = 0;                 // 总时长（秒）
    double          m_playStartTime = 0;            // 播放起始时间（秒）

//    qint64          m_currentPosition;
//    qint64          m_lastValidPosition = -1;       // 记录上一次有效位置
};

#endif // PLAYERCORE_H
