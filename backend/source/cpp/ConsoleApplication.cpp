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
#include "AbstractSocket.h"
#include "ConsoleColors.h"
#include "TestProtocol.h"
#include "ControlProtocol.h"
#include "StreamProtocol.h"
#include "MpvController.h"
#include "MpvSynchronousSocketStream.h"

Q_LOGGING_CATEGORY(catApp, "App")

using namespace std::literals::chrono_literals;

QMap<ConsoleApplication::ConnectionState, QString> ConsoleApplication::connectionStateName_ = {
    {ConnectionState::Offline,       "Offline"},
    {ConnectionState::Connecting,    "Connecting"},
    {ConnectionState::ConnectingWait,"ConnectingWait"},
    {ConnectionState::Online,        "Online"},
};


void ConsoleApplication::startConnecting() {
    connectionState_ = ConnectionState::Connecting;
    socket_.connectToServer(portName_);
}


void ConsoleApplication::init()    {
    mpvSynchronousSocketStream_ = new MpvSynchronousSocketStream(this);
    protocolList_.insert(testProtocol_=     new TestProtocol());
    protocolList_.insert(controlProtocol_ = new ControlProtocol());
    protocolList_.insert(streamProtocol_=   new StreamProtocol(mpvSynchronousSocketStream_));
    protocolList_.printDispatchTable();

    mpvController_ = new MpvController(this);
    mpvController_->registerHandler( mpvSynchronousSocketStream_->protocolName(), static_cast<void*>(mpvSynchronousSocketStream_), MpvSynchronousStreamInterface::open_cb);

    QObject::connect(qApp, &QCoreApplication::aboutToQuit, mpvController_, &MpvController::quit);

    connect(&socket_,       &AbstractSocket::stateChanged,  this, &ConsoleApplication::socketStateChanged);
    connect(&socket_,       &AbstractSocket::messageReady,  &protocolList_, &ProtocolList::decodeMessage);
    connect(&protocolList_, &ProtocolList::messageReady,    &socket_, &AbstractSocket::writeMessage);

    connect(controlProtocol_, &ControlProtocol::onMediaStart,  mpvController_, &MpvController::mediaStart);
    connect(controlProtocol_, &ControlProtocol::onMediaStop,  mpvController_, &MpvController::mediaStop);
    connect(controlProtocol_, &ControlProtocol::onMediaPause,  mpvController_, &MpvController::mediaPause);
    connect(mpvController_, &MpvController::eventStateChanged, controlProtocol_, &ControlProtocol::eventStateChanged);


    if (portName_.isEmpty())    portName_ = portNameFromExistingSocket("/tmp/backend");
    if (portName_.isEmpty())    portName_ = portNameFromProcess("remote-viewer");
    qCInfo(catApp) << "PORT NAME = " << portName_;
    startConnecting();
}

ConsoleApplication::ConsoleApplication(int &argc, char **argv) :QCoreApplication(argc,argv), singleInstance_(true), portName_(""), connectionState_(ConnectionState::Offline)  {
}

ConsoleApplication::~ConsoleApplication()   {
    closing();
}

void ConsoleApplication::closing()  {
    socket_.disconnectFromServer();
    connectionState_ = ConnectionState::Offline;

    Utils::destroy(streamProtocol_);
    Utils::destroy(controlProtocol_);
    Utils::destroy(testProtocol_);
}

QString ConsoleApplication::portNameFromProcess(const QString &processName) {
    return  QString("/tmp/%1-%2/io.bplayer.data.0").arg(processName).arg(Utils::getProcessID(processName));
}

QString ConsoleApplication::portNameFromExistingSocket(const QString &socketName) {
    return QFile::exists(socketName) ? socketName : "";
}

int ConsoleApplication::run() {
    init();
    return exec();
}

int ConsoleApplication::start() {
    if (isSingleInstance()) {
        if (isFirstInstance())  return run();
        else                    return -3;
    } else {
        return run();
    }
}

bool ConsoleApplication::isFirstInstance() {
    static QSharedMemory sm("bplayer-backend-unique-instance-key");
    if (sm.attach()) {
        //QMessageBox::information(nullptr, applicationName(), "Application already runnning");
        qCWarning(catApp).noquote() << "SHARED MEMORY ERROR > \n" << sm.error() << " - " << sm.errorString() << "\nkey: " << sm.key() << "\nNative Key: " << sm.nativeKey();
        return false;
    }
    if (!sm.create(1)) {
        qCWarning(catApp).noquote() << applicationName() << "  Error > Unable to create single instance.";
        return false;
    }
    return true;
}

void ConsoleApplication::setPortName(const QString &portname) {
    portName_=portname;
}

void ConsoleApplication::socketStateChanged(AbstractSocket::SocketState state) {
    if (state == AbstractSocket::SocketState::ConnectedState) {
        connectionState_ = ConnectionState::Online;
    }
    if (state == AbstractSocket::SocketState::UnconnectedState) {
        connectionState_ = ConnectionState::ConnectingWait;
        QTimer::singleShot(3s, this, &ConsoleApplication::retry);
    }
    qCDebug(catApp).noquote() << " State > " << connectionStateName_.value(connectionState_);
}

void ConsoleApplication::retry() {
    if (connectionState_ == ConnectionState::ConnectingWait) {
        startConnecting();
    }
}
