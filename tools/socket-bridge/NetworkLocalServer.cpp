#include "NetworkLocalServer.h"
#include <QDebug>
#include <QFile>

NetworkLocalServer::NetworkLocalServer(QObject *parent) : QLocalServer(parent) {
    connect(this, &QLocalServer::newConnection, this, &NetworkLocalServer::newConnectionAvailable);
}

void NetworkLocalServer::writeData(QByteArray data) {
    if (socket_) {
        socket_->write(data);
        socket_->flush();
    } else {
        qDebug() << serverName() << " Socket is close, sink data";
    }
}

void NetworkLocalServer::deleteSocket() {
    if (socket_) {
        socket_->disconnect();
        socket_->deleteLater();
        socket_=nullptr;
    }
}

void NetworkLocalServer::stop() {
    if (socket_) socket_->close();
    deleteSocket();
    close();
}

void NetworkLocalServer::start(const QString &port) {
    if (QFile::exists(port)) {
        QFile::remove(port);
    }
    listen(port);
    qDebug() << serverName() << " is listening";
}

void NetworkLocalServer::newConnectionAvailable() {
    if (socket_) {
        qDebug() << serverName() << "Sorry I can accept only one connection";
        return;
    }
    socket_ = nextPendingConnection();
    connect(socket_, &QLocalSocket::stateChanged, this, &NetworkLocalServer::onSocketStateChanged);
    connect(socket_, &QLocalSocket::errorOccurred, this, &NetworkLocalServer::onSocketErrorOccurred);
    connect(socket_, &QLocalSocket::readyRead,      this, &NetworkLocalServer::onSocketData);
}

void NetworkLocalServer::onSocketErrorOccurred(QLocalSocket::LocalSocketError socketError) {  Q_UNUSED(socketError)
    qDebug() << serverName() << "Socket error: " << socket_->errorString();
    deleteSocket();
}

void NetworkLocalServer::onSocketStateChanged(QLocalSocket::LocalSocketState socketState) {
    if (socketState == QLocalSocket::UnconnectedState) {
        deleteSocket();
    }
    if (socketState == QLocalSocket::ConnectedState) {
    }
    emit socketStateChanged(socketState);
}

void NetworkLocalServer::onSocketData() {
    emit dataReady(socket_->readAll());
}
