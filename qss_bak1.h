#ifndef QSS_H
#define QSS_H

#include <QString>

// 定义应用程序样式
namespace AppStyle {

// 主样式表
const QString MAIN_STYLE =
    "FFmpeg_Player { background-color: #555; color: white; }"
    "TitleBar { background-color: #555; color: white; }"
    "LeftSideBar { background-color: #555; color: white; }"
    "RightSideBar { background-color: #555; color: white; }"
    "CentralWidget { background-color: #555; color: white; }"
    "QSlider::handle { background-color: #555; color: white; }"
    "BottomBar { background-color: #555; color: white; }"
    "QPushButton { background-color: #333333; color: white; border-radius: 4px; }"
    "QPushButton:hover { background-color: #555555; }"
    "QSlider { background-color: #333333; }"
    "QSlider::handle { background-color: white; }"
    "CustomBar { background-color: #444; color: #eee; border: 1px solid #666; }"
    "QTabWidget { background-color: #777; }"
    "QTabWidget::pane { background-color: #777; border: 0px solid #ddd; }"
    "QTabBar { background-color: #555; }"
    "QTabBar::tab { background-color: #555; color: white; padding: 6px 12px; }"
    "QTabBar::tab:hover { background-color: #777; }"
    "QTabBar::tab:selected { background-color: #555; border-bottom: 2px solid white; }"
    "QListWidget { background-color: #777; }"
    "QListWidget::pane { background-color: #777; border: 0px solid #ddd; }";

} // namespace AppStyle

#endif // QSS_H
