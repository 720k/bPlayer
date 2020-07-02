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
    {ID::EventVolume,       "EventVolume"},
    {ID::EventVolumeMax,    "EventVolumeMax"},
    {ID::EventMute,         "EventMute"},
    {ID::SetVolume,         "SetVolume"},
    {ID::SetMute,           "SetMute"},

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

void ControlProtocol::setVolume(quint32 volume) {
    emit sendMessage(Message::make(ID::SetVolume, volume) );
}

void ControlProtocol::setMute(bool mute) {
    emit sendMessage(Message::make(ID::SetMute, mute) );
}

void ControlProtocol::eventStateChanged(quint32 state) {
    emit sendMessage(Message::make(ID::EventStateChange, state));
}

void ControlProtocol::eventTimePos(quint32 positionInSeconds) {
    emit sendMessage(Message::make(ID::EventTimePos, positionInSeconds));
}

void ControlProtocol::mediaLength(quint32 length) {
    emit sendMessage(Message::make(ID::MediaLength, length));
}

void ControlProtocol::playbackTime(quint32 time) {
    emit sendMessage(Message::make(ID::PlaybackTime, time));
}

void ControlProtocol::eventVolume(quint32 volume) {
    emit sendMessage(Message::make(ID::EventVolume, volume) );
}

void ControlProtocol::eventVolumeMax(quint32 volume) {
    emit sendMessage(Message::make(ID::EventVolumeMax, volume) );
}

void ControlProtocol::eventMute(bool mute) {
    emit sendMessage(Message::make(ID::EventMute, mute) );
}

void ControlProtocol::dispatchMessage(const Message &msg) {
    auto id = msg.id();   //    auto content = m.content();
     QDataStream stream(msg.content());
    qCDebug(catControlProtocol).noquote() << IDNames_.value(id);
    quint32 value;
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
        case ID::EventStateChange:
            stream >> value;
            emit onEventStateChanged(value);
            break;
        case ID::EventTimePos:
            stream >> value;
            emit onEventTimePos(value);
            break;
        case ID::MediaLength:
            stream >> value;
            emit onMediaLength(value);
            break;
        case ID::PlaybackTime:
            stream >> value;
            emit onPlaybackTime(value);
            break;
        case ID::MediaSeek: {
            quint64 position;
            stream >> position;
            emit onMediaSeek(position);
            break;
        }
        case ID::EventVolume:
            stream >> value;
            emit onEventVolume(value);
            break;
        case ID::EventVolumeMax:
            stream >> value;
            emit onEventVolumeMax(value);
            break;
        case ID::EventMute: {
            bool mute;
            stream >> mute;
            emit onEventMute(mute);
            break;
        }
        case ID::SetVolume:
            stream >> value;
            emit onSetVolume(value);
            break;
        case ID::SetMute:
            bool mute;
            stream >> mute;
            emit onSetMute(mute);
            break;
        default:
            break;
    }
}


