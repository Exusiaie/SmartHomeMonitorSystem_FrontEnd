#include "playercore.h"

#include <QDebug>
#include <QThread>
#include <QImage>
#include <QDateTime>

#define log_info(msg) \
    do { \
        qDebug("%s | %d | %s : %s", __FILE__, __LINE__, __FUNCTION__, msg); \
    } while(0)

#define log_error(msg) \
    do { \
        qCritical("%s | %d | %s : [ERROR] %s", __FILE__, __LINE__, __FUNCTION__, msg); \
    } while(0)

#define log_warn(msg) \
    do { \
        qWarning("%s | %d | %s : [WARN] %s", __FILE__, __LINE__, __FUNCTION__, msg); \
    } while(0)

PlayerCore::PlayerCore(QObject *parent)
    : QObject(parent)
    , m_formatContext(nullptr)
    , m_videoCodecContext(nullptr)
    , m_swsContext(nullptr)
    , m_frame(nullptr)
    , m_rgbFrame(nullptr)
    , m_outBuffer(nullptr)
    , m_packet(nullptr)
    , m_videoStreamIndex(-1)
    , m_duration(0)
    , m_isPlaying(false)
    , m_isEOF(false)
    , m_currentPosition(0)
    , m_fps(0.0)
    , m_frameDuration(0.0)
    , m_maxPts(-1)
    , m_threadRunning(false)  // 初始化线程运行标志
{
    // 初始化FFmpeg
    av_register_all();
    avcodec_register_all();
    avformat_network_init();        // 初始化网络组件
    av_log_set_level(AV_LOG_INFO);

    // 初始化成员变量
    m_frame = av_frame_alloc();
    m_rgbFrame = av_frame_alloc();
    m_packet = av_packet_alloc();
    m_timer = new QTimer(this);

    connect(m_timer, &QTimer::timeout, this, &PlayerCore::decodeNextFrame);
    m_timer->setInterval(1); // 设置定时器为1ms触发一次

//    // 添加条件变量和互斥锁
//    m_formatMutex = new std::mutex();
//    m_frameAvailable = new std::condition_variable();
//    m_queueNotFull = new std::condition_variable();
//    m_setSize = 50; // 设置队列最大大小

    log_info("初始化完成");
}

PlayerCore::~PlayerCore()
{
    log_info("PlayerCore析构函数开始执行");
    close();

//    if (m_decodeThread)
//    {
//        m_decodeThread->quit();
//        m_decodeThread->wait();
//        delete m_decodeThread;
//    }

    if (m_threadRunning && m_decodeThread->isRunning()) {
        m_decodeThread->wait();
        log_info("解码线程已结束");
    }

    // 释放帧和数据包
    if (m_frame)
        av_frame_free(&m_frame);
    if (m_rgbFrame)
        av_frame_free(&m_rgbFrame);
    if (m_packet)
        av_packet_free(&m_packet);

//    // 释放条件变量和互斥锁
//    delete m_formatMutex;
//    delete m_frameAvailable;
//    delete m_queueNotFull;

    log_info("退出~PlayerCore()");
}

bool PlayerCore::openFile(const QString &fileName)
{
    // 释放之前的资源
    releaseResources();
    log_info(QString("准备打开的文件路径: %1").arg(fileName).toUtf8().constData());

    // 打开文件
    int ret = avformat_open_input(&m_formatContext, fileName.toUtf8().constData(), nullptr, nullptr);
    if (ret != 0) {
        char errbuf[1024] = {0};
        av_strerror(ret, errbuf, sizeof(errbuf));
        emit errorOccurred(QString("无法打开文件: %1").arg(errbuf));
        return false;
    }
    log_info("打开文件完成");

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
    log_info("查找流信息完成");

    // 查找视频流和音频流
    // - 相比test文件，此处没有初始化解码器流程，解码放在了openFile中
    m_videoStreamIndex = -1;
//    m_audioStreamIndex = -1;
    for (int i = 0; i < m_formatContext->nb_streams; i++) {
        if (m_formatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            m_videoStreamIndex = i;
        }
//        else if (m_formatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
//            m_audioStreamIndex = i;
//        }
    }

    // 打印流信息
    log_info(QString("视频流索引: %1").arg(m_videoStreamIndex).toUtf8().data());
//    log_info(QString("音频流索引: %1").arg(m_audioStreamIndex).toUtf8().data());

    // 设置总时长
//    m_duration = m_formatContext->duration/ (double)AV_TIME_BASE;             // 秒为单位
    m_duration = m_formatContext->duration * 1000/ AV_TIME_BASE;        // 毫秒为单位
    emit durationChanged(m_duration);
    log_info(QString("视频总时长: %1毫秒").arg(m_duration).toUtf8().data()); // 视频总时长: 243158，换算为分钟是4分钟

    // 视频流时间基
    // - 时间基 1/12800 确认计算正确（512 × 1000 / 12800 = 40ms）
    AVRational tb = m_formatContext->streams[m_videoStreamIndex]->time_base;
    log_info(QString("视频流时间基: %1 / %2").arg(tb.num).arg(tb.den).toUtf8().data());

    log_info("openFile执行完成，准备播放");
    play();

    emit playStateChanged(true);

    return true;
}

// 新增openUrl方法实现
bool PlayerCore::openUrl(const QString &url)
{
    // 释放之前的资源
    releaseResources();
    log_info(QString("准备打开的URL: %1").arg(url).toUtf8().constData());

    // 打开URL（与打开文件类似，FFmpeg会自动处理网络流）
    int ret = avformat_open_input(&m_formatContext, url.toUtf8().constData(), nullptr, nullptr);
    if (ret != 0) {
        char errbuf[1024] = {0};
        av_strerror(ret, errbuf, sizeof(errbuf));
        emit errorOccurred(QString("无法打开URL: %1").arg(errbuf));
        return false;
    }
    log_info("打开URL完成");

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
    log_info("查找流信息完成");

    // 查找视频流和音频流
    m_videoStreamIndex = -1;
//    m_audioStreamIndex = -1;
    for (int i = 0; i < m_formatContext->nb_streams; i++) {
        if (m_formatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            m_videoStreamIndex = i;
        }
//        else if (m_formatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
//            m_audioStreamIndex = i;
//        }
    }

    // 打印流信息
    log_info(QString("视频流索引: %1").arg(m_videoStreamIndex).toUtf8().data());
//    log_info(QString("音频流索引: %1").arg(m_audioStreamIndex).toUtf8().data());

    // 设置总时长（毫秒）
    m_duration = m_formatContext->duration * 1000 / AV_TIME_BASE;  // 毫秒为单位
//    m_duration = m_formatContext->duration / (double)AV_TIME_BASE;
    emit durationChanged(m_duration);
    log_info(QString("视频总时长: %1毫秒").arg(m_duration).toUtf8().data());

    // 视频流时间基
    if (m_videoStreamIndex >= 0) {
        AVRational tb = m_formatContext->streams[m_videoStreamIndex]->time_base;
        log_info(QString("视频流时间基: %1 / %2").arg(tb.num).arg(tb.den).toUtf8().data());
    }

    log_info("openUrl执行完成，准备播放");
    play();

    emit playStateChanged(true);

    return true;
}

void PlayerCore::play()
{
    log_info("play开始");

//    if (!m_formatContext || m_isPlaying)
//    {
//        return;
//    }

    if (!m_formatContext || m_isPlaying)
    {
        log_info("无法播放：缺少媒体上下文或已经在播放");
        return;
    }
    log_info("执行到判断播放状态");

    // 重置播放状态
    m_isPlaying = true;
    m_maxPts = -1;
    m_isEOF = false;
    m_playStartTime = QDateTime::currentMSecsSinceEpoch();  // 记录播放起始时间
    log_info("执行到重置播放状态");

    // 显示定时器（主线程）
    m_timer->setInterval(m_frameDuration);  // 用每帧持续时间作为间隔
    m_timer->start();
    log_info("执行到显示定时器");

    // 确保之前的线程已结束
    if (m_threadRunning && m_decodeThread && m_decodeThread->isRunning()) {
        m_decodeThread->quit();
        m_decodeThread->wait();
        delete m_decodeThread;
        m_decodeThread = nullptr;
    }
    m_threadRunning = false;
    log_info("执行到m_threadRunning");

    // 创建并启动新的解码线程
    m_decodeThread = new QThread(this);     // 指定parent
    connect(m_decodeThread, &QThread::started, [this]()
    {
        initializeDecoder();  // 需要在解码线程初始化
        decodeLoop();
    });
    connect(m_decodeThread, &QThread::finished, [this]()
    {
        m_threadRunning = false;
        log_info("解码线程已结束");
    });
    log_info("执行到m_decodeThread");

    m_threadRunning = true;
    m_decodeThread->start();

    emit playStateChanged(true);
    emit playbackStarted();

    log_info("play完成，开始播放");
}

// 简单哈希计算（基于数据包数据的累加校验和）
static uint32_t calculatePacketHash(const AVPacket* packet) {
    if (!packet || !packet->data || packet->size <= 0) {
        return 0;
    }
    uint32_t hash = 0;
    // 对数据包数据进行简单累加计算哈希（可根据需要替换为MD5等算法）
    for (int i = 0; i < packet->size; i++) {
        hash = (hash << 5) - hash + packet->data[i]; // 模拟ELF哈希简化版
    }
    return hash;
}

// 子线程解码循环（decodeLoop 函数） - 线程版本
void PlayerCore::decodeLoop()
{
    static int packetCount = 0;  // 新增：统计数据包序号

    // 循环解码，直到播放停止（m_isPlaying 为 false）
    while (m_isPlaying)
    {
        AVPacket packet;     // 存储编码的数据包（可能是视频或音频）
        // 从文件中读取一个数据包（音视频混合的包，需要按流索引区分）

        // 加锁保护read操作，与seek互斥
        int ret;
        {
            std::lock_guard<std::mutex> m_lock(m_formatMutex);
            ret = av_read_frame(m_formatContext, &packet);
        }

        if (ret < 0)
        {
            log_info(QString("av_read_frame 失败：%1，标记EOF").arg(ret).toUtf8().data());
            m_isEOF = true;
            break;
        }

        packetCount++;  // 递增序号

        // 计算数据包哈希
        uint32_t packetHash = calculatePacketHash(&packet);

//        // 增强日志：增加包大小、哈希值、流类型（音频/视频）
//        QString streamType = (packet.stream_index == m_videoStreamIndex) ? "视频流" : "音频流";
//        log_info(QString("读取数据包：序号=%1，流索引=%2（%3），PTS=%4，包大小=%5字节，哈希值=0x%6")
//                 .arg(packetCount)
//                 .arg(packet.stream_index)
//                 .arg(streamType)
//                 .arg(packet.pts)
//                 .arg(packet.size)
//                 .arg(packetHash, 8, 16, QChar('0')) // 哈希以8位十六进制显示
//                 .toUtf8().data());

        // 仅处理视频流的数据包（音频流当前未处理，可忽略）
        if (packet.stream_index == m_videoStreamIndex)
        {
            // 1. 将视频数据包发送到解码器
            int ret = avcodec_send_packet(m_videoCodecContext, &packet);
            if (ret < 0 && ret != AVERROR(EAGAIN))  // EAGAIN 表示解码器需要更多数据，是正常情况
            {
                log_error("发送数据包到解码器失败");
                av_packet_unref(m_packet);  // 释放数据包资源
                continue;
            }

            // 2. 从解码器接收解码后的原始帧（可能一次 send 对应多次 receive）
            while (ret >= 0)
            {
                ret = avcodec_receive_frame(m_videoCodecContext, m_frame);
                if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
                {
                    break;  // 需要更多数据或解码结束
                }
                else if (ret < 0)
                {
                    log_error("接收解码帧失败");
                    break;
                }

                // 3. 将 YUV 格式的帧转换为 RGB32 格式（供 Qt 显示）
                sws_scale(
                            m_swsContext,
                            (const uint8_t *const *)m_frame->data,  // 源数据（YUV）
                            m_frame->linesize,                      // 源数据每行字节数
                            0, m_videoCodecContext->height,         // 转换区域（从第0行到高度）
                            m_rgbFrame->data,                       // 目标数据（RGB）
                            m_rgbFrame->linesize                    // 目标数据每行字节数
                            );

                // 4. 创建 QImage（使用转换后的 RGB 数据）
                QImage image(m_outBuffer, m_videoCodecContext->width,
                             m_videoCodecContext->height, QImage::Format_RGB32);
                if(image.isNull())
                {
                    log_warn("创建QImage失败：无效的图像数据");
                    av_frame_unref(m_frame);
                    continue;
                }

                // 5. 计算帧的显示时间戳（PTS，转换为毫秒）
                qint64 pts;
                AVRational stream_tb = m_formatContext->streams[m_videoStreamIndex]->time_base;
                if (m_frame->pts == AV_NOPTS_VALUE) {
                  log_info("帧无PTS，使用DTS替代");
                  pts = av_rescale_q(m_frame->pkt_dts, stream_tb, {1, 1000});  // 目标时间基为毫秒
                } else {
                  pts = av_rescale_q(m_frame->pts, stream_tb, {1, 1000});  // 转为毫秒
                }
//                log_info(QString("原始PTS=%1，转换后PTS=%2毫秒").arg(m_frame->pts).arg(pts).toUtf8().data());

                // 6. 将帧添加到队列，使用条件变量控制队列大小
                {
                    std::unique_lock<std::mutex> lock(m_setMutex);
                    // 等待队列有空间
                    m_queueNotFull.wait(lock, [this]{ return m_frameSet.size() < MAX_FRAME_COUNT || !m_isPlaying; });
                    if (!m_isPlaying) break;

                    m_frameSet.insert({pts, std::move(image)});
//                    log_info(QString("插入帧：PTS=%1，队列大小=%2").arg(pts).arg(m_frameSet.size()).toUtf8().data());
                }

                m_frameAvailable.notify_one();     // 通知消费者有新帧可用

                av_frame_unref(m_frame);            // 释放当前帧资源（必须调用，否则内存泄漏）
            }
        }
        av_packet_unref(&packet);       // 释放数据包资源（必须调用）
    }

    // 播放结束
    if (m_isPlaying)
    {
        stop();                         // 停止播放
    }
    m_threadRunning = false;

    if (m_isPlaying && m_isEOF) {
        emit playbackFinished(true);    // 向列表发送信息，播放自动停止，因此为true
    }

    log_info("解码线程结束运行");
}

// 主线程帧显示（decodeNextFrame）
void PlayerCore::decodeNextFrame() {
    if (!m_isPlaying) return;

    // 计算当前实际播放时长
    double currentPlayTime = QDateTime::currentMSecsSinceEpoch() - m_playStartTime;

    // 等待有帧可用或播放停止
    std::unique_lock<std::mutex> lock(m_setMutex);
    bool hasFrame = m_frameAvailable.wait_for(lock, std::chrono::milliseconds(10),
        [this]{ return !m_frameSet.empty() || !m_isPlaying || m_isEOF; });

    if (!hasFrame || !m_isPlaying) return;

    // 如果队列为空但已到EOF，结束播放
    if (m_frameSet.empty() && m_isEOF) {
        stop();
        return;
    }

    // 找到当前应该显示的帧
    static double lastDisplayTime = 0;
    static int consecutiveSlowFrames = 0; // 连续慢帧计数
    static int consecutiveFastFrames = 0; // 连续快帧计数
    auto it = m_frameSet.begin();

    // 核心逻辑：确保按帧率间隔显示帧
    if (lastDisplayTime == 0 || currentPlayTime - lastDisplayTime >= m_frameDuration) {
        // 找到最后一个PTS <= currentPlayTime的帧
        while (it != m_frameSet.end() && it->pts <= currentPlayTime) {
            ++it;
        }

        // 计算队列中滞后帧的数量
        int laggingFrames = std::distance(m_frameSet.begin(), it);

        // 如果滞后帧过多（显示速度快），删除所有滞后帧
        if (laggingFrames > 5) { // 阈值可根据实际情况调整
            consecutiveFastFrames++;
            consecutiveSlowFrames = 0;

            if (consecutiveFastFrames > 3) { // 连续多次检测到快帧
                log_info(QString("显示速度过快，删除所有滞后帧：共%1帧")
                         .arg(laggingFrames).toUtf8().data());
                if (it != m_frameSet.begin()) {
                    --it; // 回退到最后一个PTS <= currentPlayTime的帧
                    VideoFrame frame = *it;
                    m_frameSet.erase(m_frameSet.begin(), it); // 删除所有滞后帧

                    lock.unlock();

                    // 显示最新的滞后帧
                    emit frameDecoded(frame.image);
                    emit positionChanged(static_cast<qint64>(frame.pts));
                    log_info(QString("显示帧：PTS=%1 毫秒")
                             .arg(frame.pts).toUtf8().data());

                    // 更新最后显示时间
                    lastDisplayTime = currentPlayTime;
                    return;
                }
            }
        } else if (laggingFrames == 0) { // 无滞后帧（显示速度慢）
            consecutiveSlowFrames++;
            consecutiveFastFrames = 0;

            if (consecutiveSlowFrames > 3) { // 连续多次检测到慢帧
                log_info(QString("显示速度过慢，给予解码线程更多时间")
                         .toUtf8().data());
                lock.unlock();
                // 增加休眠时间，给解码线程更多时间生成帧
                std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(m_frameDuration * 0.5)));
                m_queueNotFull.notify_one(); // 通知生产者队列有空间
                return;
            }
        } else {
            // 正常情况，重置计数器
            consecutiveSlowFrames = 0;
            consecutiveFastFrames = 0;
        }

        // 正常显示逻辑
        if (it != m_frameSet.begin()) {
            --it; // 回退到最后一个PTS <= currentPlayTime的帧
            VideoFrame frame = *it;
            m_frameSet.erase(it);

            lock.unlock();

            // 显示帧
            emit frameDecoded(frame.image);
            emit positionChanged(static_cast<qint64>(frame.pts)); // 以毫秒为单位发送进度
//            log_info(QString("显示帧：PTS=%1 毫秒").arg(frame.pts).toUtf8().data());

            // 更新最后显示时间
            lastDisplayTime = currentPlayTime;
        } else if (!m_frameSet.empty()) {
            // 没有找到合适的帧，等待下一帧
            log_info(QString("无合适帧：当前播放时间=%.3f毫秒，下一帧PTS=%.3f毫秒").arg(currentPlayTime).arg(m_frameSet.begin()->pts).toUtf8().data());
        }
    }

    // 通知生产者队列有空间
    m_queueNotFull.notify_one();
}

//// 主线程帧显示 - 此处为与初值的差值计算
//// 功能：从解码缓存队列中选取合适的帧，按照时间节奏显示，保证播放流畅性
//void PlayerCore::decodeNextFrame() {
//    if (!m_isPlaying || m_frameSet.empty()) return;                 // 退出条件：如果不在播放状态，或帧缓存队列为空，直接返回

//    // 计算从播放开始到当前的毫秒数（作为参考时间，用于判断当前应该显示哪一帧）
//    static qint64 startTime = -1;                                   // 静态变量，记录播放开始的系统时间（毫秒）
//    if (startTime == -1) {
//        startTime = QDateTime::currentMSecsSinceEpoch();            // 第一次进入函数时初始化，记录当前系统时间
//    }

//    qint64 currentSysTime = QDateTime::currentMSecsSinceEpoch();    // 获取当前系统时间（毫秒，从1970-01-01 00:00:00至今）
//    qint64 elapsedTime = currentSysTime - startTime;                // 计算从播放开始到现在已经过去的毫秒数（播放时长）

//    // 尝试对帧缓存队列加锁（非阻塞模式）
//    std::unique_lock<std::mutex> lock(m_setMutex, std::try_to_lock);    // 目的：避免主线程（显示帧）等待子线程（解码帧），提高响应性
//    if (!lock.owns_lock() || m_frameSet.empty()) return;    // 加锁失败（子线程正在操作队列）或队列已空，直接返回


//    static qint64 lastPTS = -1;          // 静态变量：记录上一帧的PTS（初始为-1，表示还未播放过任何帧）
//    static qint64 lastFrameTime = -1;    // 静态变量：记录上一帧显示的系统时间


////    const int FRAME_INTERVAL = m_frameDuration * 1000;    // 不要这么搞，帧率太高会让视频播的非常快
//    const int FRAME_INTERVAL = m_frameDuration;             // 每帧的理论持续时间（毫秒），由帧率计算而来（例如25fps对应40ms）
//    log_info(QString("目前帧率间隔：%1").arg(FRAME_INTERVAL).toUtf8().data());

//    const int TOLERANCE = 100;          // 时间误差容忍范围（毫秒）：允许实际显示时间与理论时间有±100ms的偏差

//    // 步骤1：删除所有过期帧（只保留上一帧前1秒内的帧，减少内存占用）
//    if (lastPTS != -1) {  // 只有播放过帧后才需要清理过期帧

//        VideoFrame expiredFrame{lastPTS - 1000, {}};        // 定义一个"过期阈值帧"：PTS为上一帧PTS减1000ms（即1秒前）
//        auto firstValid = m_frameSet.lower_bound(expiredFrame);
//        if (firstValid != m_frameSet.begin()) {             // 删除从队列开头到第一个有效帧之间的所有过期帧
//            int erased = std::distance(m_frameSet.begin(), firstValid);            // 计算要删除的过期帧数量
//            m_frameSet.erase(m_frameSet.begin(), firstValid);
//            log_info(QString("删除过期帧：共%1帧，剩余%2帧").arg(erased).arg(m_frameSet.size()).toUtf8().data());
//        }
//    }

//    // 步骤2：确定当前应显示的帧
//    // 情况1：播放第一帧（上一帧PTS为-1，还未播放过任何帧）
//    if (lastPTS == -1) {
//        auto it = m_frameSet.begin();        // 取队列中PTS最小的帧（第一个元素，因为m_frameSet是有序集合）
//        VideoFrame frame = *it;
//        m_frameSet.erase(it);                // 从队列中移除该帧（已选中要显示）
//        lock.unlock();                       // 解锁，允许子线程继续解码写入帧

//        // 日志记录第一帧显示信息
//        log_info(QString("显示帧：PTS=%1（上一帧PTS=%2）")
//                 .arg(frame.pts).arg(lastPTS).toUtf8().data());

//        lastPTS = frame.pts;        // 更新上一帧PTS为当前帧PTS
//        emit frameDecoded(frame.image);        // 发送信号：通知UI显示当前帧图像
//        emit positionChanged(frame.pts);       // 发送信号：通知UI更新进度条位置

//        // 校准startTime：确保后续时间计算与帧PTS同步
//        // - startTime = 当前系统时间 - 当前帧PTS（让elapsedTime与帧PTS对齐）
//        startTime = currentSysTime - frame.pts;
//        return;
//    }
//    // 情况2：播放非第一帧（已播放过至少一帧）
//    else
//    {
//        qint64 timeSinceLastFrame = currentSysTime - lastFrameTime;        // 检查是否已经过了足够的时间显示下一帧
//        if (timeSinceLastFrame < FRAME_INTERVAL - TOLERANCE) {             // 还未到显示下一帧的时间
//            log_info(QString("未到显示时间：还需等待%1ms")
//                     .arg(FRAME_INTERVAL - timeSinceLastFrame).toUtf8().data());
//            std::this_thread::sleep_for(std::chrono::milliseconds(FRAME_INTERVAL - timeSinceLastFrame));            // 休眠剩余时间
//            return;
//        }

//        qint64 targetPTS = elapsedTime;        // 计算当前"理论上应该显示的帧的PTS"（即播放到现在的总毫秒数）

//        VideoFrame targetFrame{targetPTS, {}};                // 在帧队列中查找最接近targetPTS的帧
//        auto it = m_frameSet.lower_bound(targetFrame);        // lower_bound：找到第一个PTS >= targetPTS的帧（有序集合的查找特性）

//        // 如果找到了符合条件的帧（存在PTS >= targetPTS的帧）
//        if (it != m_frameSet.end()) {
//            VideoFrame frame = *it;

//            // 检查1：当前帧是否在可接受的误差范围内（帧PTS - 目标PTS <= 100ms）
//            if (frame.pts - targetPTS <= TOLERANCE) {
//                m_frameSet.erase(it);                // 符合条件，移除该帧并显示
//                lock.unlock();
//                lastPTS = frame.pts;                 // 更新上一帧PTS

//                emit frameDecoded(frame.image);      // 通知UI显示帧和更新进度
//                emit positionChanged(frame.pts);

//                log_info(QString("显示帧：PTS=%1，目标PTS=%2，系统时间差=%3ms")
//                         .arg(frame.pts).arg(targetPTS).arg(frame.pts - targetPTS).toUtf8().data());
//                return;
//            }
//            // 检查2：帧PTS远大于目标PTS（超过1秒），可能是解码滞后或时间不同步
//            else if (frame.pts - targetPTS > 1000) {

//                log_info(QString("帧PTS远大于目标PTS，强制同步：帧PTS=%1，目标PTS=%2")       // 强制同步：直接显示该帧，并重新校准时间
//                         .arg(frame.pts).arg(targetPTS).toUtf8().data());
//                m_frameSet.erase(it);
//                lock.unlock();

//                lastPTS = frame.pts;
//                emit frameDecoded(frame.image);
//                emit positionChanged(frame.pts);

//                startTime = currentSysTime - frame.pts;                                 // 重新校准startTime，让后续时间计算与当前帧对齐
//                return;
//            }
//            // 检查3：帧还未到显示时间（误差在100ms到1秒之间）
//            else {
//                log_info(QString("未到显示时间：当前PTS=%1，目标PTS=%2，差=%3ms")           // 等待：记录日志并短暂休眠，让出CPU给解码线程
//                         .arg(lastPTS).arg(targetPTS).arg(targetPTS - lastPTS).toUtf8().data());

//                std::this_thread::sleep_for(std::chrono::milliseconds(1));              // 休眠1ms，减少CPU占用，同时让解码线程有机会生成更多帧
//            }
//        }
//        // 没有找到符合条件的帧（所有帧的PTS都小于targetPTS，即解码滞后）
//        else {
//            // 子情况1：帧队列已空（完全没有可显示的帧）
//            if (m_frameSet.empty()) {
//                log_info(QString("无任何帧：上一帧PTS=%1，目标PTS=%2（需等待解码）")
//                         .arg(lastPTS).arg(targetPTS).toUtf8().data());
//            }
//            // 子情况2：帧队列有帧，但所有帧的PTS都小于targetPTS（滞后）
//            else {
//                auto firstFrame = m_frameSet.begin();                // 取队列中最早的帧（PTS最小的帧）

//                if (firstFrame->pts > targetPTS + 1000) {            // 检查是否需要强制同步（最早的帧也比目标PTS大1秒以上）
//                    log_info(QString("所有帧PTS均大于目标PTS 1秒以上，强制同步：帧PTS=%1，目标PTS=%2")
//                             .arg(firstFrame->pts).arg(targetPTS).toUtf8().data());
//                    VideoFrame frame = *firstFrame;
//                    m_frameSet.erase(firstFrame);
//                    lock.unlock();

//                    lastPTS = frame.pts;
//                    emit frameDecoded(frame.image);
//                    emit positionChanged(frame.pts);

//                    startTime = currentSysTime - frame.pts;         // 重新校准时间
//                    return;
//                }
//                // 不需要强制同步，记录日志等待解码
//                log_info(QString("无合适帧：上一帧PTS=%1，下一帧PTS=%2，目标PTS=%3（需等待解码）")
//                         .arg(lastPTS).arg(firstFrame->pts).arg(targetPTS).toUtf8().data());
//            }

//            std::this_thread::sleep_for(std::chrono::milliseconds(1));  // 休眠1ms，让出CPU给解码线程生成更多帧
//        }
//    }
//}

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
    log_info("stop开始");

    m_isPlaying = false;
    m_timer->stop();

    // 通知所有等待的线程
    m_frameAvailable.notify_one();
    m_queueNotFull.notify_one();

    emit playStateChanged(false);
    emit playbackFinished(false);   // 播放被手动停止，因此发送false
    emit clearVideoDisplay();       // 发送清除视频显示信号

    log_info("stop结束");
}

void PlayerCore::close()
{
    log_info("进入close()");
    stop();
    releaseResources();

    if (m_formatContext) {
        avformat_close_input(&m_formatContext);
        m_formatContext = nullptr;
    }

    m_videoStreamIndex = -1;
//    m_audioStreamIndex = -1;
    m_duration = 0;
    m_maxPts = -1;

    // 清空帧缓存
    {
        std::lock_guard<std::mutex> lock(m_setMutex);
        m_frameSet.clear();
    }

    log_info("退出close(), 播放器资源已完全释放");
}

void PlayerCore::seek(qint64 position)
{
    if (!m_formatContext)
    {
        return;
    }

    // 将输入的毫秒位置转换为FFmpeg的时间基单位（微秒）
//    int64_t seekPos = position * AV_TIME_BASE;  // 毫秒为单位
//    int64_t seekPos = position * AV_TIME_BASE / 1000;  // 微秒为单位
    int64_t seekPos = position * AV_TIME_BASE;

    // 加锁确保与av_read_frame（读取数据包）操作互斥，避免冲突
    std::lock_guard<std::mutex> m_lock(m_formatMutex);

    // 调用FFmpeg的av_seek_frame实现跳转：移动文件指针到目标位置
    // 参数说明：
    // - m_formatContext：音视频格式上下文
    // - -1：表示对所有流（视频/音频）生效
    // - seekPos：目标位置（微秒）
    // - AVSEEK_FLAG_BACKWARD：查找不大于目标位置的最近关键帧（保证跳转准确性）
    int seekFlags = 0;
     if (position < m_maxPts) {
         seekFlags = AVSEEK_FLAG_BACKWARD;
     } else {
         seekFlags = AVSEEK_FLAG_ANY;
     }
    av_seek_frame(m_formatContext, -1, seekPos, seekFlags);  // 看到最后一个参数，我就知道你是回溯的罪魁祸首，啊啊啊啊啊啊！！！！
    log_info(QString("触发av_seek_frame， 目前的pos为：%1").arg(seekPos).toUtf8().data());

    // 清空之前的帧缓存，避免显示跳转前的旧帧
    {
        std::lock_guard<std::mutex> lock(m_setMutex);
        m_frameSet.clear();
        m_maxPts = -1;  // 重置最大PTS，允许新位置的帧插入
    }

//    // 通知UI更新进度条位置
//    m_currentPosition = position;  // 更新当前位置
    m_currentPosition = position;  // 更新当前位置
    emit positionChanged(position);
}

//void PlayerCore::setVolume(int volume)
//{
//    m_volume = volume;
//    qDebug() << "设置音量: " << volume;
//}

// 解码器初始化
void PlayerCore::initializeDecoder()
{
    // 初始化视频解码器
    // - 仅处理视频流（音频流当前未实现解码）
    if (m_videoStreamIndex < 0) return;

    // 获取视频流的编码参数（如 codec_id、宽高、像素格式等）
    AVCodecParameters *codecParams = m_formatContext->streams[m_videoStreamIndex]->codecpar;

    // 根据编码参数查找对应的解码器（如 H.264 解码器）
    const AVCodec *codec = avcodec_find_decoder(codecParams->codec_id);
    if (!codec) {
        emit errorOccurred("无法找到视频解码器");
        return;
    }

    // 创建解码器上下文（存储解码器状态的核心结构体）
    m_videoCodecContext = avcodec_alloc_context3(codec);
    if (!m_videoCodecContext) {
        emit errorOccurred("无法分配视频解码器上下文");
        return;
    }

    // 将流的编码参数复制到解码器上下文
    if (avcodec_parameters_to_context(m_videoCodecContext, codecParams) < 0) {
        emit errorOccurred("无法复制 codec 参数到上下文");
        return;
    }

    // 打开解码器（必须调用，否则无法解码）
    if (avcodec_open2(m_videoCodecContext, codec, nullptr) < 0) {
        emit errorOccurred("无法打开视频解码器");
        return;
    }

    // 创建图像转换上下文（将 FFmpeg 解码出的 YUV 格式转换为 Qt 可显示的 RGB32 格式）
    m_swsContext = sws_getContext(
                m_videoCodecContext->width, m_videoCodecContext->height, m_videoCodecContext->pix_fmt,  // 源格式（YUV）
                m_videoCodecContext->width, m_videoCodecContext->height, AV_PIX_FMT_RGB32,              // 目标格式（RGB32）
                SWS_BICUBIC, nullptr, nullptr, nullptr);  // 转换算法（双三次插值）
    if (!m_swsContext)
    {
            log_error("无法创建格式转换上下文, 准备退出");
            releaseResources();
            return;
    }

    // 分配 RGB 帧的缓冲区（存储转换后的 RGB 数据）
    int numBytes = av_image_get_buffer_size(AV_PIX_FMT_RGB32, m_videoCodecContext->width, m_videoCodecContext->height, 1);
    m_outBuffer = (uint8_t *)av_malloc(numBytes * sizeof(uint8_t));
    av_image_fill_arrays(m_rgbFrame->data, m_rgbFrame->linesize, m_outBuffer,
                         AV_PIX_FMT_RGB32, m_videoCodecContext->width, m_videoCodecContext->height, 1);

    // 计算视频帧率（用于控制播放速度）
    m_fps = av_q2d(m_formatContext->streams[m_videoStreamIndex]->avg_frame_rate);
    if (m_fps <= 0) {            // 当无法获取帧率时，使用默认值
        m_fps = 25.0;
        log_info("使用默认帧率25fps");
    }
//    m_frameDuration = 1000/m_fps;                 // 每帧理论持续时间（毫秒）
    m_frameDuration = 1.0/m_fps;

    // 确认帧率是否正确，判断定时器间隔是否需要调整
    log_info(QString("视频实际帧率：%1，每帧理论持续时间：%2ms").arg(m_fps).arg(m_frameDuration).toUtf8().data());
}

void PlayerCore::releaseResources()
{
    // 释放解码器资源
    if (m_videoCodecContext) {
        avcodec_free_context(&m_videoCodecContext);
        m_videoCodecContext = nullptr;
    }

//    if (m_audioCodecContext) {
//        avcodec_free_context(&m_audioCodecContext);
//        m_audioCodecContext = nullptr;
//    }

    if (m_swsContext) {
        sws_freeContext(m_swsContext);
        m_swsContext = nullptr;
    }

    if (m_outBuffer) {
        av_free(m_outBuffer);
        m_outBuffer = nullptr;
    }

    // 关闭文件
    if (m_formatContext) {
        avformat_close_input(&m_formatContext);
        m_formatContext = nullptr;
    }

    m_videoStreamIndex = -1;
//    m_audioStreamIndex = -1;
    m_isPlaying = false;
    m_duration = 0;
    m_frameSet.clear();
}

