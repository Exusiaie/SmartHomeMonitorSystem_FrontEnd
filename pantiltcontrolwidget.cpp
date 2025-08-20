#include "PanTiltControlWidget.h"
#include "log.h"

#include <QPixmap>
#include <QDebug>
#include <QFile>
#include <QPainter>
#include <QSvgRenderer>

PanTiltControlWidget::PanTiltControlWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
    setButtonStyles(); // 新增函数，用于设置按钮样式
    setupConnections();

    // 创建API实例并连接信号
       ptzApi = new PanTiltControlApi(this);
       connect(ptzApi, &PanTiltControlApi::controlSucceeded, this, [=](const QString& msg){
           log_info(QString("成功: %1").arg(msg).toUtf8().data());
           //  QMessageBox::information(this, "成功", msg);
       });
       connect(ptzApi, &PanTiltControlApi::controlFailed, this, [=](const QString& errMsg){
           qDebug() << "失败: " << errMsg;
       });
}

PanTiltControlWidget::~PanTiltControlWidget() {
}

//void PanTiltControlWidget::setupUi() {
//    // 设置窗口大小，这个尺寸需要和您的 background.png 图片大小匹配
//    // 假设图片尺寸是 300x300，可以根据实际情况调整
//    setFixedSize(300, 300);

//    // 1. 设置背景图片
//    backgroundLabel = new QLabel(this);
//    QPixmap panTiltPixmap(":/Resource/icons/background.png");
//    if (panTiltPixmap.isNull()) {
//        qDebug() << "Error: Could not load background.png.";
//        backgroundLabel->setText("Image not found!");
//    } else {
//        backgroundLabel->setPixmap(panTiltPixmap.scaled(size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
//        backgroundLabel->setGeometry(0, 0, width(), height());
//    }

//    // 2. 创建9个按钮
//    upButton = new QPushButton(this);
//    downButton = new QPushButton(this);
//    leftButton = new QPushButton(this);
//    rightButton = new QPushButton(this);
//    upLeftButton = new QPushButton(this);
//    upRightButton = new QPushButton(this);
//    downLeftButton = new QPushButton(this);
//    downRightButton = new QPushButton(this);
//    resetButton = new QPushButton(this);

//    // 设置按钮的尺寸和位置
//    // 这里的坐标和尺寸需要根据您的 background.png 图片精确调整，以下是示例值
//    int btnWidth = 50;
//    int btnHeight = 50;

//    // 定位中心重置按钮
//    resetButton->setGeometry(125, 125, btnWidth, btnHeight);

//    // 定位8个箭头按钮
//    upButton->setGeometry(125, 50, btnWidth, btnHeight);
//    downButton->setGeometry(125, 200, btnWidth, btnHeight);
//    leftButton->setGeometry(50, 125, btnWidth, btnHeight);
//    rightButton->setGeometry(200, 125, btnWidth, btnHeight);

//    upLeftButton->setGeometry(75, 75, btnWidth, btnHeight);
//    upRightButton->setGeometry(175, 75, btnWidth, btnHeight);
//    downLeftButton->setGeometry(75, 175, btnWidth, btnHeight);
//    downRightButton->setGeometry(175, 175, btnWidth, btnHeight);

//    // 设置按钮的名称以便在槽函数中识别
//    upButton->setObjectName("upButton");
//    downButton->setObjectName("downButton");
//    leftButton->setObjectName("leftButton");
//    rightButton->setObjectName("rightButton");
//    upLeftButton->setObjectName("upLeftButton");
//    upRightButton->setObjectName("upRightButton");
//    downLeftButton->setObjectName("downLeftButton");
//    downRightButton->setObjectName("downRightButton");
//    resetButton->setObjectName("resetButton");
//}

void PanTiltControlWidget::setupUi() {
    // 调整窗口大小以适应右侧边栏
    setFixedSize(200, 200);

    // 删除背景图片相关代码
    // 1. 设置背景图片
    // backgroundLabel = new QLabel(this);
    // QPixmap panTiltPixmap(":/Resource/icons/background.png");
    // if (panTiltPixmap.isNull()) {
    //     qDebug() << "Error: Could not load background.png.";
    //     backgroundLabel->setText("Image not found!");
    // } else {
    //     backgroundLabel->setPixmap(panTiltPixmap.scaled(size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    //     backgroundLabel->setGeometry(0, 0, width(), height());
    // }

    // 2. 创建9个按钮
    upButton = new QPushButton(this);
    downButton = new QPushButton(this);
    leftButton = new QPushButton(this);
    rightButton = new QPushButton(this);
    upLeftButton = new QPushButton(this);
    upRightButton = new QPushButton(this);
    downLeftButton = new QPushButton(this);
    downRightButton = new QPushButton(this);
    resetButton = new QPushButton(this);

    // 调整按钮的尺寸和位置以适应200x200的窗口
    int btnWidth = 30;  // 减小按钮宽度
    int btnHeight = 30; // 减小按钮高度

    // 定位中心重置按钮
    resetButton->setGeometry(85, 85, btnWidth, btnHeight);

    // 定位8个箭头按钮
    upButton->setGeometry(85, 30, btnWidth, btnHeight);
    downButton->setGeometry(85, 140, btnWidth, btnHeight);
    leftButton->setGeometry(30, 85, btnWidth, btnHeight);
    rightButton->setGeometry(140, 85, btnWidth, btnHeight);

    upLeftButton->setGeometry(55, 55, btnWidth, btnHeight);
    upRightButton->setGeometry(115, 55, btnWidth, btnHeight);
    downLeftButton->setGeometry(55, 115, btnWidth, btnHeight);
    downRightButton->setGeometry(115, 115, btnWidth, btnHeight);

    // 设置按钮的名称以便在槽函数中识别
    upButton->setObjectName("upButton");
    downButton->setObjectName("downButton");
    leftButton->setObjectName("leftButton");
    rightButton->setObjectName("rightButton");
    upLeftButton->setObjectName("upLeftButton");
    upRightButton->setObjectName("upRightButton");
    downLeftButton->setObjectName("downLeftButton");
    downRightButton->setObjectName("downRightButton");
    resetButton->setObjectName("resetButton");
}

void PanTiltControlWidget::setButtonStyles() {
    // 设置一个通用的按钮尺寸，与 setupUi 中的几何尺寸保持一致
    int btnWidth = 50;
    int btnHeight = 50;

    // 通用的样式，确保按钮透明，没有边框
    QString commonStyle = QString("QPushButton { border: none; background-color: transparent; }"
                                  "QPushButton:hover { background-color: rgba(255, 255, 255, 50); }" // 鼠标悬停时的半透明效果
                                  "QPushButton:pressed { background-color: rgba(255, 255, 255, 100); }"); // 按下时的半透明效果

    upButton->setStyleSheet(commonStyle + QString("QPushButton { image: url(:/Resource/icons/button_up.svg); image-size: %1px %2px; }").arg(btnWidth).arg(btnHeight));
    downButton->setStyleSheet(commonStyle + QString("QPushButton { image: url(:/Resource/icons/button_down.svg); image-size: %1px %2px; }").arg(btnWidth).arg(btnHeight));
    leftButton->setStyleSheet(commonStyle + QString("QPushButton { image: url(:/Resource/icons/button_left.svg); image-size: %1px %2px; }").arg(btnWidth).arg(btnHeight));
    rightButton->setStyleSheet(commonStyle + QString("QPushButton { image: url(:/Resource/icons/button_right.svg); image-size: %1px %2px; }").arg(btnWidth).arg(btnHeight));

    // 对于斜向按钮，需要设置对应的图标
    upLeftButton->setStyleSheet(commonStyle + QString("QPushButton { image: url(:/Resource/icons/button_left_up.svg); image-size: %1px %2px; }").arg(btnWidth).arg(btnHeight));
    upRightButton->setStyleSheet(commonStyle + QString("QPushButton { image: url(:/Resource/icons/button_right_up.svg); image-size: %1px %2px; }").arg(btnWidth).arg(btnHeight));
    downLeftButton->setStyleSheet(commonStyle + QString("QPushButton { image: url(:/Resource/icons/button_left_down.svg); image-size: %1px %2px; }").arg(btnWidth).arg(btnHeight));
    downRightButton->setStyleSheet(commonStyle + QString("QPushButton { image: url(:/Resource/icons/button_right_down.svg); image-size: %1px %2px; }").arg(btnWidth).arg(btnHeight));

    // 重置按钮
    resetButton->setStyleSheet(commonStyle + QString("QPushButton { image: url(:/Resource/icons/button_reset.svg); image-size: %1px %2px; }").arg(btnWidth).arg(btnHeight));
}

void PanTiltControlWidget::setupConnections() {
    connect(upButton, &QPushButton::clicked, this, &PanTiltControlWidget::onButtonClicked);
    connect(downButton, &QPushButton::clicked, this, &PanTiltControlWidget::onButtonClicked);
    connect(leftButton, &QPushButton::clicked, this, &PanTiltControlWidget::onButtonClicked);
    connect(rightButton, &QPushButton::clicked, this, &PanTiltControlWidget::onButtonClicked);
    connect(upLeftButton, &QPushButton::clicked, this, &PanTiltControlWidget::onButtonClicked);
    connect(upRightButton, &QPushButton::clicked, this, &PanTiltControlWidget::onButtonClicked);
    connect(downLeftButton, &QPushButton::clicked, this, &PanTiltControlWidget::onButtonClicked);
    connect(downRightButton, &QPushButton::clicked, this, &PanTiltControlWidget::onButtonClicked);
    connect(resetButton, &QPushButton::clicked, this, &PanTiltControlWidget::onButtonClicked);
}

void PanTiltControlWidget::onButtonClicked() {
    QPushButton* clickedButton = qobject_cast<QPushButton*>(sender());
       if (clickedButton) {
           QString buttonName = clickedButton->objectName();
           QString controlValue;

           if (buttonName == "upButton") {
               controlValue = "u";
           } else if (buttonName == "downButton") {
               controlValue = "d";
           } else if (buttonName == "leftButton") {
               controlValue = "l";
           } else if (buttonName == "rightButton") {
               controlValue = "r";
           } else if (buttonName == "upLeftButton") {
               controlValue = "1";
           } else if (buttonName == "upRightButton") {
               controlValue = "2";
           } else if (buttonName == "downLeftButton") {
               controlValue = "3";
           } else if (buttonName == "downRightButton") {
               controlValue = "4";
           } else if (buttonName == "resetButton") {
               // 注意：重置按钮的功能可能不是停止，这里我们假设它是一个特殊的重置命令
               // 如果它就是停止，请使用 's'
               controlValue = "s";
           }

           // 调用API发送控制命令
           if (!controlValue.isEmpty()) {
               ptzApi->sendPtzControl(controlValue);
           }
       }
}
