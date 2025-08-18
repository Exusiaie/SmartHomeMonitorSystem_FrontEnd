#ifndef REGISTER_H
#define REGISTER_H

#include "Message.h"
#include "CryptString.h"

#include <QDialog>
#include <QTcpSocket>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QString>
#include <QtEndian>
#include <QAbstractSocket>
#include <QThread>

#include <QNetworkProxy> // 禁用代理
class Register : public QDialog
{
    Q_OBJECT

public:
    explicit Register(QWidget *parent = nullptr);
    ~Register();

private slots:
    void onRegisterClicked();     // 点击“注册”按钮
    void onSocketConnected();     // TCP 连接成功
    void onSocketReadyRead();     // 接收服务器消息, 执行注册逻辑

    void onSocketDisconnected();
    void onSocketError(QAbstractSocket::SocketError socketError);


private:
    void sendUsername();          // 发送用户名
    void sendPassword(wd::Packet packet);          // 发送密码

    void parseAndProcess(wd::Packet packet); // <---------------新增的解析和处理函数


    QLineEdit *m_pLineEditUser;     // 输入用户名的Line
    QLineEdit *m_pLineEditPwd;      // 输入密码的Line
    QPushButton *m_pBtnRegister;    // 注册按钮
    QPushButton *m_pBtnCancel;      // 取消按钮

    QTcpSocket *m_pSocket;

    QString m_username; // 输入的用户名
    QString m_password; // 输入的密码

    wd::TLV tlv_username;   //
    wd::TLV tlv_password;   //

    QByteArray m_readBuffer; //  用于缓存接收数据的缓冲区
    QString salt;           // 存放盐值

    QString temp;

    bool m_waitingPasswordStage;  // 是否进入了“等待密码”阶段
};

#endif // REGISTER_H
