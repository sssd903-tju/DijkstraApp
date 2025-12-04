QT += core widgets concurrent sql

# Qt6 默认使用 C++17，不需要显式指定
# CONFIG += c++17
CONFIG += c++17

# 修复 macOS 链接错误：移除 AGL framework 引用（AGL 是旧版 macOS 图形库，现代系统不需要）
# Qt6 需要 std::filesystem，最低要求 macOS 10.15
macx {
    # 移除 AGL framework
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
    graphdatabase.h

FORMS += \
    mainwindow.ui

