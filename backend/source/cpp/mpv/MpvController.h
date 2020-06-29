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
    void            eventStateChanged(quint64 state);
    void            eventTimePos(quint64 positionInSecs);
    void            eventMediaLength(quint64 length);
    void            eventPlaybackTime(quint64 length);
public slots:
    void            mediaStart();
    void            mediaStop();
    void            mediaPause(bool isPaused);
    void            mediaSeek(int position);
    void            quit();
private slots:
    void            handleMpvEvents();
    void            dispatchMpvEvent(mpv_event *event);
private:
    enum            ReplyID {Start=100, Stop, Pause, Seek,};
    static void     mpvEventReady_cb(void *ctx);
    mpv_handle*     mpvHandle_;
    QByteArray      uri_;
    // local cache:
    int64_t         isPaused_;
    ControProtocol  *controlProtocol_   = nullptr;
    QElapsedTimer   previousTimerEvent_;
};

