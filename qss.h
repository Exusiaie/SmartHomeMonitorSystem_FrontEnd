#ifndef QSS_H
#define QSS_H

#include <QString>

// 定义应用程序样式
namespace AppStyle {

// 主样式表
const QString MAIN_STYLE =
    "FFmpeg_Player { background-image: url(:/Resource/background.png);background-repeat: no-repeat;background-position: center; background-origin: content; background-clip: content; }"
    "TitleBar { background-color: rgba(85, 85, 85, 0.5); color: white; }"
    "LeftSideBar { background-color: rgba(85, 85, 85, 0.1); color: white; }"
    "RightSideBar { background-color: rgba(85, 85, 85, 0.1); color: white; }"
    "CentralWidget { background-color: rgba(0, 0, 0, 0.1); color: white; }"
    "VideoWidget { background-color: rgba(0, 0, 0, 0.1); }"
    "QSlider::handle { background-color: rgba(85, 85, 85, 0.5); color: white; }"
    "BottomBar { background-color: rgba(85, 85, 85, 0.5); color: white; }"
    "QPushButton { background-color: transparent; color: white; border-radius: 4px; }"
    "QPushButton:hover { background-color: #555555; }"
    "QSlider { background-color: #333333; }"
    "QSlider::handle { background-color: white; }"
    "CustomBar { background-color: #444; color: #eee; border: 1px solid #666; }"
    "QTabWidget { background-color: rgba(85, 85, 85, 0.5); }"
    "QTabWidget::pane { background-color: rgba(85, 85, 85, 0.5); border: 0px solid #ddd; }"
    "QTabBar { background-color: #555; }"
    "QTabBar::tab { background-color: #555; color: white; padding: 6px 12px; }"
    "QTabBar::tab:hover { background-color: #777; }"
    "QTabBar::tab:selected { background-color: #555; border-bottom: 2px solid white; }"
    "QListWidget { background-color: rgba(85, 85, 85, 0.5); }"
    "QListWidget::pane { background-color: rgba(85, 85, 85, 0.5); border: 0px solid #ddd; }"
    "QTextEdit { background-color: rgba(85, 85, 85, 0.5); }";

} // namespace AppStyle

#endif // QSS_H
