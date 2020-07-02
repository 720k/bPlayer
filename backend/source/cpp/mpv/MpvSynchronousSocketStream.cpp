#include "MpvSynchronousSocketStream.h"
#include "ConsoleColors.h"
#include "StreamProtocol.h"

MpvSynchronousSocketStream::MpvSynchronousSocketStream(QObject *parent) : MpvSynchronousStreamInterface(parent), protocol_(nullptr) {
}

QString MpvSynchronousSocketStream::protocolName() {
    static QString s("bee");
    return s;
}

void MpvSynchronousSocketStream::openReply(int64_t result) {
    terminateSynchrizedWithValue(result);
}

void MpvSynchronousSocketStream::seekReply(int64_t result) {
    terminateSynchrizedWithValue(result);
}

void MpvSynchronousSocketStream::readReply(uint64_t result, QByteArray ba) {
    if (ba.size())    memcpy(mpvReadRequestBuffer_, static_cast<const char *>(ba.data()), mpvReadRequestNBytes_);
    terminateSynchrizedWithValue(result);
}

void MpvSynchronousSocketStream::sizeReply(int64_t size) {
    terminateSynchrizedWithValue(cachedSize_ = size);
}

void MpvSynchronousSocketStream::closeReply() {
}

void MpvSynchronousSocketStream::openSynchronized() {
    CON << cORG <<"[OPEN] "<< cRST;
    protocol_->openRequest();
    // clear cache
    cachedSize_=0;
}

void MpvSynchronousSocketStream::seekSynchronized(uint64_t offset) {
    CON << cORG << "[SEEK] " << cRST << QString("[SEEK] %1 0x%2").arg(offset,8,10,QChar(' ')).arg(offset,8,16,QChar('0')) ;
    protocol_->seekRequest(offset);
}

void MpvSynchronousSocketStream::readSynchronized(char *buf, quint64 nbytes) {
    CON << cORG << "[READ] " << cRST << QString("%1 KiBytes   (%2)").arg(nbytes/1024.0, 0,'f',1).arg(nbytes);
    mpvReadRequestBuffer_ = buf;
    mpvReadRequestNBytes_ = nbytes;
    protocol_->readRequest(nbytes);
}

void MpvSynchronousSocketStream::sizeSynchronized() {
    if (cachedSize_)    terminateSynchrizedWithValue(cachedSize_);
    else {
        CON << cORG << "[SIZE] " << cRST;
        protocol_->sizeRequest();
    }
}

void MpvSynchronousSocketStream::closeSynchronized() {
    CON << cORG << "[CLOSE] " << cRST;
    protocol_->closeRequest();
}
