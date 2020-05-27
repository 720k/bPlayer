#include "Message.h"
#include <QMap>

Message::Message(const QByteArray& ba) : QByteArray(ba)  {
}

bool Message::hasMinimumLength() const {
    return static_cast<intID>(size()) >= idSize;
}

intID Message::id() const {
    intID id;
    QDataStream(*this) >> id;
    return id;
}

QByteArray Message::content() const {
    return mid(idSize);
}



