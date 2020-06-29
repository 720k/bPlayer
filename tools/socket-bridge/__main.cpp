#include "NetworkLocalServer.h"
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

    QString conduitData     ("io.bplayer.data.0");
    QString conduitStream   ("io.bplayer.stream.0");
    QDir tempDir( QString("/tmp/%1-%2").arg(app.applicationName()).arg(app.applicationPid()) );
    QDir frontendPath = tempDir.absolutePath() + QDir::separator() + "frontend";
    QDir backendPath =  tempDir.absolutePath();

    if (tempDir.exists())   tempDir.removeRecursively();
    frontendPath.mkpath(".");
    backendPath.mkpath(".");
    // conduit DATA
    NetworkLocalServer  serverDataF,serverDataB;
    connectServer(serverDataF,  serverDataB);
    serverDataF.start( frontendPath.absolutePath() + QDir::separator() + conduitData);
    serverDataB.start( backendPath.absolutePath() + QDir::separator() + conduitData);

    NetworkLocalServer  serverStreamF,serverStreamB;
    connectServer(serverStreamF,serverStreamB);
    serverStreamF.start( frontendPath.absolutePath() + QDir::separator() + conduitStream);
    serverStreamB.start( backendPath.absolutePath() + QDir::separator() + conduitStream);

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
    serverDataF.stop();
    serverDataB.stop();
    tempDir.removeRecursively();
    return retvalue;
}
