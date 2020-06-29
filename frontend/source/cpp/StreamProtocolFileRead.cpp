#include "ProtocolList.h"
#include "StreamProtocolFileRead.h"
#include <QDebug>
#include <sys/stat.h>
#ifdef Q_OS_LINUX
    #include <mpv/client.h>
#endif
#ifdef Q_OS_WIN
    #define MPV_ERROR_LOADING_FAILED -13
    #define MPV_ERROR_GENERIC -20
#endif

QMap<int, QString>  StreamProtocolFileRead::IDNames_ {
    {ID::FileOpen,          "FileOpen"},
    {ID::FileSeek,          "FileSeek"},
    {ID::FileRead,          "FileRead"},
    {ID::FileSize,          "FileSize"},
    {ID::FileClose,         "FileClose"},
};


StreamProtocolFileRead::StreamProtocolFileRead(QObject *parent) : ProtocolBase(parent)   {
    setObjectName("StreamProtocolFileRead");
    range_ = makeRange(ID::FileOpen, ID::FileClose);
    names_=IDNames_;
}

void StreamProtocolFileRead::dispatchMessage(const Message &m) {
    auto id = m.id();
    auto content = m.content();
    QDataStream stream(content);
    qint64 value, result;
    switch  (id) {
        case ID::FileOpen :
            result = file_.open(QIODevice::ReadOnly) ? 0 : MPV_ERROR_LOADING_FAILED;
            emit sendMessage( Message::make(ID::FileOpen, result) );
            break;
        case ID::FileSeek : {
            quint64 offset;
            stream >> offset;
            emit sendMessage( Message::make(ID::FileSeek, file_.seek(offset) ? 0 :  MPV_ERROR_GENERIC) );
            }
            break;
        case ID::FileRead : {
            stream >> value;
            auto buf = file_.read(value);
            result = (buf.size() != 0) ? buf.size() : file_.atEnd() ? 0 : -1;
            emit sendMessage( Message::make(ID::FileRead, result, buf) );
            }
            break;
        case ID::FileSize : {
            value = file_.size();
            emit sendMessage( Message::make(ID::FileSize, value));
            }
            break;
        case ID::FileClose:
            if (file_.isOpen()) file_.close();
            break;
        default:
            //print("Unknown message");
            break;
    }
}

void StreamProtocolFileRead::setFileName(const QString &fileName) {
    file_.setFileName(fileName);
}



