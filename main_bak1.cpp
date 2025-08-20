//// ============= "以下内容仅测试是否能够成功连接网路，不作为程序构成 ============="
//#include <iostream>
//#include <QDebug>
//#include <QApplication>

//// 包含FFmpeg头文件（根据你的路径调整）
//extern "C" {
//#include "libavformat/avformat.h"
//#include "libavutil/log.h"
//}

//// 回调函数：用于将FFmpeg的日志输出到Qt控制台
//void ffmpeg_log_callback(void *ptr, int level, const char *fmt, va_list vl) {
//    if (level > AV_LOG_WARNING) return; // 只输出警告及以上级别日志
//    char log[1024] = {0};
//    vsnprintf(log, sizeof(log), fmt, vl);
//    qDebug() << "[FFmpeg] " << log;
//}

//int main(int argc, char *argv[]) {
//    QCoreApplication a(argc, argv);

//    // 1. 初始化FFmpeg并设置日志
//    av_register_all();
//    avformat_network_init(); // 初始化网络模块（关键：用于HTTP流）
//    av_log_set_callback(ffmpeg_log_callback);
//    av_log_set_level(AV_LOG_INFO); // 设置日志级别

//    // 2. 要测试的流地址
//    const char* stream_url = "rtsp://admin:admin@192.168.5.222/live/chn=0";
////    const char* stream_url = "http://192.168.5.222:80/hdl/hlsram/live1.flv";

//    // 3. 打开流并获取格式上下文
//    AVFormatContext* fmt_ctx = nullptr;
//    int ret = avformat_open_input(&fmt_ctx, stream_url, nullptr, nullptr);
//    if (ret != 0) {
//        qCritical() << "无法打开流！错误代码：" << ret;
//        return -1;
//    }
//    qInfo() << "流打开成功！开始解析流信息...";

//    // 4. 查找流信息
//    ret = avformat_find_stream_info(fmt_ctx, nullptr);
//    if (ret < 0) {
//        qCritical() << "无法获取流信息！错误代码：" << ret;
//        avformat_close_input(&fmt_ctx);
//        return -1;
//    }

//    // 5. 输出流信息（视频/音频轨道数量、编码格式等）
//    qInfo() << "流信息解析成功：";
//    av_dump_format(fmt_ctx, 0, stream_url, 0); // 打印详细流信息

//    // 6. 查找视频和音频流
//    int video_stream_idx = -1, audio_stream_idx = -1;
//    for (unsigned int i = 0; i < fmt_ctx->nb_streams; i++) {
//        if (fmt_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
//            video_stream_idx = i;
//            qInfo() << "找到视频流，编码格式：" << avcodec_get_name(fmt_ctx->streams[i]->codecpar->codec_id);
//        } else if (fmt_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
//            audio_stream_idx = i;
//            qInfo() << "找到音频流，编码格式：" << avcodec_get_name(fmt_ctx->streams[i]->codecpar->codec_id);
//        }
//    }

//    if (video_stream_idx == -1 && audio_stream_idx == -1) {
//        qCritical() << "未找到视频或音频流！";
//        avformat_close_input(&fmt_ctx);
//        return -1;
//    }

//    // 7. 简单测试：读取10个数据包（验证流是否持续传输）
//    AVPacket* pkt = av_packet_alloc();
//    int packet_count = 0;
//    qInfo() << "开始读取数据包...";
//    while (packet_count < 10) {
//        ret = av_read_frame(fmt_ctx, pkt);
//        if (ret < 0) {
//            qWarning() << "读取数据包失败，可能已到流末尾。错误代码：" << ret;
//            break;
//        }
//        qInfo() << "读取到数据包：类型=" << (pkt->stream_index == video_stream_idx ? "视频" : "音频")
//                << "，大小=" << pkt->size << "字节";
//        av_packet_unref(pkt); // 释放数据包
//        packet_count++;
//    }
//    av_packet_free(&pkt);

//    // 8. 清理资源
//    avformat_close_input(&fmt_ctx);
//    avformat_network_deinit();
//    qInfo() << "测试完成，资源已释放。";

//    return a.exec();
//}
