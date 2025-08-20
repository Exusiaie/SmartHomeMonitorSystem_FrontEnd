#ifndef CFRAMELESSWIDGET_H
#define CFRAMELESSWIDGET_H

#include <QWidget>

class CFramelessWidget : public QWidget
{
    Q_OBJECT
public:
    CFramelessWidget(QWidget *parent = nullptr);
    ~CFramelessWidget() = default;

protected:
    bool nativeEvent(const QByteArray& eventType, void* message, long*
                     result) override;
private:
    int m_nBorderWidth = 5;
};
#endif // CFRAMELESSWIDGET_H
