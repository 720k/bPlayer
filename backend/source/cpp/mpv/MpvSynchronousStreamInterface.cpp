#include "MpvSynchronousStreamInterface.h"

MpvSynchronousStreamInterface::MpvSynchronousStreamInterface(QObject *parent) : QObject(parent) {
    connect(this, &MpvSynchronousStreamInterface::openRequest,     this,&MpvSynchronousStreamInterface::openSynchronized, Qt::QueuedConnection);
    connect(this, &MpvSynchronousStreamInterface::seekRequest,     this,&MpvSynchronousStreamInterface::seekSynchronized, Qt::QueuedConnection);
    connect(this, &MpvSynchronousStreamInterface::sizeRequest,     this,&MpvSynchronousStreamInterface::sizeSynchronized, Qt::QueuedConnection);
    connect(this, &MpvSynchronousStreamInterface::readRequest,     this,&MpvSynchronousStreamInterface::readSynchronized, Qt::QueuedConnection);
    connect(this, &MpvSynchronousStreamInterface::closeRequest,    this,&MpvSynchronousStreamInterface::closeSynchronized, Qt::QueuedConnection);
}

int MpvSynchronousStreamInterface::open_cb(void *user_data, char *uri, mpv_stream_cb_info *info) {
    Q_UNUSED(uri)
    info->cookie = user_data;
    info->size_fn = size_cb;
    info->read_fn = read_cb;
    info->seek_fn = seek_cb;
    info->close_fn = close_cb;
    auto streamer = static_cast<MpvSynchronousStreamInterface*>(user_data);
    return streamer->openAsynchronous();
}

int64_t MpvSynchronousStreamInterface::seek_cb(void *cookie, const int64_t offset) {
    auto streamer = static_cast<MpvSynchronousStreamInterface*>(cookie);
    return streamer->seekAsynchronous(offset);
}

int64_t MpvSynchronousStreamInterface::read_cb(void *cookie, char *buf, uint64_t nbytes) {
    auto streamer = static_cast<MpvSynchronousStreamInterface*>(cookie);
    return streamer->readAsynchronous(buf, nbytes);
}

int64_t MpvSynchronousStreamInterface::size_cb(void *cookie) {
    auto streamer = static_cast<MpvSynchronousStreamInterface*>(cookie);
    return streamer->sizeAsynchronous();
}

void MpvSynchronousStreamInterface::close_cb(void *cookie) {
    auto streamer = static_cast<MpvSynchronousStreamInterface*>(cookie);
    streamer->closeAsynchronous();
}

int MpvSynchronousStreamInterface::openAsynchronous()   {
    int64_t result;
    {
        QMutexLocker locker(&replyMutex_);
        emit openRequest();
        readyReply_.wait(&replyMutex_);
        result = retValue_;
    }
    return result;
}

int64_t MpvSynchronousStreamInterface::seekAsynchronous(const int64_t offset) {
    int64_t result;
    {
        QMutexLocker locker(&replyMutex_);
        emit seekRequest(offset);
        readyReply_.wait(&replyMutex_);
        result = retValue_;
    }
    return result;

}

int64_t MpvSynchronousStreamInterface::readAsynchronous(char *buf, uint64_t nbytes) {
    int64_t result;
    {
        QMutexLocker locker(&replyMutex_);
        emit readRequest(buf, nbytes);
        readyReply_.wait(&replyMutex_);
        result = retValue_;
    }
    return result;
}

int64_t MpvSynchronousStreamInterface::sizeAsynchronous() {
    int64_t result;
    {
        QMutexLocker locker(&replyMutex_);
        emit sizeRequest();
        readyReply_.wait(&replyMutex_);
        result = retValue_;
    }
    return result;
}

void MpvSynchronousStreamInterface::closeAsynchronous() {
    {
        //QMutexLocker locker(&replyMutex_);
        emit closeRequest();
        //readyReply_.wait(&replyMutex_);
    }
}
