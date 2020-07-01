#include "NetworkLocalServer.h"
#include "ConsoleColors.h"
#include <QDebug>
#include <QFile>

NetworkLocalServer::NetworkLocalServer(QObject *parent) : QLocalServer(parent) {
    connect(this, &QLocalServer::newConnection, this, &NetworkLocalServer::newConnectionAvailable);
}

NetworkLocalServer::NetworkLocalServer(const QString &objectName, QObject *parent) : NetworkLocalServer(parent) {
    setObjectName(objectName);
}

void NetworkLocalServer::print(QString str, const QByteArray &data) {
    auto dbg = qDebug().noquote().nospace();
    QString prefix = QString("%1 %2:").arg(str).arg(data.size());
    prefix += QString(30-prefix.count(),' ');
    auto len = data.left(4).toHex(' ') + " ";
    auto id = data.mid(4,4).toHex(' ') + " ";
    auto other = data.mid(8,8).toHex(' ');
    dbg << prefix << cYLW  << len << cORG << id << cRST << other;
}


void NetworkLocalServer::writeData(QByteArray data) {
    print(objectName()+" Write", data);
    qDebug() << "-";
    if (socket_) {
        socket_->write(data);
        socket_->flush();
    } else {
        qDebug() << objectName() << " Socket is close, sink data";
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
    if (socket_) {
        socket_->flush();
        socket_->close();
    }
    deleteSocket();
    close();
}

void NetworkLocalServer::start(const QString &port) {
    if (QFile::exists(port))    QFile::remove(port);
    listen(port);
    qDebug() << objectName() << " is listening on:" << serverName();
}

void NetworkLocalServer::newConnectionAvailable() {
    if (socket_) {
        qDebug() << objectName() << "Sorry, I can accept only one connection";
        return;
    }
    socket_ = nextPendingConnection();
    connect(socket_, &QLocalSocket::stateChanged, this, &NetworkLocalServer::onSocketStateChanged);
    connect(socket_, &QLocalSocket::errorOccurred, this, &NetworkLocalServer::onSocketErrorOccurred);
    connect(socket_, &QLocalSocket::readyRead,      this, &NetworkLocalServer::onSocketData);
}

void NetworkLocalServer::onSocketErrorOccurred(QLocalSocket::LocalSocketError socketError) {  Q_UNUSED(socketError)
    qDebug() << objectName() << "Socket error: " << socket_->errorString();
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
    auto data = socket_->readAll();
    print(objectName()+" Read",data );
    emit dataReady(data);
}
