#include "playertools.h"
#include <QDateTime>
#include <QDir>
#include <QScreen>
#include <QApplication>
#include <QDebug>

PlayerTools::PlayerTools(QObject *parent) : QObject(parent)
{
}

PlayerTools::~PlayerTools()
{
}

void PlayerTools::takeScreenshot(QWidget *widget)
{
    if (!widget) return;

    // 创建截图目录
    QDir screenshotDir("screenshots");
    if (!screenshotDir.exists()) {
        screenshotDir.mkpath(".");
    }

    // 生成截图文件名
    QString filename = screenshotDir.absoluteFilePath(generateScreenshotFilename());

    // 截取窗口图像
    QPixmap screenshot = widget->grab();

    // 保存截图
    if (screenshot.save(filename)) {
        qDebug() << "截图已保存: " << filename;
    } else {
        qDebug() << "截图保存失败";
    }
}

void PlayerTools::switchLayout(int layoutType)
{
    qDebug() << "切换布局: " << layoutType;
    emit layoutChanged(layoutType);
}

QString PlayerTools::generateScreenshotFilename() const
{
    QDateTime now = QDateTime::currentDateTime();
    return QString("screenshot_%1.png").arg(now.toString("yyyyMMdd_HHmmss"));
}
