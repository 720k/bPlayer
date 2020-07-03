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
    if (streamPortPath_.isEmpty())  streamPortPath_ = Utils::portNameFromProcess(BPlayer::streamPortName, processName);
    processName = "remote-viewer";
    if (streamPortPath_.isEmpty())  streamPortPath_ = Utils::portNameFromProcess(BPlayer::streamPortName, processName);
    if (!QFile::exists(streamPortPath_)) {
        qCCritical(catApp) << streamPortPath_ << (QFile::exists(streamPortPath_) ? "FOUND" : "NOT FOUND");
        exit(20);
    }
    qCInfo(catApp) << "stream port path = " << streamPortPath_;
}

void ConsoleApplication::init()    {
    qCDebug(catApp).noquote() << cORG;
    searchConduitPath();

    streamSocket_.setObjectName("StreamSocket");
    // STREAM conduit
    mpvSynchronousSocketStream_ = new MpvSynchronousSocketStream(this);
    streamProtocols_.insert(streamProtocol_=   new StreamProtocol(mpvSynchronousSocketStream_));
    streamProtocols_.insert(testProtocol_=     new TestProtocol());
    streamProtocols_.insert(controlProtocol_ = new ControlProtocol());
    streamProtocols_.printDispatchTable();

    mpvController_ = new MpvController(this);
    QObject::connect(qApp, &QCoreApplication::aboutToQuit, mpvController_, &MpvController::quit);
    mpvController_->registerHandler( mpvSynchronousSocketStream_->protocolName(), static_cast<void*>(mpvSynchronousSocketStream_), MpvSynchronousStreamInterface::open_cb);

    connect(&streamSocket_,     &AbstractSocket::stateChanged,  this, &ConsoleApplication::socketStateChanged);
    connect(&streamSocket_,     &AbstractSocket::messageReady,  &streamProtocols_, &ProtocolList::decodeMessage);
    connect(&streamProtocols_,  &ProtocolList::messageReady,    &streamSocket_, &AbstractSocket::writeMessage);

    // COMMANDS:    Control Protocol -> MPV Controller
    connect(controlProtocol_, &ControlProtocol::onMediaStart,   mpvController_, &MpvController::mediaStart);
    connect(controlProtocol_, &ControlProtocol::onMediaStop,    mpvController_, &MpvController::mediaStop);
    connect(controlProtocol_, &ControlProtocol::onMediaPause,   mpvController_, &MpvController::mediaPause);
    connect(controlProtocol_, &ControlProtocol::onMediaSeek,    mpvController_, &MpvController::mediaSeek);
    connect(controlProtocol_, &ControlProtocol::onSetVolume,    mpvController_, &MpvController::setVolume);
    connect(controlProtocol_, &ControlProtocol::onSetMute,      mpvController_, &MpvController::setMute);

    // EVENTS:      MPV Controller -> Control Protocol
    connect(mpvController_, &MpvController::eventStateChanged,  controlProtocol_, &ControlProtocol::eventStateChanged);
    connect(mpvController_, &MpvController::eventMediaLength,   controlProtocol_, &ControlProtocol::mediaLength);
    connect(mpvController_, &MpvController::eventPlaybackTime,  controlProtocol_, &ControlProtocol::playbackTime);
    connect(mpvController_, &MpvController::eventVolume,        controlProtocol_, &ControlProtocol::eventVolume);
    connect(mpvController_, &MpvController::eventVolumeMax,     controlProtocol_, &ControlProtocol::eventVolumeMax);
    connect(mpvController_, &MpvController::eventMute,          controlProtocol_, &ControlProtocol::eventMute);
    tryConnecting();
}


void ConsoleApplication::closing()  {
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
        if (streamSocket_.isConnected() && streamSocket_.isConnected())     setConnectionState(ConnectionState::Online);
        else                                                                QTimer::singleShot(3s, this, &ConsoleApplication::tryConnecting);
    }
    if (connectionState_ == ConnectionState::Online) {
        if (streamSocket_.isUnconnected() || streamSocket_.isUnconnected()) {
            setConnectionState(ConnectionState::Connecting);
            QTimer::singleShot(3s, this, &ConsoleApplication::tryConnecting);
        }
    }
}

void ConsoleApplication::setConnectionState(ConsoleApplication::ConnectionState state) {
    if (state != connectionState_)  qCDebug(catApp).noquote() << " State > " << connectionStateName_.value(connectionState_ = state);
}

void ConsoleApplication::tryConnecting() {
    if (streamSocket_.isConnected() && streamSocket_.isConnected()) return;
    setConnectionState(ConnectionState::Connecting);
    if (streamSocket_.isUnconnected())  streamSocket_.connectToServer(streamPortPath_);
}

void ConsoleApplication::socketStateChanged(AbstractSocket::SocketState state) {
    if (state == AbstractSocket::SocketState::ConnectedState || state == AbstractSocket::SocketState::UnconnectedState)  checkState();
}

