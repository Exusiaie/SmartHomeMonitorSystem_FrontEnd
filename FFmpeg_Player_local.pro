QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    FFmpeg_Player.cpp \
    bottombar.cpp \
    centralwidget.cpp \
    leftsidebar.cpp \
    main.cpp \
    playercore.cpp \
    playertools.cpp \
    rightsidebar.cpp \
    titlebar.cpp

HEADERS += \
    FFmpeg_Player.h \
    bottombar.h \
    centralwidget.h \
    leftsidebar.h \
    playercore.h \
    playertools.h \
    rightsidebar.h \
    titlebar.h

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

# 添加 MinGW 配置
win32-g++ {
    # MinGW 编译器配置
    QMAKE_CC = gcc
    QMAKE_CXX = g++
    QMAKE_LINK = g++

    # MinGW 库路径
    LIBS += -L"C:/Qt/Qt5.14.2/Tools/mingw730_32/lib"

    # 调试选项
    QMAKE_CXXFLAGS += -g
}

# 保留原有的 MSVC 配置
win32-msvc {
    # 编译器绝对路径
    QMAKE_CC = "C:/Program Files (x86)/Microsoft Visual Studio/2022/BuildTools/VC/Tools/MSVC/14.16.27023/bin/Hostx64/x86/cl.exe"
    QMAKE_CXX = $$QMAKE_CC
    QMAKE_LINK = $$QMAKE_CC

    # 库路径硬编码
    LIBS += -LIBPATH:"C:/Program Files (x86)/Microsoft Visual Studio/2022/BuildTools/VC/Tools/MSVC/14.16.27023/lib/x86"
    LIBS += -LIBPATH:"C:/Program Files (x86)/Windows Kits/10/Lib/10.0.19041.0/um/x86"
}

#message("Using compiler: $$QMAKE_CXX")

win32:
    # 明确指定库文件的完整名称（可选，但可避免歧义）
    LIBS += -L$$PWD/libffmpeg_4.4.r101753_msvc16_x86/lib/x86/
#    LIBS += -lavcodec.lib -lavdevice.lib -lavfilter.lib -lavformat.lib -lavutil.lib -lpostproc.lib -lswresample.lib -lswscale.lib

    # 根据编译器类型选择不同的库引用方式
    win32-g++:{
        # MinGW编译器使用带lib前缀的库名
        LIBS += -lavcodec -lavdevice -lavfilter -lavformat -lavutil -lpostproc -lswresample -lswscale
    }
    else:{
        # MSVC编译器使用不带lib前缀的库名
        LIBS += -lavcodec.lib -lavdevice.lib -lavfilter.lib -lavformat.lib -lavutil.lib -lpostproc.lib -lswresample.lib -lswscale.lib
    }

    # 头文件路径
    INCLUDEPATH += $$PWD/libffmpeg_4.4.r101753_msvc16_x86/include
    DEPENDPATH += $$PWD/libffmpeg_4.4.r101753_msvc16_x86/include

    # 可选：添加预编译宏（某些FFmpeg功能可能需要）
    DEFINES += __STDC_CONSTANT_MACROS

    # 确保32位编译（与x86库匹配）
    CONFIG += 32bit


