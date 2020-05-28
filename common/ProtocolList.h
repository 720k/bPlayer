#pragma once

#include "Message.h"
#include "ProtocolBase.h"
#include <QtCore/QObject>
#include <QDebug>
#include <QMap>
#include <QLoggingCategory>
class ProtocolList : public QObject
{
    Q_OBJECT

public:
    explicit                    ProtocolList(QObject *parent = nullptr);

    void                        insert(ProtocolBase *protocol, const ProtocolBase::Range& overrideRange = ProtocolBase::InvalidRange );
    void                        insert(ProtocolBase *protocol, std::initializer_list<int> l );
    void                        printList(const QString& msg, const QString& prefix = protocolName_);
    void                        printList(const QMap<int, ProtocolBase*>& table) const;
    void                        printDispatchTable() const;
signals:
    void                        messageReady(QByteArray message);
    void                        printMessage(const QString& msg);
public slots:
    void                        decodeMessage(const QByteArray& data);
    void                        sendMessage(QByteArray message);
private:
    void                        dispatch(const Message &msg);
    QString                     messageName(int ID);
    QString                     messageName(const Message& msg);
    bool                        isMessageValid(const Message& msg);

    static QString              protocolName_;
    QMap<int, ProtocolBase*>    protocols_;
};
Q_DECLARE_LOGGING_CATEGORY(catProtocolList)
