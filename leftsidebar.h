#ifndef LEFTSIDEBAR_H
#define LEFTSIDEBAR_H

#include <QWidget>
#include <QTabWidget>
#include <QListWidget>
#include <QVBoxLayout>

class LeftSideBar : public QWidget
{
    Q_OBJECT

public:
    explicit LeftSideBar(QWidget *parent = nullptr);
    ~LeftSideBar();

    // 获取播放列表
    QListWidget* getPlaylist() const;
    // 获取窗口信息列表
    QListWidget* getWindowInfoList() const;

private:
    QTabWidget *tabWidget;
    QListWidget *windowInfoList;
    QListWidget *playlist;
};

#endif // LEFTSIDEBAR_H
