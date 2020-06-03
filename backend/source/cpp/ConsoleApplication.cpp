#include "ConsoleApplication.h"
#include "Utils.h"
#include "ConsoleColors.h"

#include <QSharedMemory>
#include <QDebug>
#include <iostream>
#include <future>
#include <QCommandLineParser>
#include "AbstractSocket.h"
#include "ConsoleColors.h"
#include "TestProtocol.h"
#include "ControlProtocol.h"
#include "StreamProtocol.h"
#include "MpvController.h"
#include "MpvSynchronousSocketStream.h"

Q_LOGGING_CATEGORY(catApp, "App")

void ConsoleApplication::init()    {
    mpvController_ = new MpvController(this);
    mpvSynchronousSocketStream_ = new MpvSynchronousSocketStream(this);
    streamProtocol_= new StreamProtocol(&protocolList_, mpvSynchronousSocketStream_);
    mpvSynchronousSocketStream_->setStreamProtocol(streamProtocol_);
    mpvController_->registerHandler( mpvSynchronousSocketStream_->protocolName(), static_cast<void*>(mpvSynchronousSocketStream_), MpvSynchronousStreamInterface::open_cb);

    QObject::connect(qApp, &QCoreApplication::aboutToQuit, mpvController_, &MpvController::quit);

    connect (&socket_, &AbstractSocket::stateChanged, this, &ConsoleApplication::socketStateChanged);
    // Client socket <-> BProtocol
    connect(&socket_,&AbstractSocket::messageReady,     &protocolList_, &ProtocolList::decodeMessage);
    connect(&protocolList_,&ProtocolList::messageReady, &socket_, &AbstractSocket::writeMessage);

    testProtocol_ = new TestProtocol(&protocolList_);
    controlProtocol_ = new ControlProtocol(&protocolList_);
    protocolList_.insert(testProtocol_);
    protocolList_.insert(controlProtocol_);
    protocolList_.insert(streamProtocol_);
    protocolList_.printDispatchTable();

    connect(controlProtocol_, &ControlProtocol::onMediaStart,  mpvController_, &MpvController::mediaStart);
    connect(controlProtocol_, &ControlProtocol::onMediaStop,  mpvController_, &MpvController::mediaStop);
    connect(controlProtocol_, &ControlProtocol::onMediaPause,  mpvController_, &MpvController::mediaPause);



    if (portName_.isEmpty())    portName_ = portNameFromExistingSocket(AbstractSocket::testPort());
    if (portName_.isEmpty())    portName_ = portNameFromProcess("remote-viewer");
    qCInfo(catApp) << "PORT NAME = " << portName_;
    socket_.connectToServer(portName_);
}

ConsoleApplication::ConsoleApplication(int &argc, char **argv) : QCoreApplication(argc,argv), singleInstance_(true), portName_("") {
}

ConsoleApplication::~ConsoleApplication()   {
    closing();
}

void ConsoleApplication::closing()  {
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
        qCWarning(catApp).noquote() << "SHARED MEMORY ERROR:\n" << sm.error() << " - " << sm.errorString() << "\nkey: " << sm.key() << "\nNative Key: " << sm.nativeKey();
        return false;
    }
    if (!sm.create(1)) {
        qCWarning(catApp).noquote() << applicationName() << " Application > Error : Unable to create single instance.";
        return false;
    }
    return true;
}

void ConsoleApplication::setPortName(const QString &portname) {
    portName_=portname;
}

void ConsoleApplication::socketStateChanged(AbstractSocket::SocketState state) {
    if (state == AbstractSocket::SocketState::ConnectedState) {
        qCDebug(catApp).noquote() <<  "Client Ready, connected to: " << socket_.serverName();
    }
    if (state == AbstractSocket::SocketState::UnconnectedState) {
        qCDebug(catApp).noquote() << "Client disconnected";
    }
    qCDebug(catApp).noquote() << " Socket state: " << AbstractSocket::stateString(state);
}

//void ConsoleApplication::error(QLocalSocket::LocalSocketError socketError)  {
//    qDebug() <<  QString("[ERROR] %1 : %2").arg(socketError).arg(socket_.errorString());
//}
