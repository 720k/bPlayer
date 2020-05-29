#include "MpvController.h"
#include "ConsoleColors.h"
#include <QFileInfo>
#include <QDebug>
#include <QJsonDocument>
#include <mpv/qthelper.hpp>


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

    // Enable default bindings, because we're lazy. Normally, a player using
    // mpv as backend would implement its own key bindings.
    mpv_set_option_string(mpvHandle_, "input-default-bindings", "yes");

    // Enable keyboard input on the X11 window. For the messy details, see
    // --input-vo-keyboard on the manpage.
    mpv_set_option_string(mpvHandle_, "input-vo-keyboard", "yes");

//    // Let us receive property change events with MPV_EVENT_PROPERTY_CHANGE if
//    // this property changes.
//    mpv_observe_property(m_mpvHandle, 0, "time-pos", MPV_FORMAT_DOUBLE);

//    mpv_observe_property(m_mpvHandle, 0, "track-list", MPV_FORMAT_NODE);
//    mpv_observe_property(m_mpvHandle, 0, "chapter-list", MPV_FORMAT_NODE);

//    // Request log messages with level "info" or higher.
//    // They are received as MPV_EVENT_LOG_MESSAGE.
//    mpv_request_log_messages(m_mpvHandle, "info");
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
void MpvController::loadFile(const QString &fileName, const QString& protocol)  {
    uri_ = (protocol.isEmpty() ? fileName : QString("%1://%2").arg(protocol).arg(fileName)).toLocal8Bit();
    qDebug() << "URI: " << uri_;
    const char *args[] = {"loadfile", uri_.constData(), nullptr};
    mpv_command_async(mpvHandle_, 0, args);
}

void MpvController::registerHandler(const QString& name, void *instance, mpv_stream_cb_open_ro_fn open_fn) {
    mpv_stream_cb_add_ro(mpvHandle_, name.toLocal8Bit(), instance,  open_fn);
}

void MpvController::mediaStart() {
    const char *args[] = {"loadfile", "bee://media", nullptr};
    mpv_command_async(mpvHandle_, 0, args);
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
            if (strcmp(prop->name, "time-pos") == 0) {
                if (prop->format == MPV_FORMAT_DOUBLE) {
                    //double time = *(double *)prop->data;
                    //                std::stringstream ss;
                    //                ss << "At: " << time;
                    //                statusBar()->showMessage(QString::fromStdString(ss.str()));
                } else if (prop->format == MPV_FORMAT_NONE) {
                    // The property is unavailable, which probably means playback
                    // was stopped.
                    //                statusBar()->showMessage("");
                }
            } else {
                if (strcmp(prop->name, "chapter-list") == 0 || strcmp(prop->name, "track-list") == 0)   {
                    // Dump the properties as JSON for demo purposes.
                    if (prop->format == MPV_FORMAT_NODE) {
                        QVariant v = mpv::qt::node_to_variant((mpv_node *)prop->data);
                        // Abuse JSON support for easily printing the mpv_node contents.
                        QJsonDocument d = QJsonDocument::fromVariant(v);
                        //                append_log("Change property " + QString(prop->name) + ":\n");
                        //                append_log(d.toJson().data());
                    }
                }
            }
            break;
        }
        case MPV_EVENT_VIDEO_RECONFIG: {
            // Retrieve the new video size.
            int64_t w, h;
            if (mpv_get_property(mpvHandle_, "dwidth", MPV_FORMAT_INT64, &w) >= 0 &&
                    mpv_get_property(mpvHandle_, "dheight", MPV_FORMAT_INT64, &h) >= 0 &&
                    w > 0 && h > 0)
            {
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
            //        std::stringstream ss;
            //        ss << "[" << msg->prefix << "] " << msg->level << ": " << msg->text;
            //        append_log(QString::fromStdString(ss.str()));
            qDebug().noquote() << cYLW << "[MPV] " << msg->prefix << " > " << msg->level << ": " << QString(msg->text).trimmed() << cEOL;
            break;
        }
        case MPV_EVENT_SHUTDOWN: {
            mpv_terminate_destroy(mpvHandle_);
            mpvHandle_ = nullptr;
            break;
        }
        default: ; // Ignore uninteresting or unknown events.
    }
}

