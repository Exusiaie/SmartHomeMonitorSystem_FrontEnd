#include "rightsidebar.h"
#include "qss.h"
#include "log.h"

RightSideBar::RightSideBar(QWidget *parent) : QWidget(parent)
{
    // 设置样式和大小
//    setMinimumWidth(200);
//    setMaximumWidth(250);
    setFixedWidth(225);
//    setStyleSheet("background-color: #f0f0f0;");

    // 创建选项卡
    m_tabWidget = new QTabWidget(this);
//    tabWidget->setStyleSheet("QTabWidget::pane { border: 1px solid #ddd; }");

    // 设备列表部分
    // - 布局
    QWidget *deviceTabWidget = new QWidget();
    QVBoxLayout *deviceLayout = new QVBoxLayout(deviceTabWidget);

    // - 控制台（上半部分）
    m_pantiltControlWidget = new PanTiltControlWidget();
    deviceLayout->addWidget(m_pantiltControlWidget);

    // - 控制台（下半部分）
    m_deviceInfoLabel = new QLabel("连接设备的信息显示在这里");
    deviceLayout->addWidget(m_deviceInfoLabel);
    m_deviceList = new QListWidget();

    // - 将设备控制页面添加到标签页
    m_tabWidget->addTab(deviceTabWidget, "设备控制");

    // 系统信息
    m_systemInfoWidget = new QWidget();
    QVBoxLayout *systemLayout = new QVBoxLayout(m_systemInfoWidget);

    // 创建纯文本框，设置为只读
    m_systemInfoTextEdit = new QTextEdit();
    m_systemInfoTextEdit->setReadOnly(true);
    m_systemInfoTextEdit->setLineWrapMode(QTextEdit::WidgetWidth);  // 自动换行
    m_systemInfoTextEdit->setStyleSheet("background-color: white;");

    systemLayout->addWidget(m_systemInfoTextEdit);

    m_tabWidget->addTab(m_systemInfoWidget, "系统信息");

    // 添加布局
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(m_tabWidget);
    setLayout(layout);

    this->setStyleSheet(AppStyle::MAIN_STYLE);
}

RightSideBar::~RightSideBar()
{
}

QListWidget* RightSideBar::getDeviceList() const
{
    return m_deviceList;
}

void RightSideBar::updateSystemInfo(const QString &message)
{
    // 向文本框添加新消息，自动换行
    m_systemInfoTextEdit->append(message);
}

void RightSideBar::switchToSystemInfoTab()
{
    // 系统信息标签页是第二个标签页（索引为1）
    m_tabWidget->setCurrentIndex(1);
}
