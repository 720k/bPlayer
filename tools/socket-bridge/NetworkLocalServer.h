#pragma once
#include <QLocalServer>
#include <QLocalSocket>

class NetworkLocalServer : public QLocalServer
{
    Q_OBJECT
public:
                        NetworkLocalServer(QObject *parent = nullptr);

                        void deleteSocket();

signals:
    void                socketStateChanged(QLocalSocket::LocalSocketState socketState);
    void                dataReady(QByteArray data);
public slots:
    void                writeData(QByteArray data);
    void                stop();
    void                start(const QString& port);
private slots:
    void                newConnectionAvailable();
    void                onSocketErrorOccurred(QLocalSocket::LocalSocketError socketError);
    void                onSocketStateChanged(QLocalSocket::LocalSocketState socketState);
    void                onSocketData();

private:
    QLocalSocket*       socket_=nullptr;
};

