#pragma once

#include <QtCore/QObject>
#include <QFile>
#include <mpv/client.h>
#include <mpv/stream_cb.h>

class MpvController : public QObject
{
    Q_OBJECT
public:
    explicit        MpvController(QObject *parent = nullptr);
                    ~MpvController();

    void            loadFile(const QString& filename, const QString& protocol=QString());
    static  void    controllerWakeup(void *ctx);
    void            registerHandler(const QString &name, void *instance, mpv_stream_cb_open_ro_fn open_fn);
//    void            testFile();
//    void            open(char *uri);
signals:
    void            open(const QString& filename);
    void            replyReady();
    void            mpvEventReady();
public slots:
    void            mediaStart();
    void            quit();
private slots:
    void            handleMpvEvents();
    void            dispatchMpvEvent(mpv_event *event);
private:
    static void     mpvEventReady_cb(void *ctx);
    mpv_handle*     mpvHandle_;
    QByteArray      uri_;
};

