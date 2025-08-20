#ifndef LEFTSIDEBAR_H
#define LEFTSIDEBAR_H

#include "playlistmanager.h"

#include <QWidget>
#include <QTabWidget>
#include <QListWidget>
#include <QVBoxLayout>
#include <QTimer>
#include <QPixmap>
#include <QDateTime>
#include <QDir>
#include <QFileInfo>

class LeftSideBar : public QWidget
{
    Q_OBJECT

public:
    explicit LeftSideBar(QWidget *parent = nullptr);
    ~LeftSideBar();

    QListWidget* getPlaylist() const;           // 获取播放列表
    QListWidget* getWindowInfoList() const;     // 获取窗口信息列表

    void setMainWindow(QWidget *mainWindow);    // 设置主窗口指针用于截屏
    void setCurrentMediaName(const QString &name);  // 设置当前媒体名称

signals:
    void playRequested(const QString &filePath);
    void stopRequested();

public slots:
    // - 截图所需信号槽
    void onPlaybackStarted();                   // 播放开始的槽函数
    void captureScreenshot();                   // 定时截屏槽函数

    // - 播放队列所需信号槽
    void onPlaybackFinished(bool success);
    void onAddMediaRequested(const QString &filePath);
    void onRemoveMediaRequested(int index);
    void clearPlaylist();

private:
    // - 截图
    void createScreenshotFolder();                     // 创建截图文件夹
    bool saveScreenshot(const QPixmap &screenshot);    // 保存截图到文件

    // - 播放列表
    void setupPlaylistTab();
    void addPlaylistItem(const QString &filePath);

private:
    // 常驻UI
    QTabWidget *tabWidget;
    QListWidget *windowInfoList;                // 截图列表
    QListWidget *playlist;                      // 播放队列

    // 截图所需数据成员
    QString m_currentMediaName;  // 添加：当前播放的媒体名称
    const int MAX_SCREENSHOTS = 32;  // 最大截图数量
    QTimer *m_screenshotTimer;  // 定时截屏器
    QWidget *m_mainWindow;      // 主窗口指针

    // 播放队列所需所需数据成员
    PlaylistManager *m_playlist;
};

#endif // LEFTSIDEBAR_H
