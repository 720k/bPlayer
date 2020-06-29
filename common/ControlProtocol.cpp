#include "Message.h"
#include "ControlProtocol.h"
#include <QDebug>

Q_LOGGING_CATEGORY(catControlProtocol, "Protocol.control")

QMap<int, QString> ControlProtocol::IDNames_ {
    {ID::MediaStart,        "MediaStart"},
    {ID::MediaPause,        "MediaPause"},
    {ID::MediaStop,         "MediaStop"},
    {ID::Tik,               "Tik"},
    {ID::Tok,               "Tok"},
    {ID::EventStateChange,  "EventStateChange"},
    {ID::EventTimePos,      "EventTimePos"},
    {ID::MediaLength,       "MediaLength"},
    {ID::PlaybackTime,      "PlaybackTime"},
    {ID::MediaSeek,         "MediaSeek"},
    {ID::Last,              "Last"},
};

ControlProtocol::ControlProtocol(QObject* parent) : ProtocolBase(parent) {
    setObjectName("ControlProtocol");
    range_ = makeRange(ID::MediaStart, ID::Last);
    names_ = IDNames_;
}

void ControlProtocol::mediaStart()  {
    emit sendMessage(Message::make(ID::MediaStart) );
}

void ControlProtocol::mediaStop() {
    emit sendMessage(Message::make(ID::MediaStop));
}

void ControlProtocol::mediaPause(bool isPaused) {
    emit sendMessage(Message::make(ID::MediaPause, isPaused ));
}

void ControlProtocol::mediaSeek(quint64 position) {
    emit sendMessage(Message::make(ID::MediaSeek, position));
}

void ControlProtocol::tik() {
    emit sendMessage(Message::make(ID::Tik));
}

void ControlProtocol::eventStateChanged(quint64 state) {
    emit sendMessage(Message::make(ID::EventStateChange, state));
}

void ControlProtocol::eventTimePos(quint64 positionInSeconds) {
    emit sendMessage(Message::make(ID::EventTimePos, positionInSeconds));
}

void ControlProtocol::mediaLength(quint64 length) {
    emit sendMessage(Message::make(ID::MediaLength, length));
}

void ControlProtocol::playbackTime(quint64 time) {
    emit sendMessage(Message::make(ID::PlaybackTime, time));
}



void ControlProtocol::dispatchMessage(const Message &msg) {
    auto id = msg.id();   //    auto content = m.content();
     QDataStream stream(msg.content());
    qCDebug(catControlProtocol).noquote() << IDNames_.value(id);
    switch  (id) {
        case ID::MediaStart :
            emit onMediaStart();
            break;
        case ID::MediaStop :
            emit onMediaStop();
            break;
        case ID::MediaPause : {
            bool isPaused;
            stream >> isPaused;
            emit onMediaPause(isPaused);
            break;
            }
        case ID::Tik:
            sendMessage(Message::make(ID::Tok));
            break;
        case ID::Tok:
            emit onTok();
            break;
        case ID::EventStateChange: {
            quint64 state;
            stream >> state;
            emit onEventStateChanged(state);
            break;
            }
        case ID::EventTimePos: {
            quint64 posInSecods;
            stream >> posInSecods;
            emit onEventTimePos(posInSecods);
            break;
            }
        case ID::MediaLength: {
            quint64 length;
            stream >> length;
            emit onMediaLength(length);
            break;
            }
        case ID::PlaybackTime: {
            quint64 time;
            stream >> time;
            emit onPlaybackTime(time);
            break;
        }
        case ID::MediaSeek: {
            quint64 position;
            stream >> position;
            emit onMediaSeek(position);
            break;
        }
        default:
            break;
    }
}


