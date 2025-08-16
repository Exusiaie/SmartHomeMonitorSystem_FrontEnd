#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "TitleBar.h"

#include <QDialog>      // 模态对话框
#include <QLineEdit>    // 单行对话框
#include <QPushButton>  // 按钮
#include <QLabel>       // 显示部件

class MainWindow : public QDialog
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    QString getUsername() const;    // 安全的获取m_pLineEditUser的text(用户输入的用户名)
    QString getPassword() const;    // 安全的获取m_pLineEditPwd的text(用户输入的密码)

private slots:
    void onLoginClicked();      // 处理登录逻辑
    void onRegisterClicked();   // 处理注册逻辑
//    void onTitleBarClose();     // 处理标题栏的关闭按钮

private:
    QLabel *m_pLabelUser;
    QLabel *m_pLabelPwd;

    QLineEdit *m_pLineEditUser;
    QLineEdit *m_pLineEditPwd;

    QPushButton *m_pBtnLogin;       // 注册按钮
    QPushButton *m_pBtnRegister;    // 登录按钮
    QPushButton *m_pBtnCancel;      // 取消按钮

    TitleBar     *m_pTitleBar;       // 标题栏
};

#endif // MAINWINDOW_H
