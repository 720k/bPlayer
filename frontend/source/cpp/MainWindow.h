#pragma once

#include <QtWidgets/QMainWindow>
#include <QLoggingCategory>
#include "LocalSocket.h"
#include "TcpSocket.h"
#include "ProtocolList.h"
class QLocalServer;
class StreamProtocolFileRead;
class TestProtocol;
class ControlProtocol;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_inputEdit_returnPressed();
    void on_pingButton_clicked();
    void on_streamButton_clicked();
    void pingReady(double microsecs);
    void on_connectButton_clicked();
    void connectionTypeChanged();

    void on_action_Open_triggered();

    void socketStateChanged(AbstractSocket::SocketState state);
    //void socketError(AbstractSocket);

    void on_selectFileNameButton_clicked();

    void on_localServerCheckBox_clicked();

private:
    Ui::MainWindow *ui;
    std::unique_ptr<QLocalServer>   localServer_;
    bool                            online_;
    AbstractSocket                  *clientSocket_;
    enum ConnectionState            { Offline=0xd40000, Online=0xfff70a, Ready=0x00d600 };

    ProtocolList                    protocolList_;
    TestProtocol                    *testProtocol_;
    ControlProtocol                 *controlProtocol_;
    StreamProtocolFileRead          *streamProtocol_;
    ConnectionState                 state_;

    void                            updateWidgetStatus();
    void                            newClient();
    void                            init();
    bool                            isServerMode();
};

Q_DECLARE_LOGGING_CATEGORY(catApp)
