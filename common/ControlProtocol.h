#pragma once

#include "ProtocolBase.h"
#include <QObject>
#include <QLoggingCategory>
Q_DECLARE_LOGGING_CATEGORY(catControlProtocol)

class ControlProtocol : public ProtocolBase {
    Q_OBJECT
public:
    enum                        ID {
                                    MediaStart= 0x300, MediaPause, MediaStop,
                                    Tik,Tok,
                                    EventStateChange,
                                    EventTimePos,
                                    Last};

    enum                        EventState {
                                    PlaybackRestart, Seek, Pause, Unpause, StartFile,EndFile,FileLoaded,Idle
                                };

                                ControlProtocol(QObject* parent=nullptr);
public slots:
    // bPlayer -> backend
    void                        mediaStart();
    void                        mediaStop();
    void                        mediaPause(bool isPaused);
    void                        tik();
    // backend -> bPlayer
    void                        eventStateChanged(quint64 state);
    void                        eventTimePos(quint64 positionInSeconds);

signals:
    void                        onMediaStart();
    void                        onMediaStop();
    void                        onMediaPause(bool isPaused);
    void                        onTok();

    void                        onEventStateChanged(quint64 state);
    void                        onEventTimePos(quint64 seconds);


private:
    void                        dispatchMessage(const Message& msg) override;
    static  QMap<int, QString>  IDNames_;
};


