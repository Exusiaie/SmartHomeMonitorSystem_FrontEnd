#ifndef LOGIN_H
#define LOGIN_H

#include "message.h"
#include "cryptString.h"

#include <QDialog>
#include <QTcpSocket>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QString>
#include <QtEndian>
#include <QAbstractSocket>
#include <QThread>
#include <QObject>      // 继承自 QObject
#include <QMessageBox>  // 用于消息提示
#include <QNetworkProxy> // 禁用代理

class Login : public QObject{
    Q_OBJECT

public:
    // 构造函数，传入父窗口、用户名和密码
    explicit Login(QWidget *parent, QString username, QString password);
    ~Login();

signals:
    void systemMessage(const QString &message);
    void loginSuccess(); // 登录成功信号
    void loginFailed();  // 登录失败信号

private slots:
    void onSocketConnected();
    void onSocketReadyRead();
    void onSocketError(QAbstractSocket::SocketError socketError);

private:
    void sendUsername();
    void sendPassword(const std::string& salt);
    void parseAndProcess(wd::Packet packet);

private:
    QWidget *m_pParent; // 用于显示MessageBox的父窗口
    QString m_username;
    QString m_password;

    QTcpSocket *m_pSocket;

    QByteArray m_readBuffer; // 接收缓冲区
    bool m_waitingPasswordStage; // 登录状态机
};

#endif // LOGIN_H


