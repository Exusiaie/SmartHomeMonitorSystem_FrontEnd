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
    m_pLineEditPwd->setEchoMode(QLineEdit::Password);

    m_pBtnRegister = new QPushButton("注册", this);
    m_pBtnCancel = new QPushButton("取消", this);

    connect(m_pBtnRegister, &QPushButton::clicked, this, &Register::onRegisterClicked);
    connect(m_pBtnCancel, &QPushButton::clicked, this, &Register::reject);

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
    connect(m_pSocket, &QTcpSocket::connected, this, &Register::onSocketConnected);
    connect(m_pSocket, &QTcpSocket::readyRead, this, &Register::onSocketReadyRead);
}

Register::~Register()
{
}

// 按下注册按钮执行的逻辑
void Register::onRegisterClicked()
{
    m_username = m_pLineEditUser->text();   // 获取输入的用户名
    m_password = m_pLineEditPwd->text();    // 获取输入的密码

    if (m_username.isEmpty() || m_password.isEmpty()) {
        QMessageBox::warning(this, "提示", "用户名或密码不能为空！");
        return;
    }

// 构造 tlv_username
    tlv_username.type = 0x02;   // 类型为字符串
    tlv_username.length = m_username.length();  // 长度
    QByteArray bytearray_username = m_username.toUtf8();
    if (bytearray_username.size() < 1024) {
            memcpy(tlv_username.data, bytearray_username.constData(), bytearray_username.size());
            qDebug() << "tlv_username: " << tlv_username.type << tlv_username.length << tlv_username.data;
        } else {
            reject();
        }

// 构造 tlv_password
    tlv_password.type = 0x02;
    tlv_password.length = m_password.length();
    CryptString crypt(m_password);
    QString temp = crypt.generateMD5();
    qDebug() << "QString 加密后的password: " << temp;
    QByteArray crypt_password = temp.toUtf8();
    if (crypt_password.size() < 1024){
        memcpy(tlv_password.data, crypt_password.constData(), crypt_password.size());
        qDebug() << "tlv_password: " << tlv_password.type << tlv_password.length << tlv_password.data;
    }

// -------------------------------------------> 开始连接服务器, 服务器地址和端口在这里改 <-----------------------------------------
    m_pSocket->connectToHost("192.168.23.1", 8888);
    if (m_pSocket->state() == QAbstractSocket::ConnectedState) {
        // 连接成功
        sendUsername();
    } else {
        // 连接未成功，可能正在进行中或已失败
        qDebug() << "连接失败：" << m_pSocket->errorString();
        reject();
    }
}

void Register::onSocketConnected()
{
    // TCP 三次握手已经完成，开始发送用户名
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
    qDebug() << "最后要发送的用户名: " << data;

        // 使用 write() 函数发送数据
    m_pSocket->write(data);
    m_pSocket->flush();
}

void Register::sendPassword()
{
    // 将 type 和 length 转换成网络字节序
    wd::TLV tlv_to_send = tlv_password;
    tlv_to_send.type = qToBigEndian(tlv_to_send.type);
    tlv_to_send.length = qToBigEndian(tlv_to_send.length);

    // 将 tlv_password 结构体数据转换为 QByteArray
    QByteArray data;
    data.append(reinterpret_cast<const char*>(&tlv_password), sizeof(wd::TLV));
    qDebug() << "最后要发送的密码: " << data;

    // 使用 write() 函数发送数据
    m_pSocket->write(data);
    m_pSocket->flush();
}

void Register::onSocketReadyRead()
{
    qDebug() << "等待服务器发送OK1信号或ERROR1信号";
    QByteArray resp = m_pSocket->readAll(); // 读取服务器发来的信号

    if (!m_waitingPasswordStage) {
        // 第一阶段：验证用户名
        if (resp.contains(wd::TaskType::TASK_TYPE_LOGIN_SECTION1_RESP_OK)) {    // 用户名可以注册
            m_waitingPasswordStage = true;
            sendPassword();                                                     // 当用户名可以注册时再发送密码
        } else if (resp.contains((wd::TaskType::TASK_TYPE_LOGIN_SECTION1_RESP_ERROR))) {
            QMessageBox::critical(this, "注册失败", "用户名已存在或不合法！"); // 注册失败就结束
            reject();
        }
    } else {
        // 第二阶段：验证密码
        qDebug() << "等待服务器发送OK2信号";
        if (resp.contains(wd::TaskType::TASK_TYPE_LOGIN_SECTION2_RESP_OK)) {    // 用户名密码全部注册完成
            QMessageBox::information(this, "注册成功", "恭喜您，注册成功！");
            accept();
        } else {
            QMessageBox::critical(this, "注册失败", "服务器拒绝注册！");
            reject();
        }
    }
}
