#include "TestProtocol.h"
#include "Utils.h"
#include <QDebug>

QMap<int, QString> TestProtocol::IDNames_ {
        {ID::None,              "None"},
        {ID::Error,             "Error"},
        {ID::String,            "String"},
        {ID::Ping,              "Ping"},
};

TestProtocol::TestProtocol(QObject *parent) : ProtocolBase(parent) {
     setObjectName("TestProtocol");
     range_ = makeRange(TestProtocol::ID::None, TestProtocol::ID::Ping);
     names_ = IDNames_;
}

void TestProtocol::sendString(const QString &str) {
    emit sendMessage(Message::make(ID::String, str) );
}
void TestProtocol::ping(int bounce) {
    emit sendMessage( makePing(bounce, bounce*2-1, Utils::getMicroSeconds()) );
}

void TestProtocol::dispatchMessage(const Message &msg) {
    auto id = msg.id();
    QDataStream stream(msg.content());
    switch  (id) {
        case ID::None :
            qDebug().noquote() << objectName() << "." << IDNames_.value(id);
            break;
        case ID::String : {
                QString s;
                stream >> s;
                qDebug().noquote() << objectName() << "." << IDNames_.value(id) << ":" << s;
                }
            break;
        case ID::Ping : {
                quint8 bounce, counter;
                quint64 time;
                stream >> bounce >> counter >> time;
                if (counter) {
                    emit sendMessage( makePing ( bounce, counter-1, time) ); // ping back
                } else {
                    double microsecs = static_cast<double>(Utils::getMicroSeconds()-time)/(bounce*1000.0);
                    qDebug().noquote() << objectName() << "." << IDNames_.value(id) << QString(": %1 milliSecs ").arg(Utils::printableNumber(microsecs));
                    emit pingReady(microsecs);
                }
            }
        break;
        default:
            qDebug().noquote() << objectName() << "Unknown message";
            break;
    }
}

QByteArray TestProtocol::makePing(quint8 bounce, quint8 counter, quint64 time) {
    return Message::make(ID::Ping, bounce, counter, time);
}
