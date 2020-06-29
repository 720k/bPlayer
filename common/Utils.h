#pragma once

#include <QByteArray>
#include <QString>

namespace Utils {
    QString         printableBA(const QByteArray &ba, const QString&prefix=QString());
    QString         printableRawBA(const QByteArray &ba);
    quint64         getMicroSeconds();
    QString         printableNumber(double no);
    QString         threadName(QString text);
    qint64          getProcessID(const QString& name);
    QString         portNameFromProcess(const QString &portName, const QString &ProcessName);
    QString         formatTime(quint64 seconds);

    template<class C>
    void            destroy(C& instance) {
                        if (instance) {
                            delete instance;
                            instance = nullptr;
                        }
                    }
#ifdef  Q_OS_WIN
    void            PrintLastErrorString();
#endif
};
