#include "AbstractSocket.h"
#include "Utils.h"
#include "ConsoleColors.h"

#include <QMap>
#include <QDebug>
#include <QIODevice>

AbstractSocket::AbstractSocket(QObject *parent) : QObject(parent), socket_(nullptr), state_(UnconnectedState)  {
    readBuffer_.reserve(128 * 1024);
}

AbstractSocket::~AbstractSocket() {
    if (socket_) socket_->deleteLater();
}


QString  AbstractSocket::stateString(SocketState state) {
    static QMap<SocketState, QString> description = {
        {UnconnectedState,      "Unconnected"},
        {HostLookupState,       "HostLookUp"},
        {ConnectingState,       "Connecting"},
        {ConnectedState,        "Connected"},
        {BoundState,            "Bound"},
        {ClosingState,          "Closing"},
        {ListeningState,        "Listening"},
    };
    return description.value(state,"NO StateString FOUND");
}

void AbstractSocket::connectToServer(const QString &name, const int port) {
    connectTo(name, port);
}

void AbstractSocket::disconnectFromServer() {
    disconnectFrom();
}

QString AbstractSocket::testPort() {
    static QString port {"/tmp/bplayer"};
    return port;
}

void AbstractSocket::writeMessage(const QByteArray &message) {
    if (!socket_->isOpen()) {
        CON << cLV0 << "Socket > Warning: sendMessage with socket closed." <<cEOL;
        return;
    }
    CON << cLV0 <<"Socket <==" << Utils::printableBA(message) << cEOL;
    readBuffer_.clear();
    stream_ << message;
    flushSocket();
}

void AbstractSocket::setSocket(QIODevice *socket) {
    stream_.setDevice(socket_ = socket);
    stream_.setVersion(QDataStream::Qt_5_12);
    connectSocketSignals();
}

QString AbstractSocket::serverName() const {
    return getServerName();
}

void AbstractSocket::readyRead() {
    stream_.startTransaction();
    int size = readBuffer_.size();
    stream_ >> readBuffer_;
    //#todo if readBuffer_ is not enough for the request?
    CON << cLV0 << "Socket > Read " <<  readBuffer_.size() - size << cEOL;
    if (!stream_.commitTransaction())  return; // message complete?
    CON << cLV0 << "Socket -->" << Utils::printableBA(readBuffer_) << cEOL;
    emit messageReady(readBuffer_);
    readBuffer_.clear();
}

void AbstractSocket::connected() {
    readBuffer_.clear();
}

void AbstractSocket::disconnected() {

}

void AbstractSocket::internalStateChanged(AbstractSocket::SocketState socketState) {
    CON << cLV0 << "Socket > state:" << socketState << " - " << stateString(socketState) << cEOL;
#ifdef Q_OS_WIN
    extern void GetLastErrorStdStr();
    if (socketState == AbstractSocket::SocketState::ClosingState) {
        qDebug() << "about to close";
        Utils::PrintLastErrorString();
    }
#endif
    emit stateChanged(state_= socketState);
}
