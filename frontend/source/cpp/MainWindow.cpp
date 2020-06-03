
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
Q_LOGGING_CATEGORY(catApp, "App")

void MainWindow::init() {
    localServer_ = std::make_unique<QLocalServer>();
    connect(localServer_.get(), &QLocalServer::newConnection, this, &MainWindow::newClient);

    connectionTypeChanged();

    // populate BProtocol
    testProtocol_ = new TestProtocol(&protocolList_);
    controlProtocol_ = new ControlProtocol(&protocolList_);
    streamProtocol_ = new StreamProtocolFileRead(&protocolList_);
    protocolList_.insert(testProtocol_);
    protocolList_.insert(controlProtocol_);
    protocolList_.insert(streamProtocol_);
    protocolList_.printDispatchTable();

    connect(testProtocol_, &TestProtocol::pingReady, this, &MainWindow::pingReady);

    int id = QFontDatabase::addApplicationFont(":/font/font-roboto-nerd");
    if (id < 0) {
        qCWarning(catApp).noquote() << "Can't Add font Roboto nerd";
    } else {
        QString family = QFontDatabase::applicationFontFamilies(id).at(0);
        QFont nerd(family);
        qApp->setFont(nerd);
    }
}

bool MainWindow::isServerMode() {
    return ui->localServerCheckBox->isChecked();
}

MainWindow::MainWindow(QWidget *parent)  : QMainWindow(parent)  , ui(new Ui::MainWindow),online_(false), clientSocket_(nullptr), state_(Offline)  {
    ui->setupUi(this);
    ui->portEdit->setText(AbstractSocket::testPort());
    connect(ui->sendButton, &QPushButton::clicked, this, &MainWindow::on_inputEdit_returnPressed);
    updateWidgetStatus();
    init();
}

MainWindow::~MainWindow() {
    delete streamProtocol_;
    delete controlProtocol_;
    delete testProtocol_;
    delete ui;
}



void MainWindow::updateWidgetStatus() {
    bool offline = !online_;
    ui->localServerCheckBox->setEnabled(offline);
    ui->portEdit->setEnabled(offline && !ui->localServerCheckBox->isChecked());
    ui->portLabel->setEnabled(ui->portEdit->isEnabled());
    ui->connectButton->setText(online_ ? "Disconnect" : "Connect");
    ui->interfaceBox->setEnabled(online_);
}

void MainWindow::newClient() {
    if (!localServer_->hasPendingConnections()) return;
    qCInfo(catApp).noquote() << "New Client accepted";
    auto socket = localServer_->nextPendingConnection();
    //#todo: server must accept only one connection.
    clientSocket_->setSocket(socket);
}

void MainWindow::on_inputEdit_returnPressed() {
    testProtocol_->sendString(ui->inputEdit->text());
    ui->inputEdit->clear();
}

void MainWindow::on_pingButton_clicked() {
    testProtocol_->ping(ui->bounceSpinBox->value());
}

void MainWindow::on_streamButton_clicked() {
    streamProtocol_->setFileName(ui->filenameEdit->text());
    controlProtocol_->mediaStart();
}

void MainWindow::pingReady(double microsecs) {
    ui->logEdit->appendPlainText(QString("Ping came back in %1 milliSecs (average)").arg(Utils::printableNumber(microsecs)));
}

void MainWindow::on_connectButton_clicked() {
    if (isServerMode()) {
        if (online_) {
            clientSocket_->disconnectFromServer();
            localServer_->close();
            online_=false;
            qCDebug(catApp).noquote() << "Local Server Offline";
            state_ = Offline;
        } else {
            if (localServer_->listen(AbstractSocket::testPort())) {
                online_=true;
                state_ = Online;
                qCDebug(catApp).noquote() << "Local Server ready, listening on: " << localServer_->serverName();
            } else {
                state_ = Offline;
                qCDebug(catApp).noquote() << "Local Server error: " << localServer_->errorString() << " on port: " << localServer_->serverName() << cEOL;
            }
        }
        updateWidgetStatus();
    } else {
        if (online_)    clientSocket_->disconnectFromServer();
        else            clientSocket_->connectToServer("localhost", ui->portEdit->text().toUInt());
    }
}

void MainWindow::connectionTypeChanged() {
    // assert: state is offline
    if (clientSocket_) {
        clientSocket_->disconnect(); // no strings attached
        clientSocket_->deleteLater();
    }
    if (isServerMode()) clientSocket_ = new LocalSocket();
    else                clientSocket_ = new TcpSocket();

    // Client socket <-> BProtocol
    connect(clientSocket_,&AbstractSocket::messageReady,   &protocolList_, &ProtocolList::decodeMessage);
    connect(&protocolList_,&ProtocolList::messageReady,    clientSocket_, &AbstractSocket::writeMessage);
    connect(clientSocket_,&AbstractSocket::stateChanged , this, &MainWindow::socketStateChanged);
}

void MainWindow::on_action_Open_triggered() {
    void on_selectFileNameButton_clicked();
    on_streamButton_clicked();
}


void MainWindow::socketStateChanged(AbstractSocket::SocketState state) {
    if (!isServerMode()) { // socket status-> online/offlien
        if (state == AbstractSocket::SocketState::UnconnectedState) {
            online_ = false;
            state_ = Offline;
            qCDebug(catApp).noquote() << "Client disconnected";
        }
        if (state == AbstractSocket::SocketState::ConnectedState) {
            online_ = true;
            state_ = Online;
            qCDebug(catApp).noquote() << "Client connected to localhost:" << localServer_->serverName();
        }
        updateWidgetStatus();
    }
}

void MainWindow::on_selectFileNameButton_clicked() {
    auto fileName = QFileDialog::getOpenFileName(this,
                        tr("Open File"), QStandardPaths::standardLocations(QStandardPaths::MoviesLocation).at(0),
                        tr("Video (*.mov *.mp4 *.avi *.mpeg *.mpg);Audio (*.m4a *.mp3 *.wav);Any file (*.*)"));
    ui->filenameEdit->setText(fileName);
}

void MainWindow::on_localServerCheckBox_clicked() {
    connectionTypeChanged();
    ui->portEdit->setEnabled(!isServerMode());
}

void MainWindow::on_playButton_clicked() {
    controlProtocol_->mediaPause(true);
}
