#pragma once
#include "ProtocolBase.h"
#include <stdio.h>
#include <QFile>

class StreamProtocolFileRead : public ProtocolBase
{
    Q_OBJECT
public:
                enum            ID {FileOpen=0x200, FileSeek, FileRead, FileSize,FileClose,};

                                StreamProtocolFileRead(QObject *parent);

    void                        setFileName(const QString& fileName);
    QString                     fileName() const    { return file_.fileName(); }

private:
    void                        dispatchMessage(const Message &m) override;
    static  QMap<int, QString>  IDNames_;
    QFile               file_;
};

