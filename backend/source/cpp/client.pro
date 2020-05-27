QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# app.setWindowIcon(QIcon(QT_ICON_PATH)); // iconless on taskbar? not anymore.
QID=$$[QT_INSTALL_DATA]
QT_ICON= $$section(QID, /, 0, -3)/QtIcon-cyan.png
QT_ICON_STR = '\\"$${QT_ICON}\\"'
DEFINES += QT_ICON_PATH=\"$${QT_ICON_STR}\"

INCLUDEPATH += ./common
INCLUDEPATH += ./mpv
TARGET = client

unix: CONFIG	+= link_pkgconfig
unix: PKGCONFIG += mpv

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
    MainWindow.cpp \
    common/ProtocolList.cpp \
    mpv/MpvController.cpp \
    mpv/MpvSynchronousSocketStream.cpp \
    mpv/MpvSynchronousStreamInterface.cpp \
    StreamProtocol.cpp \
    __main.cpp \
    common/AbstractSocket.cpp \
    common/ControlProtocol.cpp \
    common/LocalSocket.cpp \
    common/Message.cpp \
    common/ProtocolBase.cpp \
    common/TcpSocket.cpp \
    common/TestProtocol.cpp \
    common/Utils.cpp

HEADERS += \
    MainWindow.h \
    common/ProtocolList.h \
    mpv/MpvController.h \
    mpv/MpvSynchronousSocketStream.h \
    mpv/MpvSynchronousStreamInterface.h \
    StreamProtocol.h \
    common/AbstractSocket.h \
    common/ConsoleColors.h \
    common/ControlProtocol.h \
    common/LocalSocket.h \
    common/Message.h \
    common/ProtocolBase.h \
    common/TcpSocket.h \
    common/TestProtocol.h \
    common/Utils.h

FORMS += \
    MainWindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
