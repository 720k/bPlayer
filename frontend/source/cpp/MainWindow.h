#pragma once

#include <QtWidgets/QMainWindow>
#include <QLoggingCategory>
#include "LocalSocket.h"
#include "TcpSocket.h"
#include "ProtocolList.h"
class QLocalServer;
class StreamProtocolFileRead;
class TestProtocol;
class ControlProtocol;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_inputEdit_returnPressed();
    void on_pingButton_clicked();
    void pingReady(double microsecs);
    void on_connectButton_clicked();
    void connectionTypeChanged();
    void on_action_Open_triggered();
    void socketStateChanged(AbstractSocket::SocketState state);
    //void socketError(AbstractSocket);
    void on_playButton_clicked();
    void closeConnection();
    void onTok();
    void onEventStateChanged(quint64 state);

private:
    Ui::MainWindow *ui;
    enum class ConnectionState : int { Offline=0xd40000,
                                      Connecting=0xe67e22,
                                      ConnectingWait,
                                      Online=0xfff70a,
                                      Ready=0x00d600,
                                    };
    enum class MediaState : int    { None,
                                      Loading,
                                      Ready,
                                      Paused,
                                      Playing,
                                      Seeking};
    AbstractSocket                  *socket_ = nullptr;
    ProtocolList                    protocolList_;
    TestProtocol                    *testProtocol_;
    ControlProtocol                 *controlProtocol_;
    StreamProtocolFileRead          *streamProtocol_;
    ConnectionState                 connectionState_    = ConnectionState::Offline;
    MediaState                      mediaState_         = MediaState::None;
    static QMap<ConnectionState, QString>   stateName_;
    static constexpr int            invalidTimerID = -1;
    int                             tickingTimerID_ = invalidTimerID;
    QString                         fileName_;

    void                            updateWidgetStatus();
    void                            init();
    bool                            isLocalSocket() const;
    bool                            isOnline() const;
    void                            startConnecting();
    void                            retry() ;
    void                            startTicking();
    void                            stopTicking();
    void                            timerEvent(QTimerEvent *event) override;

};

Q_DECLARE_LOGGING_CATEGORY(catApp)
