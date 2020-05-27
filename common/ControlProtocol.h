#pragma once

#include "ProtocolBase.h"
#include <QObject>

class ControlProtocol : public ProtocolBase {
    Q_OBJECT
public:
    enum                        ID {MediaStart= 0x300, MediaPause, MediaStop, };
                                ControlProtocol(QObject* parent);
public slots:
    void                        mediaStart();
signals:
    void                        onMediaStart();

private:
    void                        dispatchMessage(const Message& msg) override;
    static  QMap<int, QString>  IDNames_;
};

