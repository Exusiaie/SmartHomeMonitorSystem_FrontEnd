#ifndef RIGHTSIDEBAR_H
#define RIGHTSIDEBAR_H

#include <QWidget>
#include <QTabWidget>
#include <QListWidget>
#include <QVBoxLayout>
#include <QTextEdit>
#include <QLabel>
#include "PanTiltControlWidget.h"

class RightSideBar : public QWidget
{
    Q_OBJECT

public:
    explicit RightSideBar(QWidget *parent = nullptr);
    ~RightSideBar();

    // 获取设备控制台
    QListWidget* getDeviceList() const;
    // 更新系统信息
    void updateSystemInfo(const QString &message);
    void switchToSystemInfoTab();    // 新增：切换到系统信息标签页

private:
    // 布局
    QTabWidget *m_tabWidget;

    // 设备控制台
    PanTiltControlWidget *m_pantiltControlWidget;
    QListWidget *m_deviceList;

    // 系统信息
    QWidget *m_systemInfoWidget;
    QLabel *m_deviceInfoLabel;
    QTextEdit *m_systemInfoTextEdit;
};

#endif // RIGHTSIDEBAR_H
