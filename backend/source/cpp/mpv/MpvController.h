#pragma once

#include <QtCore/QObject>
#include <QFile>
#include <QMap>
#include <mpv/client.h>
#include <mpv/stream_cb.h>
#include <QElapsedTimer>

#include <QLoggingCategory>
Q_DECLARE_LOGGING_CATEGORY(catMpv)

class ControProtocol;

class MpvController : public QObject
{
    Q_OBJECT
public:
    explicit        MpvController(QObject *parent = nullptr);
                    ~MpvController();

    void            loadFile(const QString& filename, const QString& protocol=QString());
    static  void    controllerWakeup(void *ctx);
    void            registerHandler(const QString &name, void *instance, mpv_stream_cb_open_ro_fn open_fn);
signals:
    void            open(const QString& filename);
    void            replyReady();
    void            mpvEventReady();
    void            eventStateChanged(quint32 state);
    void            eventTimePos(quint32 positionInSecs);
    void            eventMediaLength(quint32 length);
    void            eventPlaybackTime(quint32 curr);
    void            eventVolumeMax(quint32 length);
    void            eventVolume(quint32 length);
    void            eventMute(bool mute);
public slots:
    void            mediaStart();
    void            mediaStop();
    void            mediaPause(bool isPaused);
    void            mediaSeek(int position);
    void            setVolume(quint32 volume);
    void            setMute(bool mute);
    void            quit();
private slots:
    void            handleMpvEvents();
    void            dispatchMpvEvent(mpv_event *event);
private:
    enum            ReplyID {Start=100, Stop, Pause, Seek, SetMute, SetVolume};
    static void     mpvEventReady_cb(void *ctx);
    ControProtocol  *controlProtocol_           = nullptr;
    QElapsedTimer   previousTimerEvent_;
    mpv_handle*     mpvHandle_                  = nullptr;
    QByteArray      uri_;
    // local cache: because these Vars are used in Async commands and passed by reference to mpv core
    int64_t         isPaused_;
    int64_t         isMute_;
    double          volume_;
};

