#pragma once

#include "AbstractSocket.h"
#include <QtCore/QObject>
#include <QTcpSocket>

class TcpSocket : public AbstractSocket
{
public:
                        TcpSocket(QObject *parent = nullptr);
protected:
    void                connectTo(const QString &name, const int port) override;
    void                disconnectFrom() override;
    void                connectSocketSignals() override;
    void                flushSocket() override;
    QString             getServerName() const override;

    void                connected();
    void                disconnected();
    void                error(QTcpSocket::SocketError socketError);
    void                stateChanged(QTcpSocket::SocketState socketState);
    void                readyRead();
private:
    static SocketState  convertState(QTcpSocket::SocketState socketState) ;

    // AbstractSocket interface
protected:
};

