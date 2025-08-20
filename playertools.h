#ifndef PLAYERTOOLS_H
#define PLAYERTOOLS_H

#include <QObject>
#include <QWidget>
#include <QPixmap>

class PlayerTools : public QObject
{
    Q_OBJECT
public:
    explicit PlayerTools(QObject *parent = nullptr);
    ~PlayerTools();

    // 截图功能
    void takeScreenshot(QWidget *widget);

    // 布局切换功能
    void switchLayout(int layoutType);

signals:
    void layoutChanged(int layoutType);

private:
    QString generateScreenshotFilename() const;
};

#endif // PLAYERTOOLS_H
