#pragma once
#include "ProtocolBase.h"

class MpvSynchronousSocketStream;
class StreamProtocol : public ProtocolBase
{
    Q_OBJECT
public:
                        enum ID {FileOpen=0x200, FileSeek, FileRead, FileSize,FileClose,};
                                StreamProtocol(QObject *parent, MpvSynchronousSocketStream *stm);
        void                    open(bool result);
        void                    seek(bool result);
        void                    read(qint64 result, QByteArray ba);
        void                    size();
        void                    close();
        void                    openRequest();
        void                    readRequest(quint64 nbytes);
        void                    seekRequest(qint64 offset);
        void                    sizeRequest();
        void                    closeRequest();

private:
    void                        dispatchMessage(const Message &m) override;
    static  QMap<int, QString>  IDNames_;
    MpvSynchronousSocketStream* mpvSocketStreamer_;
};

