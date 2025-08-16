#ifndef RIGHTSIDEBAR_H
#define RIGHTSIDEBAR_H

#include <QWidget>
#include <QTabWidget>
#include <QListWidget>
#include <QVBoxLayout>
#include <QLabel>

class RightSideBar : public QWidget
{
    Q_OBJECT

public:
    explicit RightSideBar(QWidget *parent = nullptr);
    ~RightSideBar();

    // 获取设备列表
    QListWidget* getDeviceList() const;
    // 更新系统信息
    void updateSystemInfo(const QString &cpuInfo, const QString &memoryInfo, const QString &systemInfo);

private:
    QTabWidget *tabWidget;
    QListWidget *deviceList;
    QWidget *systemInfoWidget;
    QLabel *cpuLabel;
    QLabel *memoryLabel;
    QLabel *systemLabel;
};

#endif // RIGHTSIDEBAR_H
