#include "LocalSocket.h"

#define SKT (dynamic_cast<QLocalSocket*>(socket_))
using Parent = AbstractSocket;

LocalSocket::LocalSocket(QObject *parent) : AbstractSocket (parent) {
    Parent::setSocket(new QLocalSocket());
}

void LocalSocket::connectTo(const QString &name, const int port) {              Q_UNUSED(port)
    SKT->setServerName(name);
    SKT->connectToServer();
}

void LocalSocket::disconnectFrom() {
    SKT->disconnectFromServer();
}

void LocalSocket::connectSocketSignals() {
    connect (SKT, &QLocalSocket::connected,                                       this, &LocalSocket::connected);
    connect (SKT, &QLocalSocket::disconnected,                                    this, &LocalSocket::disconnected);
    connect (SKT, &QLocalSocket::readyRead,                                       this, &LocalSocket::readyRead);
    connect (SKT, &QLocalSocket::errorOccurred,                                   this, &LocalSocket::error);
    connect (SKT, &QLocalSocket::stateChanged,                                    this, &LocalSocket::stateChanged);

}

void LocalSocket::flushSocket() {
    SKT->flush();
}

QString LocalSocket::getServerName() const {
    return SKT->serverName();
}

void LocalSocket::connected() {
    Parent::connected();
}

void LocalSocket::disconnected() {
    Parent::disconnected();
}

void LocalSocket::error(QLocalSocket::LocalSocketError socketError) {
    qDebug() << "[ERROR] " << socketError << " " << socket_->errorString();
}

void LocalSocket::stateChanged(QLocalSocket::LocalSocketState socketState) {
    Parent::internalStateChanged(convertState(socketState));
}

void LocalSocket::readyRead() {
    Parent::readyRead();
}
AbstractSocket::SocketState LocalSocket::convertState(QLocalSocket::LocalSocketState socketState) {
    static QMap<QLocalSocket::LocalSocketState, AbstractSocket::SocketState> state = {
        { QLocalSocket::UnconnectedState, AbstractSocket::UnconnectedState},
        { QLocalSocket::ConnectingState, AbstractSocket::ConnectingState},
        { QLocalSocket::ConnectedState, AbstractSocket::ConnectedState},
        { QLocalSocket::ClosingState, AbstractSocket::ClosingState},
    };
    return state.value(socketState);
}

