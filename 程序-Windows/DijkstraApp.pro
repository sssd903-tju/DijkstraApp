QT += core widgets concurrent sql

# Qt5兼容：使用C++14，Qt6使用C++17
CONFIG += c++14

# Windows平台配置
win32 {
    # Windows版本信息
    VERSION = 1.0.0.0
    # 使用UTF-8编码（MSVC）
    QMAKE_CXXFLAGS_MSVC += /utf-8
    # MinGW使用UTF-8
    QMAKE_CXXFLAGS += -finput-charset=UTF-8 -fexec-charset=UTF-8
    # 链接Windows子系统（GUI应用，无控制台窗口）
    CONFIG += windows
    # 如果需要控制台窗口用于调试，可以注释掉上面一行，启用下面这行
    # CONFIG += console
}

# macOS平台配置
macx {
    # 移除 AGL framework 引用（AGL 是旧版 macOS 图形库，现代系统不需要）
    QMAKE_LIBS -= -framework AGL
    QMAKE_LFLAGS -= -framework AGL
    # Qt6 需要 macOS 10.15+（std::filesystem 要求）
    QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.15
    # 移除所有 AGL 相关的链接标志
    LIBS -= -framework AGL
}

TARGET = DijkstraApp
TEMPLATE = app

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    visualizationwindow.cpp \
    datamanagementwindow.cpp \
    databasemanagementwindow.cpp \
    dijkstra.cpp \
    dijkstra_loader.cpp \
    graphdatabase.cpp

HEADERS += \
    mainwindow.h \
    visualizationwindow.h \
    datamanagementwindow.h \
    databasemanagementwindow.h \
    dijkstra.h \
    dijkstra_loader.h \
    graphdatabase.h \
    windows_fixes.h

FORMS += \
    mainwindow.ui

