QT += core widgets

CONFIG += c++17

TARGET = DijkstraApp
TEMPLATE = app

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    dijkstra.cpp

HEADERS += \
    mainwindow.h \
    dijkstra.h

FORMS += \
    mainwindow.ui

