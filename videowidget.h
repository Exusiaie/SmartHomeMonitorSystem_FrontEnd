/*
 * 创建一个自定义的QWidget子类，添加 setImage 方法来显示图像
 */

#ifndef VIDEOWIDGET_H
#define VIDEOWIDGET_H

#include <QWidget>
#include <QImage>

class VideoWidget : public QWidget
{
    Q_OBJECT
public:
    explicit VideoWidget(QWidget *parent = nullptr);
    ~VideoWidget();

    void setImage(const QImage &image);    // 设置要显示的图像
    void clearImage();                     // 清除当前显示的图像

protected:
    void paintEvent(QPaintEvent *event) override;    // 重绘事件

private:
    QImage m_currentImage; // 当前显示的图像
};

#endif // VIDEOWIDGET_H
