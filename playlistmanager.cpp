// playlistmanager.cpp
#include "playlistmanager.h"

PlaylistManager::PlaylistManager(QObject *parent) : QObject(parent) {}

void PlaylistManager::addMedia(const QString &filePath)
{
    m_playQueue.enqueue(filePath);
    emit playlistChanged();
}

void PlaylistManager::addMedias(const QStringList &filePaths)
{
    for (const auto &path : filePaths) {
        m_playQueue.enqueue(path);
    }
    emit playlistChanged();
}

void PlaylistManager::removeMedia(int index)
{
    if (index >= 0 && index < m_playQueue.size()) {
        // 手动重建队列（完全兼容方案）
        QQueue<QString> newQueue;
        for (int i = 0; i < m_playQueue.size(); ++i) {
            if (i != index) {
                newQueue.enqueue(m_playQueue.at(i));
            }
        }
        m_playQueue = newQueue;

        if (m_currentIndex >= index) {
            m_currentIndex--;
        }
        emit playlistChanged();
    }
}

void PlaylistManager::clear()
{
    m_playQueue.clear();
    m_currentIndex = -1;
    emit playlistChanged();
}

QString PlaylistManager::current() const
{
    return (m_currentIndex >= 0 && m_currentIndex < m_playQueue.size())
           ? m_playQueue.at(m_currentIndex) : QString();
}

QString PlaylistManager::next()
{
    if (m_playQueue.isEmpty()) return QString();

    m_currentIndex = (m_currentIndex + 1) % m_playQueue.size();
    return m_playQueue.at(m_currentIndex);
}

QString PlaylistManager::previous()
{
    if (m_playQueue.isEmpty()) return QString();

    m_currentIndex = (m_currentIndex - 1 + m_playQueue.size()) % m_playQueue.size();
    return m_playQueue.at(m_currentIndex);
}

QString PlaylistManager::at(int index) const
{
    return (index >= 0 && index < m_playQueue.size())
           ? m_playQueue.at(index) : QString();
}

bool PlaylistManager::isEmpty() const { return m_playQueue.isEmpty(); }
int PlaylistManager::count() const { return m_playQueue.size(); }
int PlaylistManager::currentIndex() const { return m_currentIndex; }

QStringList PlaylistManager::all() const
{
    QStringList list;
    for (const QString &item : m_playQueue) {
        list.append(item);
    }
    return list;
}
