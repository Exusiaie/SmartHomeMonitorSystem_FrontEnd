#ifndef PLAYERCORE_H
#define PLAYERCORE_H

#include <QObject>
#include <QString>

// 由于 FFmpeg 是 C 语言实现的，在 C++ 中使用需要用 extern "C" 包装
extern "C"
{
#include <libavutil/log.h>
#include <libavutil/avutil.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}

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

private:
    // 播放器核心数据
    AVFormatContext *m_formatContext = nullptr;
    AVCodecContext *m_videoCodecContext = nullptr;
    AVCodecContext *m_audioCodecContext = nullptr;

    int m_videoStreamIndex = -1;
    int m_audioStreamIndex = -1;
    bool m_isPlaying = false;
    int64_t m_duration = 0;
    int m_volume = 80;

    void initializeDecoder();
    void releaseResources();
};

#endif // PLAYERCORE_H
