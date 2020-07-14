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
class TestWindow;

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
    void connectionTypeChanged();
    void on_action_Open_triggered();
    void socketStateChanged(AbstractSocket::SocketState state);
    //void socketError(AbstractSocket);
    void on_playButton_clicked();
    void closeConnection();

    void onTok();
    void onEventStateChanged(quint32 state);
    void onMediaLength(quint32 length);
    void onPlaybackTime(quint32 time);
    void onEventVolume(quint32 volume);
    void onEventVolumeMax(quint32 volume);
    void onEventMute(bool mute);

    void on_actionTest_Window_triggered();
    void on_action_Quit_triggered();
    void on_stopButton_clicked();
    void on_timeSlider_sliderReleased();

    void on_muteButton_clicked();


    void on_volumeSlider_sliderMoved(int position);

private:
    Ui::MainWindow *ui;
    enum class ConnectionState : int { Offline=     0xc90000, // state = 0xRRGGBB color
                                      Connecting=   0xfcba03,
//                                      ConnectingWait,
                                      Online=       0x00c8ff,
                                      Ready=        0x07fa0b,
                                    };
    enum class MediaState : int    { None, Loading, Ready, Paused, Playing, Seeking};

    AbstractSocket                  *streamSocket_ =        nullptr;
    ProtocolList                    streamProtocols_;
    StreamProtocolFileRead          *streamProtocol_ =      nullptr;
    TestProtocol                    *testProtocol_ =        nullptr;
    ControlProtocol                 *controlProtocol_ =     nullptr;

    MediaState                      mediaState_         =   MediaState::None;
    ConnectionState                 connectionState_    =   ConnectionState::Offline;
    static QMap<ConnectionState, QString>   connectionStateName_;
    static constexpr int            invalidTimerID =        -1;
    int                             tickingTimerID_ =       invalidTimerID;
    QString                         fileName_;
    TestWindow                      *testWindow_ =          nullptr;
    friend class                    TestWindow;
    // local cache
    bool                            mute_ = false;

    void                            updateWidgetStatus();
    void                            init();
    bool                            isOnline() const { return connectionState_ == ConnectionState::Ready; }
    void                            tryConnecting();
    void                            startTicking();
    void                            stopTicking();
    void                            checkState();
    void                            setConnectionState(ConnectionState state);
    void                            timerEvent(QTimerEvent *event) override;
    void                            play(const QString &filename = QString());
};

Q_DECLARE_LOGGING_CATEGORY(catApp)
