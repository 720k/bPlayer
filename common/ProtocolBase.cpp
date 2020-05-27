#include "ProtocolBase.h"

ProtocolBase::ProtocolBase(QObject *parent) : QObject(parent), range_(InvalidRange) {
}

QString ProtocolBase::messageName(int messageID) {
    return names_.value(messageID);
}
