TEMPLATE = app

MOC_DIR = build
UI_DIR = build
RCC_DIR = build
OBJECTS_DIR = build

QT += widgets gui core serialport network

CONFIG += c++11

SOURCES += main.cpp \
    Monster.cpp \
    Radar.cpp \
    MainWindow.cpp

#RESOURCES += vkns.qrc

FORMS += \
    ui/MainWindow.ui \
    ui/StateWidget.ui

HEADERS += \
    Monster.h \
    Radar.h \
    MainWindow.h

#version.commands = bash $$PWD/vkns-version.sh

#dpkg.CONFIG = phony
#dpkg.commands = bash $$PWD/monster-gui-dpkg.sh

#QMAKE_EXTRA_TARGETS += version dpkg
