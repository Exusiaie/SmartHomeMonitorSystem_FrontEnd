#ifndef PANTILTCONTROLWIDGET_H
#define PANTILTCONTROLWIDGET_H

#include "PanTiltControlApi.h"

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QMessageBox>


class PanTiltControlWidget : public QWidget {
    Q_OBJECT

public:
    explicit PanTiltControlWidget(QWidget *parent = nullptr);
    ~PanTiltControlWidget();

private slots:
    void onButtonClicked();

private:
    QLabel *backgroundLabel;
    QPushButton *upButton;
    QPushButton *downButton;
    QPushButton *leftButton;
    QPushButton *rightButton;
    QPushButton *upLeftButton;
    QPushButton *upRightButton;
    QPushButton *downLeftButton;
    QPushButton *downRightButton;
    QPushButton *resetButton;

    PanTiltControlApi *ptzApi;

    void setupUi();
    void setupConnections();
    void setButtonStyles();
};

#endif // PANTILTCONTROLWIDGET_H
