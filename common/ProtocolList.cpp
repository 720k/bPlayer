#include "ProtocolList.h"
#include "Message.h"
#include "Utils.h"
#include "ProtocolBase.h"
#include "ConsoleColors.h"
#include <QDebug>
#include <QDateTime>
#include <QFile>

Q_LOGGING_CATEGORY(catProtocolList, "Protocol")

QString ProtocolList::protocolName_ ="ProtocolList";

ProtocolList::ProtocolList(QObject *parent) : QObject(parent)     {
}

void ProtocolList::printList(const QString &msg, const QString &prefix)  {
    qCDebug(catProtocolList).noquote() << QString("[%1] %2").arg(prefix).arg(msg);
}

void ProtocolList::printList(const QMap<int, ProtocolBase*> &table)  const{
    auto keys = table.keys();
    std::sort(keys.begin(), keys.end());
    for (auto k : keys) {
        auto protocol = table.value(k);
        qCInfo(catProtocolList).noquote() << QString("0x%1 = %2.%3").arg(k, Message::idSize,16,QChar('0')).arg(protocol->name()).arg(protocol->messageName(k));
    }
}

void ProtocolList::printDispatchTable() const {
    printList(protocols_);
}

void ProtocolList::insert(ProtocolBase*protocol, std::initializer_list<int> l)  {
    for (int idx : l)   protocols_.insert(static_cast<intID>(idx), protocol);
}

void ProtocolList::insert(ProtocolBase*protocol, const QPair<int, int> &overrideRange)  {
    auto range = ProtocolBase::isRangeValid(overrideRange) ? overrideRange : protocol->range();
    int min = qMin(range.first, range.second);
    int max = qMax(range.first, range.second);
    for (int idx=min; idx<=max; idx++)  {
        if (protocols_.contains(idx)) {
            qCDebug(catProtocolList).noquote() <<  "Collision " << idx << protocol->name() << "." << protocol->messageName(idx);
        } else {
            protocols_.insert(idx, protocol);
        }
    }
    connect(protocol, &ProtocolBase::sendMessage, this, &ProtocolList::sendMessage);
}

void ProtocolList::dispatch(const Message& msg)  {
    auto id = msg.id();
    auto subprotocol = protocols_.value(id);
    if (subprotocol == nullptr) {
         qCDebug(catProtocolList).noquote() << "Unknown message";
        return;
    }
    qCDebug(catProtocolList).noquote() << "-->" << messageName(id) << " " << Utils::printableRawBA(msg.mid(sizeof(id)));
    subprotocol->dispatchMessage(msg);
}

QString ProtocolList::messageName(int ID) {
    auto subprotocol = protocols_.value(ID);
    return subprotocol->messageName(ID);
}

QString ProtocolList::messageName(const Message &msg) {
    return messageName(msg.id());
}

bool ProtocolList::isMessageValid(const Message &msg) {
    return protocols_.contains(msg.id());
}


void ProtocolList::decodeMessage(const QByteArray &data)   {
    Message msg(data);
    if (!msg.hasMinimumLength())    { qCDebug(catProtocolList).noquote() << "message is too short (less than 4 bytes)"; return; }
    if (!isMessageValid(msg))       { qCDebug(catProtocolList).noquote() << "message is invalid   (wrong ID)"; return; }
    dispatch(msg);
}

void ProtocolList::sendMessage(QByteArray message) {
    auto msg = static_cast<Message>(message);
    qCDebug(catProtocolList).noquote()  << "<==" << messageName(msg) << " " << Utils::printableRawBA(msg.content());
    emit messageReady(message);
}

