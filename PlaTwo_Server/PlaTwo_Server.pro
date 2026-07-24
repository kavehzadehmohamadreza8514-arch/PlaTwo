QT += core
QT -= gui

CONFIG += c++17 console
CONFIG -= app_bundle

LIBS += -lws2_32

HEADERS += \
    BaseGame.h \
    GameServer.h \
    GameSession.h \
    NetworkPacket.h \
    SavedGame.h \
    User.h \
    UserManager.h

SOURCES += \
    GameServer.cpp \
    NetworkPacket.cpp \
    User.cpp \
    UserManager.cpp \
    main.cpp
