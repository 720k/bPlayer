
#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "AbstractSocket.h"
#include "ConsoleColors.h"
#include "ProtocolList.h"
#include "TestProtocol.h"
#include "ControlProtocol.h"
#include "StreamProtocolFileRead.h"
#include "Utils.h"

#include <QLocalServer>
#include <QFileDialog>
#include <QStandardPaths>
#include <QFontDatabase>
#include <QTimer>
#include <chrono>
Q_LOGGING_CATEGORY(catApp, "App")

using namespace std::literals::chrono_literals;

QMap<MainWindow::ConnectionState, QString>  MainWindow::stateName_ = {
    {ConnectionState::Offline,       "Offline"},
    {ConnectionState::Connecting,    "Connecting"},
    {ConnectionState::ConnectingWait,"ConnectingWait"},
    {ConnectionState::Online,        "Online"},
    {ConnectionState::Ready,         "Ready"},
};


void MainWindow::init() {
    connect(ui->sendButton, &QPushButton::clicked, this, &MainWindow::on_inputEdit_returnPressed);

    connectionTypeChanged();
    connect(ui->tcpipSocketRadioButton, &QRadioButton::clicked,     this, &MainWindow::connectionTypeChanged );
    connect(ui->localSocketRadioButton, &QRadioButton::clicked,     this, &MainWindow::connectionTypeChanged );
    // populate BProtocol
    testProtocol_ = new TestProtocol(&protocolList_);
    controlProtocol_ = new ControlProtocol(&protocolList_);
    streamProtocol_ = new StreamProtocolFileRead(&protocolList_);
    protocolList_.insert(testProtocol_);
    protocolList_.insert(controlProtocol_);
    protocolList_.insert(streamProtocol_);
    protocolList_.printDispatchTable();

    connect(testProtocol_,      &TestProtocol::pingReady,       this, &MainWindow::pingReady);
    connect(controlProtocol_,   &ControlProtocol::onTok,        this,&MainWindow::onTok);
    connect(controlProtocol_,   &ControlProtocol::onEventStateChanged, this, &MainWindow::onEventStateChanged);

    int id = QFontDatabase::addApplicationFont(":/font/font-roboto-nerd");
    if (id < 0) {
        qCWarning(catApp).noquote() << "Can't Add font Roboto nerd";
    } else {
        QString family = QFontDatabase::applicationFontFamilies(id).at(0);
        QFont nerd(family);
        qApp->setFont(nerd);
    }
}

bool MainWindow::isLocalSocket() const {
    return ui->localSocketRadioButton->isChecked();
}

bool MainWindow::isOnline() const {
    return connectionState_ == ConnectionState::Ready;
}


MainWindow::MainWindow(QWidget *parent)  : QMainWindow(parent), ui(new Ui::MainWindow){
    ui->setupUi(this);
    ui->portEdit->setText("/tmp/frontend");
    updateWidgetStatus();
    init();
    QTimer::singleShot(3s,this,&MainWindow::startConnecting);
}

MainWindow::~MainWindow() {
    delete streamProtocol_;
    delete controlProtocol_;
    delete testProtocol_;
    delete ui;
}


void MainWindow::updateWidgetStatus() {
    bool online = isOnline();
    ui->tcpipSocketRadioButton->setEnabled(!online);
    ui->localSocketRadioButton->setEnabled(!online);
    ui->portEdit->setEnabled(!online);
    ui->portLabel->setEnabled(ui->portEdit->isEnabled());
    ui->connectButton->setText(online ? "Disconnect" : "Connect");
    ui->interfaceBox->setEnabled(online);
    // MAIN TAB
    QColor c{ static_cast<QRgb>(connectionState_) };
    ui->ledLabel->setStyleSheet(QString("color:%1").arg(c.name()));

    ui->playButton->setText(mediaState_ == MediaState::Playing ? "":"");
}


void MainWindow::on_inputEdit_returnPressed() {
    testProtocol_->sendString(ui->inputEdit->text());
    ui->inputEdit->clear();
}

void MainWindow::on_pingButton_clicked() {
    testProtocol_->ping(ui->bounceSpinBox->value());
}


void MainWindow::pingReady(double microsecs) {
    ui->logEdit->appendPlainText(QString("Ping came back in %1 milliSecs (average)").arg(Utils::printableNumber(microsecs)));
}

void MainWindow::startConnecting() {
    connectionState_ = ConnectionState::Connecting;
    if (isLocalSocket())    socket_->connectToServer(ui->portEdit->text());
    else                    socket_->connectToServer("localhost", ui->portEdit->text().toUInt());
}

void MainWindow::retry() {
    if (connectionState_ == ConnectionState::ConnectingWait) {
        startConnecting();
    }
    updateWidgetStatus();
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



void MainWindow::on_connectButton_clicked() {
    if (isOnline()) closeConnection();
    else            startConnecting();
    updateWidgetStatus();
}

void MainWindow::connectionTypeChanged() {
    // assert: state is offline
    if (socket_) {
        socket_->disconnect(); // no strings attached
        socket_->deleteLater();
    }
    if (isLocalSocket())    socket_ = new LocalSocket();
    else                    socket_ = new TcpSocket();

    // Client socket <-> BProtocol
    connect(socket_,&AbstractSocket::messageReady,    &protocolList_, &ProtocolList::decodeMessage);
    connect(socket_,&AbstractSocket::stateChanged,    this, &MainWindow::socketStateChanged);
    connect(&protocolList_,&ProtocolList::messageReady,     socket_, &AbstractSocket::writeMessage);
}

void MainWindow::on_action_Open_triggered() {
    auto fileName = QFileDialog::getOpenFileName(this,
                            tr("Open File"), QStandardPaths::standardLocations(QStandardPaths::MoviesLocation).at(0),
                            tr("Video (*.mov *.mp4 *.avi *.mpeg *.mpg);Audio (*.m4a *.mp3 *.wav);Any file (*.*)"));
    if (fileName.isNull()) return;
    streamProtocol_->setFileName(fileName_ = fileName);
    controlProtocol_->mediaStart();
}


void MainWindow::socketStateChanged(AbstractSocket::SocketState state) {
    if (state == AbstractSocket::SocketState::UnconnectedState) {
        stopTicking();
        if (connectionState_ == ConnectionState::Connecting)  {
            connectionState_ = ConnectionState::ConnectingWait;
            QTimer::singleShot(3s, this,  &MainWindow::retry);
        }
        if (connectionState_ == ConnectionState::Online || connectionState_ == ConnectionState::Ready)  {
            startConnecting();
        }
    }
    if (state == AbstractSocket::SocketState::ConnectedState) {
        connectionState_ = ConnectionState::Online;
        startTicking();
    }
    qCDebug(catApp).noquote() << " state > " << stateName_.value(connectionState_);
    updateWidgetStatus();
}


void MainWindow::on_playButton_clicked() {
    if (mediaState_ == MediaState::None)    on_action_Open_triggered();
    if (mediaState_ == MediaState::Playing) controlProtocol_->mediaPause(true);
    if (mediaState_ == MediaState::Paused) controlProtocol_->mediaPause(false);
}

void MainWindow::closeConnection() {
    socket_->disconnectFromServer();
    connectionState_ = ConnectionState::Offline;
}

void MainWindow::onTok() {
    if (connectionState_ != ConnectionState::Ready) {
        connectionState_ = ConnectionState::Ready;
        updateWidgetStatus();
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
            break;
    }
    updateWidgetStatus();
}
