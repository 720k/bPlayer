#include "ConsoleApplication.h"
#include "Utils.h"
#include "ConsoleColors.h"

#include <QSharedMemory>
#include <QDebug>
#include <iostream>
#include <future>
#include <chrono>
#include <QTimer>

#include <QCommandLineParser>
#include <QDir>

#include "AbstractSocket.h"
#include "ConsoleColors.h"
#include "TestProtocol.h"
#include "ControlProtocol.h"
#include "StreamProtocol.h"
#include "MpvController.h"
#include "MpvSynchronousSocketStream.h"
#include "BPlayer.h"

Q_LOGGING_CATEGORY(catApp, "App")

using namespace std::literals::chrono_literals;

QMap<ConsoleApplication::ConnectionState, QString> ConsoleApplication::connectionStateName_ = {
    {ConnectionState::Offline,       "Offline"},
    {ConnectionState::Connecting,    "Connecting"},
    {ConnectionState::Online,        "Online"},
};


ConsoleApplication::ConsoleApplication(int &argc, char **argv) :QCoreApplication(argc,argv)  {
}

ConsoleApplication::~ConsoleApplication()   {
    closing();
}




void ConsoleApplication::searchConduitPath() {
    QString processName = "socket-bridge";
    // first check bridge!
    if (dataPortPath_.isEmpty())    dataPortPath_ = Utils::portNameFromProcess(BPlayer::controlPortName, processName);
    if (streamPortPath_.isEmpty())  streamPortPath_ = Utils::portNameFromProcess(BPlayer::streamPortName, processName);

    // then check Remote-Viewer process
    processName = "remote-viewer";
    if (dataPortPath_.isEmpty())    dataPortPath_ = Utils::portNameFromProcess(BPlayer::controlPortName, processName);
    if (streamPortPath_.isEmpty())  streamPortPath_ = Utils::portNameFromProcess(BPlayer::streamPortName, processName);

    // check existance
    if (!QFile::exists(dataPortPath_) || !QFile::exists(streamPortPath_)) {
        qCCritical(catApp) << dataPortPath_ << (QFile::exists(dataPortPath_) ? "FOUND" : "NOT FOUND");
        qCCritical(catApp) << streamPortPath_ << (QFile::exists(streamPortPath_) ? "FOUND" : "NOT FOUND");
        exit(20);
    }
    qCInfo(catApp) << "data port path = " << dataPortPath_;
    qCInfo(catApp) << "stream port path = " << streamPortPath_;
}

void ConsoleApplication::init()    {
    qCDebug(catApp).noquote() << cORG;
    searchConduitPath();

    dataSocket_.setObjectName("DataSocket");
    streamSocket_.setObjectName("StreamSocket");

    // DATA conduit
    dataProtocols_.insert(testProtocol_=     new TestProtocol());
    dataProtocols_.insert(controlProtocol_ = new ControlProtocol());
    dataProtocols_.printDispatchTable();
    connect(&dataSocket_,       &AbstractSocket::stateChanged,  this, &ConsoleApplication::socketStateChanged);
    connect(&dataSocket_,       &AbstractSocket::messageReady,  &dataProtocols_, &ProtocolList::decodeMessage);
    connect(&dataProtocols_,    &ProtocolList::messageReady,    &dataSocket_, &AbstractSocket::writeMessage);


    // STREAM conduit
    mpvSynchronousSocketStream_ = new MpvSynchronousSocketStream(this);
    streamProtocols_.insert(streamProtocol_=   new StreamProtocol(mpvSynchronousSocketStream_));
    streamProtocols_.printDispatchTable();

    mpvController_ = new MpvController(this);
    QObject::connect(qApp, &QCoreApplication::aboutToQuit, mpvController_, &MpvController::quit);
    mpvController_->registerHandler( mpvSynchronousSocketStream_->protocolName(), static_cast<void*>(mpvSynchronousSocketStream_), MpvSynchronousStreamInterface::open_cb);

    connect(&streamSocket_,     &AbstractSocket::stateChanged,  this, &ConsoleApplication::socketStateChanged);
    connect(&streamSocket_,     &AbstractSocket::messageReady,  &streamProtocols_, &ProtocolList::decodeMessage);
    connect(&streamProtocols_,  &ProtocolList::messageReady,    &streamSocket_, &AbstractSocket::writeMessage);

    // COMMANDS:    Control Protocol -> MPV Controller
    connect(controlProtocol_, &ControlProtocol::onMediaStart,  mpvController_, &MpvController::mediaStart);
    connect(controlProtocol_, &ControlProtocol::onMediaStop,  mpvController_, &MpvController::mediaStop);
    connect(controlProtocol_, &ControlProtocol::onMediaPause,  mpvController_, &MpvController::mediaPause);
    connect(controlProtocol_, &ControlProtocol::onMediaSeek,  mpvController_, &MpvController::mediaSeek);

    // EVENTS:      MPV Controller -> Control Protocol
    connect(mpvController_, &MpvController::eventStateChanged,  controlProtocol_, &ControlProtocol::eventStateChanged);
    connect(mpvController_, &MpvController::eventMediaLength,   controlProtocol_, &ControlProtocol::mediaLength);
    connect(mpvController_, &MpvController::eventPlaybackTime,  controlProtocol_, &ControlProtocol::playbackTime);

    tryConnecting();
}


void ConsoleApplication::closing()  {
    dataSocket_.disconnectFromServer();
    streamSocket_.disconnectFromServer();
    connectionState_ = ConnectionState::Offline;

    Utils::destroy(streamProtocol_);
    Utils::destroy(controlProtocol_);
    Utils::destroy(testProtocol_);
}


int ConsoleApplication::run() {
    if (isSingleInstance() && !isFirstInstance()) return 200;
    init();
    return exec();
}

bool ConsoleApplication::isFirstInstance() {
    static QSharedMemory sm("bplayer-backend-unique-instance-key");
    if (sm.attach()) {
        qCCritical(catApp).noquote() << "SHARED MEMORY ERROR > \n" << sm.error() << " - " << sm.errorString() << "\nkey: " << sm.key() << "\nNative Key: " << sm.nativeKey();
        return false;
    }
    if (!sm.create(1)) {
        qCCritical(catApp).noquote() << applicationName() << "  Error > Unable to create single instance.";
        return false;
    }
    return true;
}

void ConsoleApplication::checkState() {
    if (connectionState_ == ConnectionState::Connecting) {
        if (dataSocket_.isConnected() && streamSocket_.isConnected())   setConnectionState(ConnectionState::Online);
        else                                                            QTimer::singleShot(3s, this, &ConsoleApplication::tryConnecting);
    }
    if (connectionState_ == ConnectionState::Online) {
        if (dataSocket_.isUnconnected() || streamSocket_.isUnconnected()) {
            setConnectionState(ConnectionState::Connecting);
            QTimer::singleShot(3s, this, &ConsoleApplication::tryConnecting);
        }
    }
}

void ConsoleApplication::setConnectionState(ConsoleApplication::ConnectionState state) {
    if (state != connectionState_)  qCDebug(catApp).noquote() << " State > " << connectionStateName_.value(connectionState_ = state);
}

void ConsoleApplication::tryConnecting() {
    if (dataSocket_.isConnected() && streamSocket_.isConnected()) return;
    setConnectionState(ConnectionState::Connecting);
    if (dataSocket_.isUnconnected())    dataSocket_.connectToServer(dataPortPath_);
    if (streamSocket_.isUnconnected())  streamSocket_.connectToServer(streamPortPath_);
}

void ConsoleApplication::socketStateChanged(AbstractSocket::SocketState state) {
    if (state == AbstractSocket::SocketState::ConnectedState || state == AbstractSocket::SocketState::UnconnectedState)  checkState();
}

