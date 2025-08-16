#ifndef TITLEBAR_H
#define TITLEBAR_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>

class TitleBar : public QWidget
{
    Q_OBJECT
public:
    explicit TitleBar(QWidget *parent = nullptr);

protected:
    void mousePressEvent(QMouseEvent * event) override;

signals:
    void sigClose(); // 点击关闭按钮TitleBar发送的关闭信号

private:
    void initUI();

private slots:
    void onClickedSlot();

private:
    QLabel * _plogoLabel;
    QLabel * _ptitleTextLabel;
    QPushButton * _psetButton;
    QPushButton * _pminButton;
    QPushButton * _pmaxButton;
    QPushButton * _pcloseButton;

};


#endif // TITLEBAR_H
