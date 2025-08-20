#ifndef LOG_H
#define LOG_H

#include <QDebug>
#include <QFile>
#include <QDir>
#include <QDateTime>
#include <QTextStream>

// 日志文件路径
#define LOG_FILE_PATH "d:/White_Hat_Course/03_Embedded/CPP_QT_Project/04_Github_Sync_FFmpeg/local/FFmpeg_Player_local/Log/log.txt"

// 确保日志目录和文件存在
inline void ensureLogFileExists() {
    // 提取目录路径
    QString logDirPath = QString(LOG_FILE_PATH).left(QString(LOG_FILE_PATH).lastIndexOf('/'));

    // 创建目录（如果不存在）
    QDir logDir(logDirPath);
    if (!logDir.exists()) {
        logDir.mkpath(".");
    }

    // 创建文件（如果不存在）
    QFile logFile(LOG_FILE_PATH);
    if (!logFile.exists()) {
        logFile.open(QIODevice::WriteOnly | QIODevice::Text);
        logFile.close();
    }
}

// 写入日志到文件
inline void writeLogToFile(const QString &level, const QString &message) {
    ensureLogFileExists();

    QFile logFile(LOG_FILE_PATH);
    if (logFile.open(QIODevice::Append | QIODevice::Text)) {
        QTextStream out(&logFile);
        // 设置编码为UTF-8，解决中文乱码问题
        out.setCodec("UTF-8");
        QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
        out << QString("[%1] [%2] %3\n").arg(timestamp).arg(level).arg(message);
        logFile.close();
    }
}

// 日志宏定义 - 使用Qt的arg()方法
#define log_info(msg) \
    do { \
        QString logMsg = QString("%1 | %2 | %3 : %4").arg(__FILE__).arg(__LINE__).arg(__FUNCTION__).arg(msg); \
        qDebug().noquote() << logMsg; \
        writeLogToFile("INFO", logMsg); \
    } while(0)

#define log_error(msg) \
    do { \
        QString logMsg = QString("%1 | %2 | %3 : [ERROR] %4").arg(__FILE__).arg(__LINE__).arg(__FUNCTION__).arg(msg); \
        qCritical().noquote() << logMsg; \
        writeLogToFile("ERROR", logMsg); \
    } while(0)

#define log_warn(msg) \
    do { \
        QString logMsg = QString("%1 | %2 | %3 : [WARN] %4").arg(__FILE__).arg(__LINE__).arg(__FUNCTION__).arg(msg); \
        qWarning().noquote() << logMsg; \
        writeLogToFile("WARN", logMsg); \
    } while(0)

#endif // LOG_H
