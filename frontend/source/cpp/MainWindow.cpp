
#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "AbstractSocket.h"
#include "ConsoleColors.h"
#include "ProtocolList.h"
#include "TestProtocol.h"
#include "ControlProtocol.h"
#include "StreamProtocolFileRead.h"
#include "Utils.h"
#include "TestWindow.h"
#include "BPlayer.h"

#include <QLocalServer>
#include <QFileDialog>
#include <QStandardPaths>
#include <QFontDatabase>
#include <QTimer>
#include <chrono>
#include <QTime>
Q_LOGGING_CATEGORY(catApp, "App")

using namespace std::literals::chrono_literals;

QMap<MainWindow::ConnectionState, QString>  MainWindow::connectionStateName_ = {
    {ConnectionState::Offline,       "Offline"},
    {ConnectionState::Connecting,    "Connecting"},
//    {ConnectionState::ConnectingWait,"ConnectingWait"},
    {ConnectionState::Online,        "Online"},
    {ConnectionState::Ready,         "Ready"},
};


void MainWindow::init() {
    connectionTypeChanged();
    // DATA conduit
    dataProtocols_.insert(testProtocol_ =    new TestProtocol(&dataProtocols_));
    dataProtocols_.insert(controlProtocol_ = new ControlProtocol(&dataProtocols_));
    dataProtocols_.printDispatchTable();

    // STREAM conduit
    streamProtocols_.insert(streamProtocol_ =  new StreamProtocolFileRead(&streamProtocols_));
    streamProtocols_.printDispatchTable();


    connect(controlProtocol_,   &ControlProtocol::onTok,        this,&MainWindow::onTok);
    connect(controlProtocol_,   &ControlProtocol::onEventStateChanged, this, &MainWindow::onEventStateChanged);
    connect(controlProtocol_,   &ControlProtocol::onMediaLength,    this, &MainWindow::onMediaLength);
    connect(controlProtocol_,   &ControlProtocol::onPlaybackTime,    this, &MainWindow::onPlaybackTime);

    int id = QFontDatabase::addApplicationFont(":/font/font-roboto-nerd");
    if (id < 0) {
        qCWarning(catApp).noquote() << "Can't Add font Roboto nerd";
    } else {
        QString family = QFontDatabase::applicationFontFamilies(id).at(0);
        QFont nerd(family);
        qApp->setFont(nerd);
    }
    testWindow_ = new TestWindow(this);
}

bool MainWindow::isLocalSocket() const {
#ifdef Q_OS_LINUX
    return true;
#else
    return false;
#endif
}


MainWindow::MainWindow(QWidget *parent)  : QMainWindow(parent), ui(new Ui::MainWindow){
    ui->setupUi(this);
    updateWidgetStatus();
    init();

    QTimer::singleShot(2s,this,&MainWindow::tryConnecting);
}

MainWindow::~MainWindow() {
    delete ui;
}


void MainWindow::updateWidgetStatus() {
    bool online = isOnline();
    // MAIN TAB
    QColor c{ static_cast<QRgb>(connectionState_) };
    ui->ledLabel->setStyleSheet(QString("color:%1").arg(c.name()));
    ui->playButton->setText(mediaState_ == MediaState::Playing ? "":"");
    ui->stopButton->setEnabled(online && mediaState_ != MediaState::None);
    ui->muteButton->setEnabled(online && mediaState_ != MediaState::None);
    ui->prefsButton->setEnabled(online);
    ui->enlargeButton->setEnabled(online); // #TODO diseable/enable container panel
    ui->timeSlider->setEnabled(online && mediaState_ != MediaState::None);
    ui->timeLabel->setEnabled(online && mediaState_ != MediaState::None);
    ui->timePosLabel->setEnabled(online && mediaState_ != MediaState::None);
    ui->volumeSlider->setEnabled(online && mediaState_ != MediaState::None);
}


void MainWindow::tryConnecting() {
    if (dataSocket_->isConnected() && streamSocket_->isConnected()) return;
    setConnectionState(ConnectionState::Connecting);
    if (isLocalSocket())    {
        QString processName = "socket-bridge";
        if (dataSocket_->isUnconnected())   dataSocket_->connectToServer(Utils::portNameFromProcess(QString("frontend/%1").arg(BPlayer::dataPortName), processName) );
        if (streamSocket_->isUnconnected()) streamSocket_->connectToServer(Utils::portNameFromProcess(QString("frontend/%1").arg(BPlayer::streamPortName), processName) );
    } else {
        if (dataSocket_->isUnconnected())   dataSocket_->connectToServer(BPlayer::locolhost, BPlayer::dataPortNumber);
        if (streamSocket_->isUnconnected()) streamSocket_->connectToServer(BPlayer::locolhost, BPlayer::streamPortNumber);
    }
}

void MainWindow::startTicking() {
    tickingTimerID_ = startTimer(1s);
}

void MainWindow::stopTicking() {
    if (tickingTimerID_ != invalidTimerID) {
        killTimer(tickingTimerID_);
        tickingTimerID_ = invalidTimerID;
    }
}


void MainWindow::timerEvent(QTimerEvent *event) {   Q_UNUSED(event)
    controlProtocol_->tik();
}

void MainWindow::connectionTypeChanged() {
    if (dataSocket_) {
        dataSocket_->disconnect(); // no strings attached
        dataSocket_->deleteLater();
    }
    if (streamSocket_) {
        streamSocket_->disconnect(); // no strings attached
        streamSocket_->deleteLater();
    }

    if (isLocalSocket())    {
        dataSocket_ = new LocalSocket();
        streamSocket_ = new LocalSocket();
    } else {
        dataSocket_ = new TcpSocket();
        streamSocket_ = new TcpSocket();
    }

    // data socket <-> data Protocols
    connect(dataSocket_,&AbstractSocket::stateChanged,    this, &MainWindow::socketStateChanged);
    connect(dataSocket_,&AbstractSocket::messageReady,    &dataProtocols_, &ProtocolList::decodeMessage);
    connect(&dataProtocols_,&ProtocolList::messageReady,     dataSocket_, &AbstractSocket::writeMessage);

    // stream socket <-> stream Protocols
    connect(streamSocket_,&AbstractSocket::stateChanged,    this, &MainWindow::socketStateChanged);
    connect(streamSocket_,&AbstractSocket::messageReady,    &streamProtocols_, &ProtocolList::decodeMessage);
    connect(&streamProtocols_,&ProtocolList::messageReady,     streamSocket_, &AbstractSocket::writeMessage);
}

void MainWindow::on_action_Open_triggered() {
    auto fileName = QFileDialog::getOpenFileName(this,
                            tr("Open File"), QStandardPaths::standardLocations(QStandardPaths::MoviesLocation).at(0),
                            tr("Video (*.mov *.mp4 *.avi *.mpeg *.mpg);Audio (*.m4a *.mp3 *.wav);Any file (*.*)"));
    if (fileName.isNull()) return;
    streamProtocol_->setFileName(fileName_ = fileName);
    controlProtocol_->mediaStart();
}

void MainWindow::checkState() {
    if (connectionState_ == ConnectionState::Connecting) {
        if (dataSocket_->isConnected() && streamSocket_->isConnected()) setConnectionState(ConnectionState::Online);
        else                                                            QTimer::singleShot(3s, this, &MainWindow::tryConnecting);
    }
    if (connectionState_ == ConnectionState::Online) {
        if (dataSocket_->isUnconnected() || streamSocket_->isUnconnected()) {
            setConnectionState(ConnectionState::Connecting);
            QTimer::singleShot(3s, this, &MainWindow::tryConnecting);
        } else {
            startTicking();
        }
    }
}

void MainWindow::setConnectionState(MainWindow::ConnectionState state) {
    if (state != connectionState_)  {
        qCDebug(catApp).noquote() << " State > " << connectionStateName_.value(connectionState_ = state);
        updateWidgetStatus();
    }
}

void MainWindow::socketStateChanged(AbstractSocket::SocketState state) {
    if (state == AbstractSocket::SocketState::ConnectedState || state == AbstractSocket::SocketState::UnconnectedState)  checkState();
}


void MainWindow::on_playButton_clicked() {
    if (mediaState_ == MediaState::None)    on_action_Open_triggered();
    if (mediaState_ == MediaState::Playing) controlProtocol_->mediaPause(true);
    if (mediaState_ == MediaState::Paused) controlProtocol_->mediaPause(false);
}

void MainWindow::closeConnection() {
    dataSocket_->disconnectFromServer();
    connectionState_ = ConnectionState::Offline;
}

void MainWindow::onTok() {
    if (connectionState_ != ConnectionState::Ready) {
        setConnectionState(ConnectionState::Ready);
        stopTicking();
    }
}

void MainWindow::onEventStateChanged(quint64 state) {
    switch (state) {
        case ControlProtocol::EventState::Pause:
            mediaState_ = MediaState::Paused;
            break;
        case ControlProtocol::EventState::Unpause:
        case ControlProtocol::EventState::PlaybackRestart:
            mediaState_ = MediaState::Playing;
            break;
        case ControlProtocol::EventState::EndFile:
            mediaState_ = MediaState::None;
            ui->timeSlider->setValue(0);
            ui->timeLabel->setText(Utils::formatTime(0));
            ui->timePosLabel->setText(Utils::formatTime(0));
            break;
    }
    updateWidgetStatus();
}

void MainWindow::onMediaLength(quint64 length) {
    ui->timeLabel->setText( Utils::formatTime(length) );
    ui->timeSlider->setMaximum(length);
}

void MainWindow::onPlaybackTime(quint64 time) {
    ui->timePosLabel->setText( Utils::formatTime(time));
    if (!ui->timeSlider->isSliderDown()) ui->timeSlider->setValue(time);
}

void MainWindow::on_actionTest_Window_triggered() {
    testWindow_->show();
}

void MainWindow::on_action_Quit_triggered() {
    close();
}

void MainWindow::on_stopButton_clicked() {
    controlProtocol_->mediaStop();
}

void MainWindow::on_timeSlider_sliderReleased() {
    controlProtocol_->mediaSeek(ui->timeSlider->value());
}
