#include "Message.h"
#include "ControlProtocol.h"
#include <QDebug>

Q_LOGGING_CATEGORY(catControlProtocol, "Protocol.control")

QMap<int, QString> ControlProtocol::IDNames_ {
    {ID::MediaStart,        "MediaStart"},
    {ID::MediaPause,        "MediaPause"},
    {ID::MediaStop,         "MediaStop"},
};

ControlProtocol::ControlProtocol(QObject* parent) : ProtocolBase(parent) {
    setObjectName("ControlProtocol");
    range_ = makeRange(ID::MediaStart, ID::MediaStop);
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
            }
            break;
        default:
            break;
    }
}


