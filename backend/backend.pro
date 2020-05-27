QT += core network
QT -= gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17 console


INCLUDEPATH += ../common
INCLUDEPATH += ./source/cpp
INCLUDEPATH += ./source/cpp/mpv

TARGET = bplayer-backend

CONFIG	+= link_pkgconfig
PKGCONFIG += mpv

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
    source/cpp/ConsoleApplication.cpp \
    source/cpp/__main.cpp \
    source/cpp/mpv/MpvController.cpp \
    source/cpp/mpv/MpvSynchronousSocketStream.cpp \
    source/cpp/mpv/MpvSynchronousStreamInterface.cpp \
    source/cpp/StreamProtocol.cpp \
    ../common/AbstractSocket.cpp \
    ../common/ControlProtocol.cpp \
    ../common/LocalSocket.cpp \
    ../common/Message.cpp \
    ../common/ProtocolBase.cpp \
    ../common/TcpSocket.cpp \
    ../common/TestProtocol.cpp \
    ../common/ProtocolList.cpp \
    ../common/Utils.cpp

HEADERS += \
    source/cpp/ConsoleApplication.h \
    source/cpp/common/ProtocolList.h \
    source/cpp/mpv/MpvController.h \
    source/cpp/mpv/MpvSynchronousSocketStream.h \
    source/cpp/mpv/MpvSynchronousStreamInterface.h \
    source/cpp/StreamProtocol.h \
    ../common/ProtocolList.h \
    ../common/AbstractSocket.h \
    ../common/ConsoleColors.h \
    ../common/ControlProtocol.h \
    ../common/LocalSocket.h \
    ../common/Message.h \
    ../common/ProtocolBase.h \
    ../common/TcpSocket.h \
    ../common/TestProtocol.h \
    ../common/Utils.h

FORMS +=

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
