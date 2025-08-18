#include "playercore.h"
#include <QDebug>
#include <QThread>
#include <QImage>
#include <QDateTime>

// 注：其实FFpmeg有自己的日志，后面记得更换回来
#define log_info(msg) \
    qDebug("%s | %d | %s : %s", __FILE__, __LINE__, __FUNCTION__, msg);
#define log_error(msg) \
    qCritical("%s | %d | %s : [ERROR] %s", __FILE__, __LINE__, __FUNCTION__, msg);

PlayerCore::PlayerCore(QObject *parent) : QObject(parent)
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

    log_info("初始化完成");
}

PlayerCore::~PlayerCore()
{
    stop();

    if (m_decodeThread)
    {
        m_decodeThread->quit();
        m_decodeThread->wait();
        delete m_decodeThread;
    }

    releaseResources();

    // 释放帧和数据包
    if (m_frame)
        av_frame_free(&m_frame);
    if (m_rgbFrame)
        av_frame_free(&m_rgbFrame);
    if (m_packet)
        av_packet_free(&m_packet);
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
    m_audioStreamIndex = -1;
    for (int i = 0; i < m_formatContext->nb_streams; i++) {
        if (m_formatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            m_videoStreamIndex = i;
        } else if (m_formatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            m_audioStreamIndex = i;
        }
    }

    // 打印流信息
    log_info(QString("视频流索引: %1").arg(m_videoStreamIndex).toUtf8().data());
    log_info(QString("音频流索引: %1").arg(m_audioStreamIndex).toUtf8().data());

    // 设置总时长（毫秒）
//    m_duration = m_formatContext->duration/ AV_TIME_BASE;             // 秒为单位
    m_duration = m_formatContext->duration * 1000/ AV_TIME_BASE;        // 毫秒为单位
    emit durationChanged(m_duration);
    log_info(QString("视频总时长: %1").arg(m_duration).toUtf8().data()); // 视频总时长: 243158，换算为分钟是4分钟

    // 视频流时间基
    // - 时间基 1/12800 确认计算正确（512 × 1000 / 12800 = 40ms）
    AVRational tb = m_formatContext->streams[m_videoStreamIndex]->time_base;
    log_info(QString("视频流时间基: %1 / %2").arg(tb.num).arg(tb.den).toUtf8().data());

    log_info("openFile执行完成，准备播放");
    play();

    return true;
}

void PlayerCore::play()
{
    log_info("play开始");

    if (!m_formatContext || m_isPlaying) {
        return;
    }

    // 重置最大PTS（每次播放重新开始）
    m_maxPts = -1;

    // 初始化解码器
    m_decodeThread = new QThread(this);     // 指定parent
    connect(m_decodeThread, &QThread::started, [this]()
    {
        initializeDecoder();  // 需要在解码线程初始化
        decodeLoop();
    });
    log_info("初始化解码器执行完成");


    // 启动线程
//    this->moveToThread(m_decodeThread);  // 将整个对象移到新线程
    m_decodeThread->start();

    // 显示定时器（主线程）
    // 设置40ms间隔（25fps）
//    m_timer->setInterval(40);
    m_timer->setInterval(m_frameDuration);  // 用每帧持续时间作为间隔
    m_timer->start();
    m_isPlaying = true;

    log_info("开始播放");
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
    static qint64 lastDecodedPts = -1;  // 新增：记录上一次解码的PTS
    const int MIN_PTS_INTERVAL = m_frameDuration / 2;  // 最小允许的PTS间隔（20ms）

    // 循环解码，直到播放停止（m_isPlaying 为 false）
    while (m_isPlaying)
    {

        AVPacket packet;     // 存储编码的数据包（可能是视频或音频）
        // 从文件中读取一个数据包（音视频混合的包，需要按流索引区分）

        // 加锁保护read操作，与seek互斥
        int ret;
        {
            std::lock_guard<std::mutex> lock(m_formatMutex);
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

        // 增强日志：增加包大小、哈希值、流类型（音频/视频）
        QString streamType = (packet.stream_index == m_videoStreamIndex) ? "视频流" : "音频流";
        log_info(QString("读取数据包：序号=%1，流索引=%2（%3），PTS=%4，包大小=%5字节，哈希值=0x%6")
                 .arg(packetCount)
                 .arg(packet.stream_index)
                 .arg(streamType)
                 .arg(packet.pts)
                 .arg(packet.size)
                 .arg(packetHash, 8, 16, QChar('0')) // 哈希以8位十六进制显示
                 .toUtf8().data());

        // 仅处理视频流的数据包（音频流当前未处理，可忽略）
        if (packet.stream_index == m_videoStreamIndex)
        {
            // 1. 将视频数据包发送到解码器
            int ret = avcodec_send_packet(m_videoCodecContext, &packet);
            if (ret < 0 && ret != AVERROR(EAGAIN))  // EAGAIN 表示解码器需要更多数据，是正常情况
            {
                emit errorOccurred("发送数据包到解码器失败");
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
                    emit errorOccurred("接收解码帧失败");
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
                    emit errorOccurred("创建QImage失败：无效的图像数据");
                    av_frame_unref(m_frame);
                    continue;
                }

                // 5. 计算帧的显示时间戳（PTS，转换为毫秒）
                qint64 pts;
                if (m_frame->pts == AV_NOPTS_VALUE) {
                    log_info("帧无PTS，使用DTS替代");
                    pts = av_rescale_q(m_frame->pkt_dts,
                                       m_formatContext->streams[m_videoStreamIndex]->time_base,
                    {1, 1000});
                } else {
                    pts = av_rescale_q(m_frame->pts,
                                       m_formatContext->streams[m_videoStreamIndex]->time_base,
                    {1, 1000});
                }
                log_info(QString("原始PTS=%1，转换后PTS=%2").arg(m_frame->pts).arg(pts).toUtf8().data());

                // 6. 插入有序集合（自动排序，只保留PTS大于当前最大PTS的帧）
                std::lock_guard<std::mutex> lock(m_setMutex);
                if (pts > m_maxPts) {
                    // 集合满时删除最旧帧（PTS最小的）
                    if (m_frameSet.size() >= MAX_FRAME_COUNT) {
                        auto oldest = m_frameSet.begin();
                        log_info(QString("集合满，删除最旧帧：PTS=%1").arg(oldest->pts).toUtf8().data());
                        m_frameSet.erase(oldest);
                    }

                    // 插入新帧（自动排序+去重）
                    auto [it, inserted] = m_frameSet.insert({pts, std::move(image)});
                    if (inserted) {
                        m_maxPts = pts;  // 更新最大PTS
                        log_info(QString("插入帧：PTS=%1，集合大小=%2").arg(pts).arg(m_frameSet.size()).toUtf8().data());
                    } else {
                        log_info(QString("重复帧，跳过：PTS=%1").arg(pts).toUtf8().data());
                    }
                } else {
                    log_info(QString("旧帧，跳过：PTS=%1（当前最大PTS=%2）").arg(pts).arg(m_maxPts).toUtf8().data());
                }

                av_frame_unref(m_frame);// 释放当前帧资源（必须调用，否则内存泄漏）
            }
        }

        av_packet_unref(&packet);  // 释放数据包资源（必须调用）
    }

    // 播放结束后停止播放
    if (m_isPlaying)
    {
        stop();
    }
}

// 主线程帧显示（decodeNextFrame 队列版本）
void PlayerCore::decodeNextFrame() {
    if (!m_isPlaying || m_frameSet.empty()) return;  // 不播放或队列为空则退出

//    // 计算从播放开始到当前的毫秒数（作为参考时间）
//    qint64 currentSysTime = QDateTime::currentMSecsSinceEpoch();

    // 尝试加锁（非阻塞，避免主线程等待子线程）
    std::unique_lock<std::mutex> lock(m_setMutex, std::try_to_lock);
    if (!lock.owns_lock() || m_frameSet.empty()) return;  // 加锁失败或队列为空则退出

    static qint64 lastPTS = -1;  // 上一帧的PTS（初始为-1，表示未播放过）
    const int FRAME_INTERVAL = m_frameDuration;  // 40ms（每帧间隔）
    const int TOLERANCE = 100;  // 允许的误差范围（100ms）

    // 步骤1：删除所有过期帧（只保留上一帧前1秒内的帧）
    if (lastPTS != -1) {
//        VideoFrame expiredFrame{lastPTS - 1000, {}};
        VideoFrame expiredFrame{lastPTS - 100, {}};
        auto firstValid = m_frameSet.lower_bound(expiredFrame);
        if (firstValid != m_frameSet.begin()) {
            int erased = std::distance(m_frameSet.begin(), firstValid);
            m_frameSet.erase(m_frameSet.begin(), firstValid);
            log_info(QString("删除过期帧：共%1帧，剩余%2帧").arg(erased).arg(m_frameSet.size()).toUtf8().data());
        }
    }

    // 步骤2：确定当前应显示的帧
    // - 显示第一帧：直接取PTS最小的帧（通常是0）
    if (lastPTS == -1) {

        auto it = m_frameSet.begin();
        VideoFrame frame = *it;
        m_frameSet.erase(it);
        lock.unlock();

        log_info(QString("显示帧：PTS=%1（上一帧PTS=%2）")
                 .arg(frame.pts).arg(lastPTS).toUtf8().data());  // 修正日志，确保上一帧PTS正确;

        lastPTS = frame.pts;  // 记录第一帧PTS
        emit frameDecoded(frame.image);
        emit positionChanged(frame.pts);

        return;
    }
    else
    {
        // 后续帧：只要PTS > 上一帧PTS，就直接播放
        VideoFrame lowerBound{lastPTS, {}};
        auto it = m_frameSet.upper_bound(lowerBound);  // 第一个大于lastPTS的帧

        if (it != m_frameSet.end()) {
            // 找到符合条件的帧
            VideoFrame frame = *it;
            m_frameSet.erase(it);
            lock.unlock();

            lastPTS = frame.pts;
            emit frameDecoded(frame.image);
            emit positionChanged(frame.pts);
            log_info(QString("显示帧：PTS=%1（上一帧PTS=%2）")
                     .arg(frame.pts).arg(lastPTS).toUtf8().data());
            return;
        } else {
            // 无符合条件的帧（解码滞后）
            if (m_frameSet.empty()) {
                // 集合为空，没有任何帧
                log_info(QString("无任何帧：上一帧PTS=%1（需等待解码）")
                         .arg(lastPTS).toUtf8().data());
            } else {
                // 集合有帧，但都是不符合条件的（PTS小于等于上一帧）
                auto firstFrame = m_frameSet.begin();
                log_info(QString("无合适帧：上一帧PTS=%1，下一帧PTS=%2（需等待解码）")
                         .arg(lastPTS).arg(firstFrame->pts).toUtf8().data());
            }
            // 让出线程并休眠1ms（使用标准库方法）
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }
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
//    m_timer->stop();    // 定时器只能在主线程停止，通过信号槽确保线程安全
    QMetaObject::invokeMethod(m_timer, "stop", Qt::QueuedConnection);
    emit stopDecoding();

    // 释放解码器资源
    if (m_videoCodecContext) {
        avcodec_free_context(&m_videoCodecContext);
        m_videoCodecContext = nullptr;
    }
    if (m_audioCodecContext) {
        avcodec_free_context(&m_audioCodecContext);
        m_audioCodecContext = nullptr;
    }
    if (m_swsContext) {
        sws_freeContext(m_swsContext);
        m_swsContext = nullptr;
    }
    if (m_outBuffer) {
        av_free(m_outBuffer);
        m_outBuffer = nullptr;
    }

    emit stopStateChanged();
    qDebug() << "停止播放";
}

void PlayerCore::seek(qint64 position)
{
    if (!m_formatContext) {
        return;
    }

    // 将输入的毫秒位置转换为FFmpeg的时间基单位（微秒）
//    int64_t seekPos = position * AV_TIME_BASE;  // 毫秒为单位
//    int64_t seekPos = position * AV_TIME_BASE / 1000;  // 微秒为单位
    int64_t seekPos = position * AV_TIME_BASE / 1000;

    // 加锁确保与av_read_frame（读取数据包）操作互斥，避免冲突
    std::lock_guard<std::mutex> lock(m_formatMutex);
    // 调用FFmpeg的av_seek_frame实现跳转：移动文件指针到目标位置
    // 参数说明：
    // - m_formatContext：音视频格式上下文
    // - -1：表示对所有流（视频/音频）生效
    // - seekPos：目标位置（微秒）
    // - AVSEEK_FLAG_BACKWARD：查找不大于目标位置的最近关键帧（保证跳转准确性）
    av_seek_frame(m_formatContext, -1, seekPos, AVSEEK_FLAG_BACKWARD);  // 看到最后一个参数，我就知道你是回溯的罪魁祸首，啊啊啊啊啊啊！！！！
//    av_seek_frame(m_formatContext, -1, seekPos, 0);
    log_info(QString("触发av_seek_frame， 目前的pos为：%1").arg(seekPos).toUtf8().data());

    // 清空之前的帧缓存，避免显示跳转前的旧帧
    {
        std::lock_guard<std::mutex> lock(m_setMutex);
        m_frameSet.clear();
        m_maxPts = -1;  // 重置最大PTS，允许新位置的帧插入
    }

    // 通知UI更新进度条位置
    emit positionChanged(position);
}

void PlayerCore::setVolume(int volume)
{
    m_volume = volume;
    qDebug() << "设置音量: " << volume;
}

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
    m_frameDuration = 1000/m_fps;                 // 每帧理论持续时间（毫秒）

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

    if (m_audioCodecContext) {
        avcodec_free_context(&m_audioCodecContext);
        m_audioCodecContext = nullptr;
    }

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
    m_audioStreamIndex = -1;
    m_isPlaying = false;
    m_duration = 0;
    m_frameSet.clear();
}

