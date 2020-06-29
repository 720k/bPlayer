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
    void                        eventStateChanged(quint64 state);
    void                        eventTimePos(quint64 positionInSeconds);
    void                        mediaLength(quint64 length);
    void                        playbackTime(quint64 time);

signals:
    void                        onMediaStart();
    void                        onMediaStop();
    void                        onMediaPause(bool isPaused);
    void                        onTok();
    void                        onMediaSeek(quint64 position);

    void                        onEventStateChanged(quint64 state);
    void                        onEventTimePos(quint64 seconds);
    void                        onMediaLength(quint64 length);
    void                        onPlaybackTime(quint64 time);

private:
    void                        dispatchMessage(const Message& msg) override;
    static  QMap<int, QString>  IDNames_;
};


