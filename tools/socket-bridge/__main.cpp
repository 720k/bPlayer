#include "NetworkLocalServer.h"
#include "BPlayer.h"
#include <QtCore/QCoreApplication>
#include <iostream>
#include <future>
#include <QTimer>
#include <QDebug>
#include <QFile>
#include <QDir>

void connectServer(const NetworkLocalServer& A, const NetworkLocalServer& B) {
    QObject::connect(&A, &NetworkLocalServer::dataReady, &B, &NetworkLocalServer::writeData);
    QObject::connect(&B, &NetworkLocalServer::dataReady, &A, &NetworkLocalServer::writeData);
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    QString controlConduitPortName  (BPlayer::controlPortName);
    QString streamConduitPortName   (BPlayer::streamPortName);

    QDir tempDir( QString("/tmp/%1-%2").arg(app.applicationName()).arg(app.applicationPid()) );
    QDir backendPath =  tempDir.absolutePath();
    QDir frontendPath = tempDir.absolutePath() + QDir::separator() + "frontend";

    if (tempDir.exists())   tempDir.removeRecursively();
    frontendPath.mkpath(".");
    backendPath.mkpath(".");
    // conduit CONTROL
    NetworkLocalServer  serverFrontendControlConduit, serverBackendControlConduit;
    connectServer(serverFrontendControlConduit,  serverBackendControlConduit);
    serverFrontendControlConduit.start( frontendPath.absolutePath() + QDir::separator() + controlConduitPortName);
    serverBackendControlConduit.start( backendPath.absolutePath() + QDir::separator() + controlConduitPortName);
    // conduit STREAM
    NetworkLocalServer  serverFrontendStreamConduit, serverBackendStreamConduit;
    connectServer(serverFrontendStreamConduit,serverBackendStreamConduit);
    serverFrontendStreamConduit.start( frontendPath.absolutePath() + QDir::separator() + streamConduitPortName);
    serverBackendStreamConduit.start( backendPath.absolutePath() + QDir::separator() + streamConduitPortName);

    bool exitFlag = false;
    auto exitFn = std::async(std::launch::async, [&exitFlag]{ std::getchar(); exitFlag = true; });
    QTimer exitTimer;
    exitTimer.setInterval(500);
    exitTimer.setSingleShot(false);
    QObject::connect(&exitTimer, &QTimer::timeout, [&app,&exitFlag] { if (exitFlag) app.quit();} );
    exitTimer.start();
    qDebug() << "<ENTER> TO QUIT";
    auto retvalue = app.exec();
    exitFn.wait();
    serverFrontendControlConduit.stop();
    serverBackendControlConduit.stop();
    tempDir.removeRecursively();
    return retvalue;
}
