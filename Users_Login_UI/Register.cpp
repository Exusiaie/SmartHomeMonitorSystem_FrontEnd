#include "Register.h"
#include <QVBoxLayout>
#include <QFormLayout>
#include <QMessageBox>


Register::Register(QWidget *parent)
    : QDialog(parent),
      m_pSocket(new QTcpSocket(this)),
      m_waitingPasswordStage(false)
{
    setWindowTitle("用户注册");
    setModal(true);
    setFixedSize(300, 150);

    QLabel *labelUser = new QLabel("用户名:", this);
    QLabel *labelPwd = new QLabel("密码:", this);

    m_pLineEditUser = new QLineEdit(this);
    m_pLineEditPwd = new QLineEdit(this);
    m_pLineEditPwd->setEchoMode(QLineEdit::Password);   // 隐藏密码

    m_pBtnRegister = new QPushButton("注册", this);
    m_pBtnCancel = new QPushButton("取消", this);

    connect(m_pBtnRegister, &QPushButton::clicked, this, &Register::onRegisterClicked); // 注册按钮 ---> onRegisterClicked()
    connect(m_pBtnCancel, &QPushButton::clicked, this, &Register::reject);  // 取消按钮 ---> 取消

    QFormLayout *formLayout = new QFormLayout;
    formLayout->addRow(labelUser, m_pLineEditUser);
    formLayout->addRow(labelPwd, m_pLineEditPwd);

    QHBoxLayout *btnLayout = new QHBoxLayout;
    btnLayout->addWidget(m_pBtnRegister);
    btnLayout->addWidget(m_pBtnCancel);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(formLayout);
    mainLayout->addLayout(btnLayout);
    setLayout(mainLayout);

    // 绑定 socket 信号槽
    connect(m_pSocket, &QTcpSocket::connected, this, &Register::onSocketConnected); // 在connectToHost()并且连接建立成功后发送信号 ---> onSocketConnected
    connect(m_pSocket, &QTcpSocket::readyRead, this, &Register::onSocketReadyRead); // 在有新的套接字可以读取时 ---> onSocketReadyRead

    connect(m_pSocket, QOverload<QAbstractSocket::SocketError>::of(&QTcpSocket::error), this, &Register::onSocketError);
}

Register::~Register()
{
}

// 按下注册按钮执行的逻辑
void Register::onRegisterClicked()
{
    m_username = m_pLineEditUser->text();   // 获取输入的用户名
    m_password = m_pLineEditPwd->text();    // 获取输入的密码

    temp = m_pLineEditUser->text();//======================

    if (m_username.isEmpty() || m_password.isEmpty()) {
        QMessageBox::warning(this, "提示", "用户名或密码不能为空！");
        return;
    }

    // 构造 tlv_username
    tlv_username.type = wd::TaskType::TASK_TYPE_REGISTER_SECTION1;
    tlv_username.length = m_username.length();  // 长度
    QByteArray bytearray_username = m_username.toUtf8();
    if (bytearray_username.size() < 1024) {
        memcpy(tlv_username.data, bytearray_username.constData(), bytearray_username.size());
        qDebug() << "tlv_username: " << tlv_username.type << tlv_username.length << tlv_username.data;
    } else {
        reject();
    }



    // -------------------------------------------> 开始连接服务器, 服务器地址和端口在这里改 <-----------------------------------------

    qDebug() << "现在开始尝试连接";
    m_pSocket->setProxy(QNetworkProxy::NoProxy);    // 禁用代理
    // 如果 socket 未连接, 现在发起连接
    if (m_pSocket->state() == QAbstractSocket::UnconnectedState){
        // 否则，发起连接。当连接成功后，sendUsername() 会在 onSocketConnected() 中被调用。
        qDebug() << "发起连接";
        m_pSocket->connectToHost("47.111.20.8", 8888);
    }

}

void Register::onSocketConnected()
{
    qDebug() << "连接状态: " << m_pSocket->state();
    qDebug() << "成功连接到服务器，开始发送用户名。";    // TCP 三次握手已经完成，开始发送用户名
    sendUsername();
}


void Register::sendUsername()
{
    // 将 type 和 length 转换成网络字节序
    wd::TLV tlv_to_send = tlv_username;
    tlv_to_send.type = qToBigEndian(tlv_to_send.type);
    tlv_to_send.length = qToBigEndian(tlv_to_send.length);

    // 将 tlv_username 结构体数据转换为 QByteArray
    QByteArray data;
    data.append(reinterpret_cast<const char*>(&tlv_to_send), sizeof(wd::TLV));

    // 使用 write() 函数发送数据
    m_pSocket->write(data);
    m_pSocket->flush(); // 强制套接字立即将缓冲区中的所有数据发送出去。当你调用这个函数时，它会阻塞直到所有数据都被写入到底层网络套接字。
    qDebug() << "已发送Username";
    qDebug() << "连接状态: " << m_pSocket->state();
}

// QString -> QByteArray ->
void Register::sendPassword(wd::Packet packet)
{
    // 1. 将用户输入的密码与服务器返回的盐值进行拼接
    QString combined_password = m_password + QString::fromStdString(packet.msg);

    //2.  对拼接后的字符串进行MD5加密
    CryptString crypt(combined_password);
    QString encrypted_password = crypt.generateMD5();

    // 3. 构造 tlv_password
    tlv_password.type = wd::TaskType::TASK_TYPE_REGISTER_SECTION2; // 类型为字符串

    // length 为加密后字符串的长度 // -=============
    QByteArray bytearray_password =temp.toUtf8() + '\n' + encrypted_password.toUtf8();    // 把 QString 内部的 UTF-16 数据转换成 UTF-8 编码的字节数据，然后封装到一个 QByteArray 对象中
    tlv_password.length = bytearray_password.size();

    if (tlv_password.length < 1024){ //memcpy(目标位置, 原位置, 长度)
        memcpy(tlv_password.data, bytearray_password.constData(), tlv_password.length); // constData()返回一个 const char* 类型的指针，这个指针指向 QByteArray 内部数据块的第一个字节
        qDebug() << "拼接盐值并加密后的密码: " << encrypted_password;
    } else {
        // 异常处理
        qDebug() << "加密后的密码过长，无法发送";
        reject();
        return;
    }

    // 将 type 和 length 转换成网络字节序
    wd::TLV tlv_to_send = tlv_password; // 浅拷贝

    tlv_to_send.type = qToBigEndian(tlv_to_send.type);
    tlv_to_send.length = qToBigEndian(tlv_to_send.length);

    qDebug() << "149--> type: " << tlv_to_send.type << "lenth: " << tlv_to_send.length << "密码: " << tlv_to_send.data;
    // 将 tlv_password 结构体数据转换为 QByteArray
    QByteArray data;
    data.append(reinterpret_cast<const char*>(&tlv_to_send), sizeof(wd::TLV));

    // 使用 write() 函数发送数据
    m_pSocket->write(data);
    m_pSocket->flush();
    qDebug() << "发送完成";
}

void Register::onSocketReadyRead()
{
    qDebug() << "服务器已经发送OK1信号或ERROR1信号";

    m_readBuffer.append(m_pSocket->readAll()); // 将新到达的数据追加到缓冲区

    // 解析循环：持续从缓冲区解析完整的包
    // 如果缓冲区内数据的大小还不够构建一个本地的TLV结构体, 就继续等待够的数据
    while (m_readBuffer.size() >= 8) {

        // 创建一个 Packet 对象来存储解析结果
        wd::Packet packet;

        // 从缓冲区中读取 type 和 length
        const char *bufferData = m_readBuffer.constData();  // 转为C风格字符串
        int type_be = *(reinterpret_cast<const int*>(bufferData));  // 转为int类型并存入type_be
        int length_be = *(reinterpret_cast<const int*>(bufferData + sizeof(int))); // 转为int类型并存入length_be

        // 字节序转换：从大端（网络字节序）转为主机字节序
        packet.type = qFromBigEndian(type_be);
        packet.length = qFromBigEndian(length_be);

        qDebug() << "缓冲区中包的大小: " << (qint64)(2 * sizeof(int)) + packet.length;
        qDebug() << "服务器发送的返回值: " << packet.type;
        // 检查缓冲区中是否包含完整的包
        if (m_readBuffer.size() >= (qint64)(2 * sizeof(int)) + packet.length) {
            // 复制 data 部分，并创建 string
            std::string msgData(bufferData + (2 * sizeof(int)), packet.length);
            packet.msg = msgData;
            qDebug() << "盐值: " << QString::fromStdString(packet.msg);

            // 处理解析出的包
            parseAndProcess(packet);

            // 从缓冲区中移除已处理的包
            m_readBuffer.remove(0, (qint64)(2 * sizeof(int)) + packet.length);
        } else {
            qDebug() << "数据不完整，跳出循环等待更多数据";
            // 数据不完整，跳出循环等待更多数据
            break;
        }
    }
}

void Register::parseAndProcess(wd::Packet packet)
{

    if (!m_waitingPasswordStage) {
        // 第一阶段：验证用户名
            qDebug() << "开始验证用户名";
        if (packet.type == wd::TaskType::TASK_TYPE_REGISTER_SECTION1_RESP_OK) {
            qDebug() << "收到服务器的OK";
            m_waitingPasswordStage = true;
            sendPassword(packet);       // 当用户名可以注册时再发送密码
        } else if (packet.type == wd::TaskType::TASK_TYPE_REGISTER_SECTION1_RESP_ERROR) {
            qDebug() << "收到服务器的ERROR";
            QMessageBox::critical(this, "注册失败", "用户名已存在或不合法！");
            reject();
        }
    } else {
        qDebug() << "开始验证密码";
        // 第二阶段：验证密码
        qDebug() << "等待服务器发送OK2信号";
        if (packet.type == wd::TaskType::TASK_TYPE_REGISTER_SECTION2_RESP_OK) {
            qDebug() << "服务器发送的返回值: " << packet.type;
            QMessageBox::information(this, "注册成功", "注册成功!");
            accept();
        } else if (packet.type == wd::TaskType::TASK_TYPE_REGISTER_SECTION2_RESP_ERROR){
            qDebug() << "服务器发送的返回值: " << packet.type;
            QMessageBox::critical(this, "注册失败", "服务器拒绝注册！");
            reject();
        }
    }
}




void Register::onSocketDisconnected()
{
    // 连接已断开
    qDebug() << "Disconnected from server.";
}

void Register::onSocketError(QAbstractSocket::SocketError socketError)
{
    // 发生错误
    qDebug() << "连接出错: " << m_pSocket->errorString();
}
