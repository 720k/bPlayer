#pragma once
#include "MpvSynchronousStreamInterface.h"
class StreamProtocol;

class MpvSynchronousSocketStream : public MpvSynchronousStreamInterface
{
    Q_OBJECT
public:
    MpvSynchronousSocketStream(QObject *parent=nullptr);

    void            setStreamProtocol(StreamProtocol* protocol) { protocol_=protocol; }
    static QString  protocolName();
    void            registerStreamHandler(mpv_handle *ctx);
public slots:
    void            openReply(int64_t result);
    void            seekReply(int64_t result);
    void            readReply(uint64_t result, QByteArray ba);
    void            sizeReply(int64_t size);
    void            closeReply();
        // MpvSynchronousStreamInterface interface
protected slots:
    virtual void    openSynchronized() override;
    virtual void    seekSynchronized(uint64_t offset) override;
    virtual void    readSynchronized(char *buf, quint64 nbytes) override;
    virtual void    sizeSynchronized() override;
    virtual void    closeSynchronized() override;
private:
    StreamProtocol*     protocol_;

    char*               mpvReadRequestBuffer_;          // keep track of request
    uint64_t            mpvReadRequestNBytes_;


};

