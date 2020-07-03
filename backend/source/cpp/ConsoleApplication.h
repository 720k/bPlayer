#pragma once

#include "LocalSocket.h"
#include "ProtocolList.h"
#include "AbstractSocket.h"

#include <QCoreApplication>
#include <QLoggingCategory>
#include <QBitArray>

Q_DECLARE_LOGGING_CATEGORY(catApp)

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
    bool                    isSingleInstance() const { return singleInstance_; }
    bool                    isFirstInstance();
    void                    setStreamPortPath(const QString& streamPortPath) { streamPortPath_ = streamPortPath; }
    QString                 streamPortPath() const { return streamPortPath_; }
//    void                    setDataPortPath(const QString& dataPortPath) { dataPortPath_ = dataPortPath; }
//    QString                 dataPortPath() const { return dataPortPath_; }
    void                    searchConduitPath();

private slots:
    void                    socketStateChanged(AbstractSocket::SocketState state);
    void                    tryConnecting();
//    void                    error(QLocalSocket::LocalSocketError socketError);
private:

    enum class ConnectionState :int  { Offline=0xd40000, Connecting=0xe67e22, Online=0xfff70a};
    void                            init();
    void                            closing();
    void                            checkState();
    void                            setConnectionState(ConnectionState state);

    bool                            singleInstance_=    false;

    TestProtocol                    *testProtocol_=     nullptr;
    ControlProtocol                 *controlProtocol_=  nullptr;
    StreamProtocol                  *streamProtocol_=   nullptr;
    ConnectionState                 connectionState_=   ConnectionState::Offline;

    QString                         streamPortPath_=    "";
    LocalSocket                     streamSocket_;
    ProtocolList                    streamProtocols_;
    MpvController*                  mpvController_;
    MpvSynchronousSocketStream*     mpvSynchronousSocketStream_;
    static QMap<ConnectionState,QString>        connectionStateName_;
};

