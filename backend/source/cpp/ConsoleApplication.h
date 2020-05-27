#pragma once

#include "LocalSocket.h"
#include "ProtocolList.h"
#include "AbstractSocket.h"

#include <QCoreApplication>

class TestProtocol;
class ControlProtocol;
class StreamProtocol;
class MpvController;
class MpvSynchronousSocketStream;

class ConsoleApplication : public QCoreApplication
{
    Q_OBJECT
public:
                            ConsoleApplication(int& argc, char** argv);
                            ~ConsoleApplication() override;
    int                     run();
    int                     start();
    bool                    isSingleInstance() const { return singleInstance_; }
    bool                    isFirstInstance();
    void                    setPortName(const QString& portname);
    QString                 portName() const { return portName_;}

private slots:
    void                    socketStateChanged(AbstractSocket::SocketState state);
//    void                    error(QLocalSocket::LocalSocketError socketError);
private:
    void                    init();
    void                    closing();
    QString                 portNameFromProcess(const QString &processName);
    QString                 portNameFromExistingSocket(const QString &socketName);

    bool                    singleInstance_;
    QString                 portName_;
    LocalSocket                     socket_;
    ProtocolList                    protocolList_;
    TestProtocol                    *testProtocol_;
    ControlProtocol                 *controlProtocol_;
    StreamProtocol                  *streamProtocol_;
    MpvController*                  mpvController_;
    MpvSynchronousSocketStream*     mpvSynchronousSocketStream_;
};

