#ifndef PANTILTCONTROLAPI_H
#define PANTILTCONTROLAPI_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>

class PanTiltControlApi : public QObject {
    Q_OBJECT
public:
    explicit PanTiltControlApi(QObject *parent = nullptr);

    // 发送控制请求
    void sendPtzControl(const QString &value, int speed = 65, int steps = 1, int stop = 1);

signals:
    void controlSucceeded(const QString &msg);
    void controlFailed(const QString &errMsg);

private slots:
    void onReplyFinished(QNetworkReply *reply);

private:
    QString baseUrl;
    QString secretKey;
    QNetworkAccessManager *manager;

    QMap<QString, QString> generateSignedParams(QMap<QString, QString> params);
    QString md5Hash(const QString &input);
};

#endif // PANTILTCONTROLAPI_H
