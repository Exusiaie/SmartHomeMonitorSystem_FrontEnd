#include "PanTiltControlApi.h"
#include "log.h"

#include <QDateTime>
#include <QCryptographicHash>
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUrlQuery>
#include <QDebug>

PanTiltControlApi::PanTiltControlApi(QObject *parent)
    : QObject(parent),
      baseUrl("http://192.168.5.222/xsw/api/ptz/control"),
      secretKey("f6fdffe48c908deb0f4c3bd36c032e72")
{
    manager = new QNetworkAccessManager(this);
    connect(manager, &QNetworkAccessManager::finished, this, &PanTiltControlApi::onReplyFinished);
}

void PanTiltControlApi::sendPtzControl(const QString &value, int speed, int steps, int stop) {
    log_info(QString("【调试】开始构造云台控制 GET 请求，控制值 = %1").arg(value).toUtf8().data());

    QMap<QString, QString> params;
    params.insert("value", value);

    if (speed > 0) params.insert("speed", QString::number(speed));
    if (steps > 0) params.insert("steps", QString::number(steps));
    if (stop >= 0) params.insert("stop", QString::number(stop));

    // 添加签名参数
    QMap<QString, QString> signedParams = generateSignedParams(params);

    // 用 QUrlQuery 拼接参数
    QUrl url(baseUrl);
    QUrlQuery query;
    for (auto it = signedParams.begin(); it != signedParams.end(); ++it) {
        query.addQueryItem(it.key(), it.value());
    }
    url.setQuery(query);

    log_info(QString("【调试】最终请求 URL = = %1").arg(url.toString()).toUtf8().data());

    QNetworkRequest request(url);
    // GET 请求无需特别设置 Content-Type
    QNetworkReply *reply = manager->get(request);
    if (!reply) {
        log_error("【错误】无法创建网络请求 reply 对象");
        emit controlFailed("网络请求失败，无法创建 reply 对象");
    }
}

void PanTiltControlApi::onReplyFinished(QNetworkReply *reply) {
    if (reply->error() != QNetworkReply::NoError) {
        log_info(QString("【错误】网络请求失败: = %1").arg(reply->errorString()).toUtf8().data());
        emit controlFailed(reply->errorString());
        reply->deleteLater();
        return;
    }

    QByteArray responseData = reply->readAll();
    log_info(QString("【调试】服务器返回原始数据: = %1").arg(QString(responseData)).toUtf8().data());

    QJsonParseError jsonError;
    QJsonDocument doc = QJsonDocument::fromJson(responseData, &jsonError);

    if (jsonError.error != QJsonParseError::NoError) {
        log_info(QString("【错误】返回数据解析失败: %1").arg(jsonError.errorString()).toUtf8().data());
        emit controlFailed("返回数据解析失败");
        reply->deleteLater();
        return;
    }

    if (doc.isObject()) {
        QJsonObject obj = doc.object();
        int code = obj.value("code").toInt(-1);
        QString msg = obj.value("msg").toString();

        log_info(QString("【调试】返回 code = %1, msg = %2").arg(code).arg(msg).toUtf8().data());

        if (code == 0) {
            emit controlSucceeded(msg.isEmpty() ? "操作成功" : msg);
        } else {
            emit controlFailed(msg.isEmpty() ? QString("操作失败，code=%1").arg(code) : msg);
        }
    } else {
        log_info("【错误】返回数据不是 JSON 对象");
        emit controlFailed("返回数据不是 JSON 对象");
    }

    reply->deleteLater();
}

QMap<QString, QString> PanTiltControlApi::generateSignedParams(QMap<QString, QString> params) {
    QString t = QString::number(QDateTime::currentSecsSinceEpoch());
    params.insert("t", t);

    // 排序 keys
    QStringList keys = params.keys();
    keys << "secret";
    keys.sort();

    QString str;
    for (int i = 0; i < keys.size(); ++i) {
        QString key = keys.at(i);
        if (key == "token") continue;
        if (!str.isEmpty()) str += "&";
        if (key == "secret")
            str += key + "=" + secretKey;
        else
            str += key + "=" + params.value(key);
    }

    QString token = md5Hash(str);

    log_info(QString("【调试】生成签名字符串 = %1").arg(str).toUtf8().data());
    log_info(QString("【调试】生成 token = %1").arg(token).toUtf8().data());

    params.insert("token", token);
    return params;
}

QString PanTiltControlApi::md5Hash(const QString &input) {
    QByteArray byteArray = QCryptographicHash::hash(input.toUtf8(), QCryptographicHash::Md5);
    return QString(byteArray.toHex());
}
