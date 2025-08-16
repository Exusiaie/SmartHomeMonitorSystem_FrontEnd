#include "rightsidebar.h"

RightSideBar::RightSideBar(QWidget *parent) : QWidget(parent)
{
    // 设置样式和大小
    setMinimumWidth(200);
    setMaximumWidth(250);
    setStyleSheet("background-color: #f0f0f0;");

    // 创建选项卡
    tabWidget = new QTabWidget(this);
    tabWidget->setStyleSheet("QTabWidget::pane { border: 1px solid #ddd; }");

    // 设备列表
    deviceList = new QListWidget();
    deviceList->addItem("设备 1");
    deviceList->addItem("设备 2");
    tabWidget->addTab(deviceList, "设备列表");

    // 系统信息
    systemInfoWidget = new QWidget();
    QVBoxLayout *systemLayout = new QVBoxLayout(systemInfoWidget);

    cpuLabel = new QLabel("CPU: Intel i5");
    memoryLabel = new QLabel("内存: 8GB");
    systemLabel = new QLabel("系统: Windows 10");

    systemLayout->addWidget(cpuLabel);
    systemLayout->addWidget(memoryLabel);
    systemLayout->addWidget(systemLabel);

    tabWidget->addTab(systemInfoWidget, "系统信息");

    // 添加布局
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(tabWidget);
    setLayout(layout);
}

RightSideBar::~RightSideBar()
{
}

QListWidget* RightSideBar::getDeviceList() const
{
    return deviceList;
}

void RightSideBar::updateSystemInfo(const QString &cpuInfo, const QString &memoryInfo, const QString &systemInfo)
{
    cpuLabel->setText(QString("CPU: %1").arg(cpuInfo));
    memoryLabel->setText(QString("内存: %1").arg(memoryInfo));
    systemLabel->setText(QString("系统: %1").arg(systemInfo));
}
