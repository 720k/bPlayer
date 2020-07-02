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
                                    MediaLength,
                                    PlaybackTime,
                                    MediaSeek,
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
    void                        mediaSeek(quint64 position);
    void                        tik();
    // backend -> bPlayer
    void                        eventStateChanged(quint32 state);
    void                        eventTimePos(quint32 positionInSeconds);
    void                        mediaLength(quint32 length);
    void                        playbackTime(quint32 time);

signals:
    void                        onMediaStart();
    void                        onMediaStop();
    void                        onMediaPause(bool isPaused);
    void                        onTok();
    void                        onMediaSeek(quint64 position);

    void                        onEventStateChanged(quint32 state);
    void                        onEventTimePos(quint32 seconds);
    void                        onMediaLength(quint32 length);
    void                        onPlaybackTime(quint32 time);

private:
    void                        dispatchMessage(const Message& msg) override;
    static  QMap<int, QString>  IDNames_;
};


