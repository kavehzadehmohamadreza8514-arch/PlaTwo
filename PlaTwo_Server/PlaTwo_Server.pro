QT += core network
QT -= gui

CONFIG += c++17 console
CONFIG -= app_bundle

SOURCES += \
    main.cpp \
    servermanager.cpp \
    User.cpp \
    UserManager.cpp

HEADERS += \
    servermanager.h \
    User.h \
    UserManager.h
