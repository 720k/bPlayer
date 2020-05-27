#pragma once

#include <cstdint>
#include <mpv/client.h>
#include <mpv/stream_cb.h>
#include <QObject>
#include <QMutex>
#include <QWaitCondition>

class MpvSynchronousStreamInterface : public QObject
{
    Q_OBJECT
public:
    explicit MpvSynchronousStreamInterface(QObject *parent = nullptr);

    static              int     open_cb(void *user_data, char *uri, mpv_stream_cb_info *info);
private:
    static              int64_t seek_cb(void *cookie, const int64_t offset);
    static              int64_t read_cb(void *cookie, char *buf, uint64_t nbytes);
    static              int64_t size_cb(void *cookie);
    static              void     close_cb(void *cookie);
signals:
    int                         openRequest() ;
    int64_t                     seekRequest(const int64_t offset) ;
    int64_t                     readRequest(char *buf, uint64_t nbytes) ;
    int64_t                     sizeRequest() ;
    void                        closeRequest() ;
protected slots:      // 'synchronized' slots are execute in MAIN THREAD context
    virtual void                openSynchronized()   =0;
    virtual void                seekSynchronized(uint64_t offset)    =0;
    virtual void                readSynchronized(char *buf, quint64 nbytes) =0;
    virtual void                sizeSynchronized()  =0;
    virtual void                closeSynchronized() =0;

    void                        terminateSynchrizedWithValue(int64_t retValue) {
         QMutexLocker locker(&replyMutex_);
         retValue_ = retValue;
         readyReply_.wakeAll();
     }
private:
    int                         openAsynchronous();
    int64_t                     seekAsynchronous(const int64_t offset);
    int64_t                     readAsynchronous(char *buf, uint64_t nbytes);
    int64_t                     sizeAsynchronous();
    void                        closeAsynchronous();

    // protect return value with mutex
    QMutex              replyMutex_;
    volatile int64_t    retValue_;  // CRITICAL SECTION
    // use wait condition to force mpv thread to sleep
    // while main thread is going to process the request
    QWaitCondition      readyReply_;

};

