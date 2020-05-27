#include "Message.h"
#include "ControlProtocol.h"
#include <QDebug>


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

void ControlProtocol::dispatchMessage(const Message &msg) {
    auto id = msg.id();   //    auto content = m.content();
    switch  (id) {
        case ID::MediaStart :
            emit onMediaStart();
            qDebug().noquote() << objectName() << "." << IDNames_.value(id);
            break;
        default:
            qDebug().noquote() << objectName() << "Unknown message";
            break;
    }
}


