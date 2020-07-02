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
                                    EventVolume,
                                    EventVolumeMax,
                                    EventMute,
                                    SetVolume,
                                    SetMute,
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
    void                        setVolume(quint32 volume);
    void                        setMute(bool mute);
    // backend -> bPlayer
    void                        eventStateChanged(quint32 state);
    void                        eventTimePos(quint32 positionInSeconds);
    void                        mediaLength(quint32 length);
    void                        playbackTime(quint32 time);
    void                        eventVolume(quint32 volume);
    void                        eventVolumeMax(quint32 volume);
    void                        eventMute(bool mute);


signals:
    void                        onMediaStart();
    void                        onMediaStop();
    void                        onMediaPause(bool isPaused);
    void                        onMediaSeek(quint64 position);
    void                        onTok();
    void                        onSetVolume(quint32 volume);
    void                        onSetMute(bool mute);

    void                        onEventStateChanged(quint32 state);
    void                        onEventTimePos(quint32 seconds);
    void                        onMediaLength(quint32 length);
    void                        onPlaybackTime(quint32 time);
    void                        onEventVolume(quint32 volume);
    void                        onEventVolumeMax(quint32 volume);
    void                        onEventMute(bool mute);

private:
    void                        dispatchMessage(const Message& msg) override;
    static  QMap<int, QString>  IDNames_;
};


