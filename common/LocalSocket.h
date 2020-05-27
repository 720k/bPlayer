#pragma once

#include "AbstractSocket.h"
#include <QtCore/QObject>
#include <QLocalSocket>

class LocalSocket : public AbstractSocket
{
public:
                        LocalSocket(QObject *parent = nullptr);
protected:
    void                connectTo(const QString &name, const int port) override;
    void                disconnectFrom() override;
    void                connectSocketSignals() override ;
    void                flushSocket() override;
    QString             getServerName() const override;

    void                connected();
    void                disconnected();
    void                error(QLocalSocket::LocalSocketError socketError);
    void                stateChanged(QLocalSocket::LocalSocketState socketState);
    void                readyRead();
private:
    static SocketState  convertState(QLocalSocket::LocalSocketState socketState) ;

    // AbstractSocket interface
protected:
};

