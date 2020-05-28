#pragma once

#include <QObject>
#include <QLoggingCategory>

#include "Message.h"
#include "ProtocolBase.h"
Q_DECLARE_LOGGING_CATEGORY(catTestProtocol)

class TestProtocol : public ProtocolBase{
    Q_OBJECT
public:
    enum                        ID {None=0x100,Error,String,Ping,};

                                TestProtocol(QObject *parent);
                                ~TestProtocol() override = default;

    void                        sendString(const QString &s);
    void                        ping(int bounce=1);
signals:
    void                        pingReady(double microSecs);

private:
    void                        dispatchMessage(const Message& m) override;
    QByteArray                  makePing(quint8 bounce, quint8 counter,quint64 time);
    static   QMap<int, QString> IDNames_;
};

