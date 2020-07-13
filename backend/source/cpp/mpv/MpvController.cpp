#include "MpvController.h"
#include "ConsoleColors.h"
#include "ControlProtocol.h"
#include <QFileInfo>
#include <QDebug>
#include <QJsonDocument>
#include <mpv/qthelper.hpp>

Q_LOGGING_CATEGORY(catMpv, "MpvCtrl")



MpvController::MpvController(QObject *parent) : QObject(parent)   {
    // Qt sets the locale in the QApplication constructor, but libmpv requires
    // the LC_NUMERIC category to be set to "C", so change it back.
    std::setlocale(LC_NUMERIC, "C");

    // CALLBACK -> MAIN thread event loop
    connect(this, &MpvController::mpvEventReady,   this, &MpvController::handleMpvEvents, Qt::QueuedConnection);

    // qt queued signals (events) only with known types
    qRegisterMetaType<int64_t>("int64_t");
    qRegisterMetaType<uint64_t>("uint64_t");


    mpvHandle_ = mpv_create();
    if (!mpvHandle_)   throw std::runtime_error("can't create mpv instance");
    mpv_set_option_string(mpvHandle_, "osc", "yes");
    mpv_set_option_string(mpvHandle_, "idle", "yes");
    mpv_set_option_string(mpvHandle_, "border", "no");
    mpv_set_option_string(mpvHandle_, "input-ipc-server", "/tmp/mpvsocket");

    // Enable default bindings, because we're lazy. Normally, a player using
    // mpv as backend would implement its own key bindings.
    mpv_set_option_string(mpvHandle_, "input-default-bindings", "yes");

    // Enable keyboard input on the X11 window. For the messy details, see
    // --input-vo-keyboard on the manpage.
    mpv_set_option_string(mpvHandle_, "input-vo-keyboard", "yes");

    //cache 5 secs
    //#hardcoded: cache can be force via 'profile'
    mpv_set_option_string(mpvHandle_, "cache", "yes");
    mpv_set_option_string(mpvHandle_, "cache-secs", "5");
    mpv_set_option_string(mpvHandle_, "demuxer-max-bytes", "16777216"); // 16 MiB
    mpv_set_option_string(mpvHandle_, "demuxer-readahead-secs", "5");
    mpv_set_option_string(mpvHandle_, "cache-pause-initial", "yes");
    mpv_set_option_string(mpvHandle_, "cache-pause", "yes");
    mpv_set_option_string(mpvHandle_, "cache-pause-wait", "5");




    // Let us receive property change events with MPV_EVENT_PROPERTY_CHANGE if
    // this property changes.
    mpv_observe_property(mpvHandle_, 0, "playback-time", MPV_FORMAT_DOUBLE);
    mpv_observe_property(mpvHandle_, 0, "volume",        MPV_FORMAT_DOUBLE);
    mpv_observe_property(mpvHandle_, 0, "mute",          MPV_FORMAT_FLAG);
//    mpv_observe_property(mpvHandle_, 0, "time-pos", MPV_FORMAT_DOUBLE);
//    mpv_observe_property(mpvHandle_, 0, "duration", MPV_FORMAT_DOUBLE);

//    mpv_observe_property(m_mpvHandle, 0, "track-list", MPV_FORMAT_NODE);
//    mpv_observe_property(m_mpvHandle, 0, "chapter-list", MPV_FORMAT_NODE);

    // Request log messages with level "info" or higher.
    // They are received as MPV_EVENT_LOG_MESSAGE.
    mpv_request_log_messages(mpvHandle_, "info"); // info

    // From this point on, the wakeup function will be called. The callback
    // can come from any thread, so we use the QueuedConnection mechanism to
    // relay the wakeup in a thread-safe way.
    mpv_set_wakeup_callback(mpvHandle_, mpvEventReady_cb, this);
    if (mpv_initialize(mpvHandle_) < 0)    throw std::runtime_error("mpv failed to initialize");
}

MpvController::~MpvController()   {
    quit();
}


// STATIC METHODS
void MpvController::mpvEventReady_cb(void *ctx)     {
    emit static_cast<MpvController*>(ctx)->mpvEventReady();
    // This callback is invoked from any mpv thread (but possibly also
    // recursively from a thread that is calling the mpv API). Just notify
    // the Qt GUI thread to wake up (so that it can process events with
    // mpv_wait_event()), and return as quickly as possible.
}


// METHODS
void MpvController::registerHandler(const QString& name, void *instance, mpv_stream_cb_open_ro_fn open_fn) {
    mpv_stream_cb_add_ro(mpvHandle_, name.toLocal8Bit(), instance,  open_fn);
}

void MpvController::loadFile(const QString &fileName, const QString& protocol)  {
    uri_ = (protocol.isEmpty() ? fileName : QString("%1://%2").arg(protocol).arg(fileName)).toLocal8Bit();
    qCDebug(catMpv) << "URI: " << uri_;
    const char *args[] = {"loadfile", uri_.constData(), nullptr};
    mpv_command_async(mpvHandle_, 0, args);
}

void MpvController::mediaStart() {
    const char *args[] = {"loadfile", "bee://media", nullptr};
    mpv_command_async(mpvHandle_, ReplyID::Start, args);
}

void MpvController::mediaStop() {
    const char *args[] = {"stop", nullptr};
    mpv_command_async(mpvHandle_, ReplyID::Stop, args);
}

void MpvController::mediaPause(bool isPaused) {
    isPaused_ = isPaused ? 1 : 0;
    mpv_set_property_async(mpvHandle_,ReplyID::Pause, "pause",MPV_FORMAT_FLAG, &isPaused_);
}

void MpvController::mediaSeek(int position) {
    const QByteArray pos = QString::number(position).toUtf8();
    const char *args[] = {"seek", pos.constData(), "absolute", NULL};
    mpv_command_async(mpvHandle_, ReplyID::Seek, args);
}

void MpvController::setVolume(quint32 volume) {
    volume_ = static_cast<double>(volume);
    mpv_set_property_async(mpvHandle_, ReplyID::SetVolume, "volume",MPV_FORMAT_DOUBLE, &volume_);
}

void MpvController::setMute(bool mute) {
    isMute_ = mute ? 1 : 0;
    mpv_set_property_async(mpvHandle_, ReplyID::SetMute, "mute",MPV_FORMAT_FLAG, &isMute_);
}

void MpvController::quit() {
    mpv_terminate_destroy(mpvHandle_);
    mpvHandle_ = nullptr;
}



void MpvController::handleMpvEvents()  {
    while (mpvHandle_) {           // Process all events, until the event queue is empty.
        mpv_event *event = mpv_wait_event(mpvHandle_, 0);
        if (event->event_id == MPV_EVENT_NONE)      break;
        dispatchMpvEvent(event);
    }
}

void MpvController::dispatchMpvEvent(mpv_event *event)     {
    switch (event->event_id) {
        case MPV_EVENT_PROPERTY_CHANGE: {
            mpv_event_property *prop = (mpv_event_property *)event->data;
            if(QString(prop->name) == "playback-time")  {// playback-time is like time-pos but works for streaming media
                if(prop->format == MPV_FORMAT_DOUBLE) {
                    quint32 curr =(quint32) *(double*)prop->data;
                    // 1/3 of second is enough for u/i time notification
                    if (!previousTimerEvent_.isValid() || previousTimerEvent_.hasExpired(300) ) {
                        previousTimerEvent_.restart();
                        emit eventPlaybackTime(curr);
                    }
                }
            }
            else if(QString(prop->name) == "volume")  {
                if(prop->format == MPV_FORMAT_DOUBLE) {
                    quint32 volume =(quint32) *(double*)prop->data;
                    emit eventVolume(volume);
                }
            }
            else if(QString(prop->name) == "mute")  {
                if(prop->format == MPV_FORMAT_FLAG) {
                    bool mute = *(int64_t*)prop->data;
                    emit eventMute(mute);
                }
            }

//            if (strcmp(prop->name, "time-pos") == 0) {
//                if (prop->format == MPV_FORMAT_DOUBLE) {
//                    double time = *(double *)prop->data;
//                    emit eventTimePos(time);
//                } else if (prop->format == MPV_FORMAT_NONE) {
//                    // The property is unavailable, which probably means playback
//                    // was stopped.
//                }
//            } else {
//                if (strcmp(prop->name, "chapter-list") == 0 || strcmp(prop->name, "track-list") == 0)   {
//                    // Dump the properties as JSON for demo purposes.
//                    if (prop->format == MPV_FORMAT_NODE) {
//                        QVariant v = mpv::qt::node_to_variant((mpv_node *)prop->data);
//                        // Abuse JSON support for easily printing the mpv_node contents.
//                        QJsonDocument d = QJsonDocument::fromVariant(v);
//                        //                append_log("Change property " + QString(prop->name) + ":\n");
//                        //                append_log(d.toJson().data());
//                    }
//                }

            break;
        }
        case MPV_EVENT_VIDEO_RECONFIG: {
            // Retrieve the new video size.
            int64_t w, h;
            if (mpv_get_property(mpvHandle_, "dwidth", MPV_FORMAT_INT64, &w) >= 0
            && mpv_get_property(mpvHandle_, "dheight", MPV_FORMAT_INT64, &h) >= 0
            && w > 0 && h > 0) {
                // Note that the MPV_EVENT_VIDEO_RECONFIG event doesn't necessarily
                // imply a resize, and you should check yourself if the video
                // dimensions really changed.
                // mpv itself will scale/letter box the video to the container size
                // if the video doesn't fit.
                //            std::stringstream ss;
                //            ss << "Reconfig: " << w << " " << h;
                //            statusBar()->showMessage(QString::fromStdString(ss.str()));
            }
            break;
        }
        case MPV_EVENT_LOG_MESSAGE: {
            struct mpv_event_log_message *msg = static_cast<struct mpv_event_log_message *>(event->data);
            qCDebug(catMpv).noquote() << cYLW << msg->prefix << " > " << msg->level << ": " << QString(msg->text).trimmed() << cEOL;
            break;
        }
        case MPV_EVENT_SHUTDOWN: {
            mpv_terminate_destroy(mpvHandle_);
            mpvHandle_ = nullptr;
            break;
        }
        case MPV_EVENT_SEEK:    emit eventStateChanged(ControlProtocol::EventState::Seek);   break;
        case MPV_EVENT_PAUSE:   emit eventStateChanged(ControlProtocol::EventState::Pause);  break;
        case MPV_EVENT_UNPAUSE: emit eventStateChanged(ControlProtocol::EventState::Unpause);    break;
        case MPV_EVENT_PLAYBACK_RESTART: emit eventStateChanged(ControlProtocol::EventState::PlaybackRestart); break;
        case MPV_EVENT_START_FILE: emit eventStateChanged(ControlProtocol::EventState::StartFile); break;
        case MPV_EVENT_END_FILE: emit eventStateChanged(ControlProtocol::EventState::EndFile); break;
        case MPV_EVENT_IDLE: emit eventStateChanged(ControlProtocol::EventState::Idle); break;

        case MPV_EVENT_FILE_LOADED: {
            double value;
            mpv_get_property(mpvHandle_, "duration", MPV_FORMAT_DOUBLE, &value);
            emit eventMediaLength((quint32)value);
            mpv_get_property(mpvHandle_, "volume-max", MPV_FORMAT_DOUBLE, &value);
            emit eventVolumeMax((quint32)value);
            mpv_get_property(mpvHandle_, "volume", MPV_FORMAT_DOUBLE, &value);
            emit eventVolume((quint32)value);
            break;
        }
        default: ; // Ignore uninteresting or unknown events.
    }
}

