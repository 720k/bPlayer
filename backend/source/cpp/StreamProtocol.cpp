#include "StreamProtocol.h"
#include "MpvSynchronousSocketStream.h"
#include <QDebug>

QMap<int, QString>  StreamProtocol::IDNames_ {
    {ID::FileOpen,          "FileOpen"},
    {ID::FileSeek,          "FileSeek"},
    {ID::FileRead,          "FileRead"},
    {ID::FileSize,          "FileSize"},
    {ID::FileClose,         "FileClose"},
};

StreamProtocol::StreamProtocol(MpvSynchronousSocketStream *stm, QObject *parent) : ProtocolBase(parent), mpvSocketStreamer_(stm) {
     setObjectName("StreamProtocol");
     range_ = makeRange(ID::FileOpen, ID::FileClose);
     names_ = IDNames_;
     mpvSocketStreamer_->setStreamProtocol(this);
}

void StreamProtocol::dispatchMessage(const Message &m)  {
    auto id = m.id();
    auto content = m.content();
    QDataStream stream(content);
    qint64 value;
    switch  (id) {
        case ID::FileOpen:
            stream >> value;
            mpvSocketStreamer_->openReply(value);
            break;
        case ID::FileSeek :
            stream >> value;
            mpvSocketStreamer_->seekReply(value);
            break;
        case ID::FileRead : {
            QByteArray data;
            stream >> value >> data;
            mpvSocketStreamer_->readReply(value, data);
            }
            break;
        case ID::FileSize :
            stream >> value;
            mpvSocketStreamer_->sizeReply(value);
            break;
        case ID::FileClose:
            qDebug() << "NOT YET IMPLEMENTED" << IDNames_.value(id) ;
            break;
        default:
            qDebug() << "Unknown message";
            break;
    }
}

void StreamProtocol::openRequest() {
    emit sendMessage(Message::make(ID::FileOpen) );
}

void StreamProtocol::readRequest(quint64 nbytes) {
    emit sendMessage(Message::make(ID::FileRead, nbytes) );
}

void StreamProtocol::seekRequest(qint64 offset)   {
    emit sendMessage(Message::make(ID::FileSeek, offset) );
}

void StreamProtocol::sizeRequest()  {
    emit sendMessage( Message::make(ID::FileSize) );
}

void StreamProtocol::closeRequest() {
    emit sendMessage(Message::make(ID::FileClose) );
}

