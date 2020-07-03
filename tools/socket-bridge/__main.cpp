#include "NetworkLocalServer.h"
#include "BPlayer.h"
#include <QtCore/QCoreApplication>
#include <iostream>
#include <future>
#include <QTimer>
#include <QDebug>
#include <QFile>
#include <QDir>

void createConduitBetweenServer(const NetworkLocalServer& A, const NetworkLocalServer& B) {
    QObject::connect(&A, &NetworkLocalServer::dataReady, &B, &NetworkLocalServer::writeData);
    QObject::connect(&B, &NetworkLocalServer::dataReady, &A, &NetworkLocalServer::writeData);
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    //QString controlConduitPortName  (BPlayer::controlPortName);
    QString streamConduitPortName   (BPlayer::streamPortName);

    QDir tempDir( QString("/tmp/%1-%2").arg(app.applicationName()).arg(app.applicationPid()) );
    QDir backendPath =  tempDir.absolutePath();
    QDir frontendPath = tempDir.absolutePath() + QDir::separator() + "frontend";

    if (tempDir.exists())   tempDir.removeRecursively();
    frontendPath.mkpath(".");
    backendPath.mkpath(".");

    // conduit CONTROL
//    NetworkLocalServer  controlConduitFrontendServer("Control::Frontend"), controlConduitBackendServer("Control::Backend");
//    createConduitBetweenServer(controlConduitFrontendServer,  controlConduitBackendServer);
//    controlConduitFrontendServer.start( frontendPath.absolutePath() + QDir::separator() + controlConduitPortName);
//    controlConduitBackendServer.start( backendPath.absolutePath() + QDir::separator() + controlConduitPortName);

    // conduit STREAM
    NetworkLocalServer  streamConduitFrontendServer("Stream::Frontend"), streamConduitBackendServer("Stream::Backend");
    createConduitBetweenServer(streamConduitFrontendServer, streamConduitBackendServer);
    streamConduitFrontendServer.start( frontendPath.absolutePath() + QDir::separator() + streamConduitPortName);
    streamConduitBackendServer.start( backendPath.absolutePath() + QDir::separator() + streamConduitPortName);

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
//    controlConduitFrontendServer.stop();
//    controlConduitBackendServer.stop();
    streamConduitFrontendServer.stop();
    streamConduitBackendServer.stop();
    tempDir.removeRecursively();
    return retvalue;
}
