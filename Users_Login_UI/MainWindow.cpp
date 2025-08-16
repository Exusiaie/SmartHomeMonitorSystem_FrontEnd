#include "MainWindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QDialog(parent)
{
    // 设置无边框 + 模态
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    setModal(true);
    setFixedSize(400, 250);


    // 用户名 Label+LineEdit
    m_pLabelUser = new QLabel("用户名:", this);
    m_pLineEditUser = new QLineEdit(this);

    // 密码 Label+LineEdit
    m_pLabelPwd = new QLabel("密码:", this);
    m_pLineEditPwd = new QLineEdit(this);
    m_pLineEditPwd->setEchoMode(QLineEdit::Password);

    // 按钮 三个Button
    m_pBtnLogin = new QPushButton("登录", this);
    m_pBtnRegister = new QPushButton("注册", this);
    m_pBtnCancel = new QPushButton("取消", this);

    // 标题栏
    m_pTitleBar = new TitleBar(this);

    // 信号槽
    connect(m_pBtnLogin, &QPushButton::clicked, this, &MainWindow::onLoginClicked);
    connect(m_pBtnRegister, &QPushButton::clicked, this, &MainWindow::onRegisterClicked);
    connect(m_pBtnCancel, &QPushButton::clicked, this, &MainWindow::reject); // QDialog 类的一个内置槽函数，拒绝(reject)对话框并将其关闭。
    connect(m_pTitleBar, &TitleBar::sigClose, this, &MainWindow::reject);

    // 布局: 这是用户名密码那两行
    QFormLayout *formLayout = new QFormLayout;
    formLayout->addRow(m_pLabelUser, m_pLineEditUser);
    formLayout->addRow(m_pLabelPwd, m_pLineEditPwd);

    // 布局: 这是三个按钮那一行
    QHBoxLayout *btnLayout = new QHBoxLayout;
    btnLayout->addWidget(m_pBtnLogin);
    btnLayout->addWidget(m_pBtnRegister);
    btnLayout->addWidget(m_pBtnCancel);

    // ---- 外层主布局：仅负责放标题栏 + 内容容器 ----
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);  // 让 TitleBar 真正贴边
    mainLayout->setSpacing(0);                   // TitleBar 与内容容器之间不留缝
    mainLayout->addWidget(m_pTitleBar);

    // ---- 内容容器：保留原有的 margin/spacing，不影响原有布局手感 ----
    QWidget *contentWidget = new QWidget(this);
    QVBoxLayout *contentLayout = new QVBoxLayout(contentWidget);
    // 这里设置你想要的常规边距与间距（Qt 默认大约是 9，你也可以自定义）
    contentLayout->setContentsMargins(12, 12, 12, 12);
    contentLayout->setSpacing(12);

    contentLayout->addLayout(formLayout);
    contentLayout->addLayout(btnLayout);

    mainLayout->addWidget(contentWidget);

    setLayout(mainLayout);

}

MainWindow::~MainWindow()
{
}

QString MainWindow::getUsername() const
{
    return m_pLineEditUser->text();
}

QString MainWindow::getPassword() const
{
    return m_pLineEditPwd->text();
}

void MainWindow::onLoginClicked()
{
    if (m_pLineEditUser->text().isEmpty() || m_pLineEditPwd->text().isEmpty()) {
        QMessageBox::warning(this, "提示", "用户名或密码不能为空！");
        return;
    }


    // ---- 这里加上验证逻辑-----


    accept();  // 登录成功，关闭对话框并返回 QDialog::Accepted
}

void MainWindow::onRegisterClicked()
{
    QMessageBox::information(this, "注册", "这里实现注册逻辑！");



    // ---- 这里加上注册逻辑-----


}


