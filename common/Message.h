#pragma once

#include <QtCore/QObject>
#include <QDataStream>
#include <QMap>

using intID = quint32;

class Message : public QByteArray { // EXTENDS ByteArray

public:
    static const int                        idSize = sizeof(intID);
    explicit                                Message(const QByteArray&ba);

    bool                                    hasMinimumLength() const;
    intID                                   id() const;
    QByteArray                              content() const;

    template<typename ...Args> // folding expression
    static QByteArray make(intID id, Args&&... args) {
        QByteArray ba;
        QDataStream data{&ba, QIODevice::WriteOnly};
        data << id;
        ( data << ... << std::forward<Args>(args) );
        return ba;
    }
};

