#include "TestWindow.h"
#include "MainWindow.h"
#include "TestProtocol.h"
#include "Utils.h"

TestWindow::TestWindow(QWidget *parent) : QMainWindow(parent) {
    setupUi(this);
    mainWindow_ = dynamic_cast<MainWindow*>(parent);

    connect(sendButton, &QPushButton::clicked, this, &TestWindow::on_inputEdit_returnPressed);
    connect(mainWindow_->testProtocol_, &TestProtocol::pingReady,  this, &TestWindow::pingReady);

}

void TestWindow::print(const QString &text) {
    logEdit->appendPlainText(text);
}

void TestWindow::closeEvent(QCloseEvent *event) {
    hide();
    event->ignore();
}

void TestWindow::on_inputEdit_returnPressed() {
    mainWindow_->testProtocol_->sendString(inputEdit->text());
    inputEdit->clear();
}


void TestWindow::on_pingButton_clicked() {
    mainWindow_->testProtocol_->ping(bounceSpinBox->value());
}

void TestWindow::pingReady(double microsecs) {
    logEdit->appendPlainText(QString("Ping %1 milliSecs (average)").arg(Utils::printableNumber(microsecs)));
}

void TestWindow::updateWidgetStatus() {
    bool online = mainWindow_->isOnline();
    interfaceBox->setEnabled(online);
}
