#pragma once
#include <QObject>
#include "Message.h"
class ProtocolList;

class ProtocolBase: public QObject   {
    Q_OBJECT
public:

    friend class ProtocolList;

    using                           Range = QPair<int,int>;
    static constexpr Range          InvalidRange {-1,-1};
    static constexpr Range          makeRange(int min, int max) { return qMakePair(min,max); }
    static bool                     isRangeValid(Range range)   { return range != InvalidRange; }

    explicit                        ProtocolBase(QObject* parent);
    virtual                         ~ProtocolBase() = default;

    QString                         name() const    { return objectName(); }
    Range                           range() const   { return range_; }
    QString                         messageName(int messageID);
signals:
    void                            sendMessage(QByteArray msg);
protected:
    virtual void                    dispatchMessage(const Message& msg) =0;

    Range                           range_;
    QMap<int, QString>              names_;
};

