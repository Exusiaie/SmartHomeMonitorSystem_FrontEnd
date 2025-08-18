#include "cframelesswidget.h"

#include <qt_windows.h>
#include <windows.h>
#include <windowsx.h>

#include <QPushButton>
#include <QMenu>

CFramelessWidget::CFramelessWidget(QWidget *parent)
    : QWidget(parent)
{
    this->setWindowFlags(Qt::FramelessWindowHint |
                         Qt::WindowMinMaxButtonsHint);
//    setAttribute(Qt::WA_Hover);

}

//该函数实现了窗口的拖拽拉伸
bool CFramelessWidget::nativeEvent(const QByteArray& eventType, void*
                                   message, long* result)
{
    MSG* param = static_cast<MSG*>(message);
    switch (param->message)
    {
    case WM_NCHITTEST:
    {
        int nX = GET_X_LPARAM(param->lParam) - this->geometry().x();
        int nY = GET_Y_LPARAM(param->lParam) - this->geometry().y();

        // 1. 先判断是否在标题栏区域（假设标题栏高度为42px，可根据实际调整）
        // 增加拖拽移动的判断
        if (nX > m_nBorderWidth && nX < this->width() - m_nBorderWidth &&
            nY > m_nBorderWidth && nY < this->height() - m_nBorderWidth)
        {
            if (childAt(nX, nY) != nullptr)
                return QWidget::nativeEvent(eventType, message, result);
            // 添加移动窗口的判断：如果点击的是可拖拽区域（如标题栏或空白处）
            *result = HTCAPTION; // 允许拖拽移动窗口
            return true;
        }
        const int titleBarHeight = 42; // 对应CTitleBar的setFixedHeight(42)
        if (nY >= 0 && nY < titleBarHeight)
        {
            // 检查标题栏内是否有子控件（如按钮）
            // - 如果点击的是空白区域则允许拖拽
            // - 若果不是按钮或者菜单也允许拖拽（不过看起来应该是失败了hhh）
            QWidget* child = childAt(nX, nY);
            if (!child || (qobject_cast<QPushButton*>(child) == nullptr && qobject_cast<QMenu*>(child) == nullptr)) // 无任何子控件，说明是空白区域
            {
                *result = HTCAPTION; // 允许拖拽移动窗口
                return true;
            }
        }

        // 2. 原有拉伸逻辑（保持不变）
        if (nX > m_nBorderWidth && nX < this->width() - m_nBorderWidth &&
                nY > m_nBorderWidth && nY < this->height() - m_nBorderWidth)
        {
            if (childAt(nX, nY) != nullptr)
                return QWidget::nativeEvent(eventType, message, result);
        }
        if ((nX > 0) && (nX < m_nBorderWidth))
            *result = HTLEFT;
        if ((nX > this->width() - m_nBorderWidth) && (nX < this->width()))
            *result = HTRIGHT;
        if ((nY > 0) && (nY < m_nBorderWidth))
            *result = HTTOP;
        if ((nY > this->height() - m_nBorderWidth) && (nY < this->height()))
            *result = HTBOTTOM;
        if ((nX > 0) && (nX < m_nBorderWidth) && (nY > 0)
                && (nY < m_nBorderWidth))
            *result = HTTOPLEFT;
        if ((nX > this->width() - m_nBorderWidth) && (nX < this->width())
                && (nY > 0) && (nY < m_nBorderWidth))
            *result = HTTOPRIGHT;
        if ((nX > 0) && (nX < m_nBorderWidth)
                && (nY > this->height() - m_nBorderWidth) && (nY < this->height()))
            *result = HTBOTTOMLEFT;
        if ((nX > this->width() - m_nBorderWidth) && (nX < this->width())
                && (nY > this->height() - m_nBorderWidth) && (nY < this->height()))
            *result = HTBOTTOMRIGHT;

        return true;
    }//end of case
    }//end of switch
    return false;
}
