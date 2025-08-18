#include "Login.h"
#include <QDebug>
#include <QtEndian>
#include <QMessageBox>
#include <QNetworkProxy>

Login::Login(QDialog *parent, QString username, QString password)
    : m_pParent(parent),
      m_username(username),
      m_password(password),
      m_pSocket(new QTcpSocket(this)),
      m_waitingPasswordStage(false)
{
    // 禁用代理，确保正常连接
    m_pSocket->setProxy(QNetworkProxy::NoProxy);

    connect(m_pSocket, &QTcpSocket::connected, this, &Login::onSocketConnected);
    connect(m_pSocket, &QTcpSocket::readyRead, this, &Login::onSocketReadyRead);
    connect(m_pSocket, QOverload<QAbstractSocket::SocketError>::of(&QTcpSocket::error), this, &Login::onSocketError);

    qDebug() << "开始尝试连接服务器...";
    m_pSocket->connectToHost("47.111.20.8", 8888);
}

Login::~Login(){
    if (m_pSocket) {
        m_pSocket->close();
    }
}

void Login::onSocketConnected(){
    qDebug() << "成功连接到服务器，开始发送用户名。";
    sendUsername();
}

void Login::onSocketReadyRead()
{
    qDebug() << "收到了服务器的返回信息";
    // 将新到达的数据追加到缓冲区
    m_readBuffer.append(m_pSocket->readAll());

    // 解析循环：持续从缓冲区解析完整的包
    while (m_readBuffer.size() >= 8) {
        // 从缓冲区中读取 type 和 length
        const char *bufferData = m_readBuffer.constData();
        int type_be = *(reinterpret_cast<const int*>(bufferData));
        int length_be = *(reinterpret_cast<const int*>(bufferData + sizeof(int)));

        // 字节序转换：从大端（网络字节序）转为主机字节序
        int type = qFromBigEndian(type_be);
        int length = qFromBigEndian(length_be);

        // 检查缓冲区中是否包含完整的包
        if (m_readBuffer.size() >= (qint64)(2 * sizeof(int)) + length) {
            // 创建一个 Packet 对象来存储解析结果
            qDebug() << "已经收到了一个完整的服务器返回的包";
            wd::Packet packet;
            packet.type = type;
            packet.length = length;

            // 复制 data 部分，并创建 string
            std::string msgData(bufferData + (2 * sizeof(int)), length);
            packet.msg = msgData;

            // 处理解析出的包
            parseAndProcess(packet);

            // 从缓冲区中移除已处理的包
            m_readBuffer.remove(0, sizeof(wd::TLV) + length);
        } else {
            // 数据不完整，跳出循环等待更多数据
            break;
        }
    }
}

void Login::onSocketError(QAbstractSocket::SocketError socketError)
{
    qDebug() << "连接错误：" << m_pSocket->errorString();
    QMessageBox::critical(m_pParent, "网络错误", m_pSocket->errorString());
    emit loginFailed();
}

void Login::sendUsername()
{
    wd::TLV tlv_to_send;
    tlv_to_send.type = wd::TaskType::TASK_TYPE_LOGIN_SECTION1;

    QByteArray bytearray_username = m_username.toUtf8();
    tlv_to_send.length = bytearray_username.size();

    if (tlv_to_send.length < 1024) {
        memcpy(tlv_to_send.data, bytearray_username.constData(), tlv_to_send.length);
        qDebug() << "tlv_to_send (username): " << tlv_to_send.type << tlv_to_send.length << tlv_to_send.data;

        // 将 type 和 length 转换成网络字节序
        tlv_to_send.type = qToBigEndian(tlv_to_send.type);
        tlv_to_send.length = qToBigEndian(tlv_to_send.length);

        // 将结构体数据转换为 QByteArray
        QByteArray data;
        data.append(reinterpret_cast<const char*>(&tlv_to_send), sizeof(wd::TLV));
        m_pSocket->write(data);
        m_pSocket->flush();
        qDebug() << "已向服务器发送了Username";
    }
}

void Login::sendPassword(const std::string& salt)
{
    wd::TLV tlv_to_send;
    tlv_to_send.type = wd::TaskType::TASK_TYPE_LOGIN_SECTION2;

    // 将用户输入的密码与服务器返回的盐值进行拼接
    QString combined_password = m_password + QString::fromStdString(salt);

    // 对拼接后的字符串进行MD5加密
    CryptString crypt(combined_password);
    QString encrypted_password = crypt.generateMD5();

    // length 为加密后字符串的长度，通常为32个字符
    QByteArray bytearray_password = encrypted_password.toUtf8();
    tlv_to_send.length = bytearray_password.size();

    if (tlv_to_send.length < 1024){
        memcpy(tlv_to_send.data, bytearray_password.constData(), tlv_to_send.length);
        qDebug() << "加密并拼接后的密码: " << encrypted_password;
    } else {
        qDebug() << "加密后的密码过长，无法发送";
        QMessageBox::critical(m_pParent, "登录失败", "加密后的密码过长。");
        emit loginFailed();
        return;
    }

    // 将 type 和 length 转换成网络字节序
    tlv_to_send.type = qToBigEndian(tlv_to_send.type);
    tlv_to_send.length = qToBigEndian(tlv_to_send.length);

    // 将 tlv_to_send 结构体数据转换为 QByteArray
    QByteArray data;
    data.append(reinterpret_cast<const char*>(&tlv_to_send), sizeof(wd::TLV));

    m_pSocket->write(data);
    m_pSocket->flush();
       qDebug() << "密码已经发送";
}

void Login::parseAndProcess(wd::Packet packet)
{
    qDebug() << "接收到服务器数据，类型：" << packet.type << "长度：" << packet.length << "盐值：" << QString::fromStdString(packet.msg);

    if (!m_waitingPasswordStage) {
        // 第一阶段：验证用户名
        if (packet.type == wd::TaskType::TASK_TYPE_LOGIN_SECTION1_RESP_OK) {
            qDebug() << "收到服务器的OK，开始发送密码。";
            m_waitingPasswordStage = true;
            sendPassword(packet.msg); // 发送密码并附带服务器返回的盐值
        } else if (packet.type == wd::TaskType::TASK_TYPE_LOGIN_SECTION1_RESP_ERROR) {
            QMessageBox::critical(m_pParent, "登录失败", "用户名不存在，请注册！");
            emit loginFailed();
        } else {
            QMessageBox::critical(m_pParent, "登录失败", "服务器返回未知错误。");
            emit loginFailed();
        }
    } else {
        // 第二阶段：验证密码
        if (packet.type == wd::TaskType::TASK_TYPE_LOGIN_SECTION2_RESP_OK) {
            QMessageBox::information(m_pParent, "登录成功", "恭喜您，登录成功！");
            emit loginSuccess();
        } else if (packet.type == wd::TaskType::TASK_TYPE_LOGIN_SECTION2_RESP_ERROR) {
            QMessageBox::critical(m_pParent, "登录失败", "密码错误！");
            emit loginFailed();
        } else {
            QMessageBox::critical(m_pParent, "登录失败", "服务器返回未知错误。");
            emit loginFailed();
        }
    }
}


