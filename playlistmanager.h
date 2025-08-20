// playlistmanager.h
#ifndef PLAYLISTMANAGER_H
#define PLAYLISTMANAGER_H

#include <QObject>
#include <QQueue>
#include <QStringList>

class PlaylistManager : public QObject
{
    Q_OBJECT
public:
    explicit PlaylistManager(QObject *parent = nullptr);

    // 播放列表操作
    void addMedia(const QString &filePath);
    void addMedias(const QStringList &filePaths);
    void removeMedia(int index);
    void clear();

    // 播放控制
    QString current() const;
    QString next();
    QString previous();
    QString at(int index) const;

    // 状态查询
    bool isEmpty() const;
    int count() const;
    int currentIndex() const;
    QStringList all() const;

signals:
    void playlistChanged();

private:
    QQueue<QString> m_playQueue;
    int m_currentIndex = -1;
};

#endif // PLAYLISTMANAGER_H
