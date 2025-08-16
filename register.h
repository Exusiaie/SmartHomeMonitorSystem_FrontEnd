#ifndef REGISTER_H
#define REGISTER_H

#include "message.h"
#include "cryptstring.h"

#include <QDialog>
#include <QTcpSocket>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QString>
#include <QtEndian>
#include <QAbstractSocket>

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

private:
    void sendUsername();          // 发送用户名
    void sendPassword();          // 发送密码

    QLineEdit *m_pLineEditUser;
    QLineEdit *m_pLineEditPwd;
    QPushButton *m_pBtnRegister;
    QPushButton *m_pBtnCancel;

    QTcpSocket *m_pSocket;

    QString m_username; // 输入的用户名
    QString m_password; // 输入的密码

    wd::TLV tlv_username;   // 最终发到网络上的包
    wd::TLV tlv_password;   //


    bool m_waitingPasswordStage;  // 是否进入了“等待密码”阶段
};

#endif // REGISTER_H
