#include "ProtocolList.h"
#include "Message.h"
#include "Utils.h"
#include "ProtocolBase.h"
#include "ConsoleColors.h"
#include <QDebug>
#include <QDateTime>
#include <QFile>


QString ProtocolList::protocolName_ ="ProtocolList";

ProtocolList::ProtocolList(QObject *parent) : QObject(parent)     {
}

void ProtocolList::print(const QString &msg, const QString &prefix)  {
    qDebug() << QString("[%1] %2").arg(prefix).arg(msg);
}

void ProtocolList::print(const QMap<int, ProtocolBase*> &table, const QString &title)  const{
    if (!title.isEmpty()) qDebug() << title;
    auto keys = table.keys();
    std::sort(keys.begin(), keys.end());
    for (auto k : keys) {
        auto protocol = table.value(k);
        qDebug() << QString("0x%1 = %2.%3").arg(k, Message::idSize,16,QChar('0')).arg(protocol->name()).arg(protocol->messageName(k));
    }
}

void ProtocolList::printDispatchTable() const {
    print(protocols_, protocolName_+"::DispatchTable");
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
            CON << cRED << "ProtocolList > Error : COLLISION " << cRST << idx << protocol->name() << "." << protocol->messageName(idx) << cEOL;
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
         CON << cRED << "ProtocolList > Error : " << cRST << "[UNHANDLED] message not dispatched";
        return;
    }
    CON << cLV1 << "ProtocolList: --> " << messageName(id) << " " << Utils::printableRawBA(msg.mid(sizeof(id))) << cEOL;
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
    if (!msg.hasMinimumLength())    { print("message is too short (less than 4 bytes)");        return; }
    if (!isMessageValid(msg))       { print("message is invalid   (wrong ID)");                 return; }
    dispatch(msg);
}

void ProtocolList::sendMessage(QByteArray message) {
    auto msg = static_cast<Message>(message);
    CON << cLV1 << "ProtocolList: <== " << messageName(msg) << " " << Utils::printableRawBA(msg.content()) << cEOL;
    emit messageReady(message);
}

