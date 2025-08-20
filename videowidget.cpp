#include "videowidget.h"
#include "qss.h"
#include "log.h"

#include <QPainter>

VideoWidget::VideoWidget(QWidget *parent) : QWidget(parent)
{
//    setStyleSheet("background-color: black;");
    setStyleSheet("background-color: transparent;");
//    this->setStyleSheet(AppStyle::MAIN_STYLE);
}

VideoWidget::~VideoWidget()
{
}

void VideoWidget::setImage(const QImage &image)
{
    m_currentImage = image;
    update(); // 触发重绘
}

void VideoWidget::clearImage()
{
    m_currentImage = QImage();  // 清空当前图像
    update();  // 触发重绘
}

void VideoWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    if (m_currentImage.isNull()) {
        // 如果没有图像，绘制黑色背景
        painter.fillRect(rect(), Qt::transparent);
    } else {
        // 缩放图像以适应窗口
        QImage scaledImage = m_currentImage.scaled(
            size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);

        // 居中绘制图像
        int x = (width() - scaledImage.width()) / 2;
        int y = (height() - scaledImage.height()) / 2;
        painter.drawImage(x, y, scaledImage);
    }
}
