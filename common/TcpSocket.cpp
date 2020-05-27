#include "TcpSocket.h"

#define SKT (dynamic_cast<QTcpSocket*>(socket_))
using Parent = AbstractSocket;


TcpSocket::TcpSocket(QObject *parent) : AbstractSocket (parent) {
    Parent::setSocket(new QTcpSocket());
    SKT->setSocketOption(QAbstractSocket::KeepAliveOption, 1);
}

void TcpSocket::connectTo(const QString &name, const int port) {
    SKT->connectToHost(name, port);
}

void TcpSocket::disconnectFrom() {
    SKT->disconnectFromHost();
}

void TcpSocket::connectSocketSignals() {
    connect (SKT, &QTcpSocket::connected,                                       this, &TcpSocket::connected);
    connect (SKT, &QTcpSocket::disconnected,                                    this, &TcpSocket::disconnected);
    connect (SKT, &QTcpSocket::readyRead,                                       this, &TcpSocket::readyRead);
    connect (SKT, &QTcpSocket::errorOccurred,                                   this, &TcpSocket::error);
    connect (SKT, &QTcpSocket::stateChanged,                                    this, &TcpSocket::stateChanged);

}

void TcpSocket::flushSocket() {
    SKT->flush();
}

QString TcpSocket::getServerName() const {
    return SKT->peerName()+":"+SKT->peerPort();
}

void TcpSocket::connected() {
    Parent::connected();
}

void TcpSocket::disconnected() {
    Parent::disconnected();
}

void TcpSocket::error(QAbstractSocket::SocketError socketError) {
    qDebug() << "[ERROR] " << socketError << " " << socket_->errorString();
}

void TcpSocket::stateChanged(QAbstractSocket::SocketState socketState) {
    Parent::internalStateChanged(convertState(socketState));
}

void TcpSocket::readyRead() {
    Parent::readyRead();
}
AbstractSocket::SocketState TcpSocket::convertState(QAbstractSocket::SocketState socketState) {
    static QMap<QAbstractSocket::SocketState, AbstractSocket::SocketState> state = {
        { QAbstractSocket::UnconnectedState, AbstractSocket::UnconnectedState},
        { QAbstractSocket::HostLookupState, AbstractSocket::HostLookupState},
        { QAbstractSocket::ConnectingState, AbstractSocket::ConnectingState},
        { QAbstractSocket::ConnectedState, AbstractSocket::ConnectedState},
        { QAbstractSocket::BoundState, AbstractSocket::BoundState},
        { QAbstractSocket::ClosingState, AbstractSocket::ClosingState},
        { QAbstractSocket::ListeningState, AbstractSocket::ListeningState},
    };
    return state.value(socketState);
}

