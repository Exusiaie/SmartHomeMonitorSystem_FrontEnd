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

    // 设置要显示的图像
    void setImage(const QImage &image);

protected:
    // 重绘事件
    void paintEvent(QPaintEvent *event) override;

private:
    QImage m_currentImage; // 当前显示的图像
};

#endif // VIDEOWIDGET_H
