#pragma once

#include <QtCore/QObject>
#include <QByteArray>
#include <QDataStream>
#include <QLoggingCategory>

class AbstractSocket : public QObject
{
    Q_OBJECT
public:


    enum                SocketState {UnconnectedState,HostLookupState, ConnectingState, ConnectedState, BoundState,ClosingState,ListeningState};

    explicit            AbstractSocket(QObject *parent = nullptr);
                        ~AbstractSocket();

    static QString      stateString(SocketState state);
    void                connectToServer(const QString &name, const int port=0) ;
    void                disconnectFromServer();
    SocketState         state() const { return state_;}
    static QString      testPort();
    void                setSocket(QIODevice *socket);
    QString             serverName() const;

public slots:
    void                writeMessage(const QByteArray& message);
signals:
    void                messageReady(const QByteArray& message);
    void                stateChanged(SocketState socketState);

protected slots:
    void                readyRead();
    void                connected();
    void                disconnected();
    void                internalStateChanged(SocketState socketState);
//    void                error(QTcpSocket::SocketError socketError);
protected:
    virtual void        connectTo(const QString &name, const int port) = 0;
    virtual void        disconnectFrom() = 0;
    virtual void        connectSocketSignals() = 0;
    virtual void        flushSocket() = 0;
    virtual QString     getServerName() const =0;
    QByteArray          readBuffer_;
    QDataStream         stream_;
    QIODevice           *socket_;
    SocketState         state_;
};

Q_DECLARE_LOGGING_CATEGORY(catSocket)

