#include "NetworkLocalServer.h"
#include <QtCore/QCoreApplication>
#include <iostream>
#include <future>
#include <QTimer>
#include <QDebug>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    NetworkLocalServer  serverA_,serverB_;
    QObject::connect(&serverA_, &NetworkLocalServer::dataReady, &serverB_, &NetworkLocalServer::writeData);
    QObject::connect(&serverB_, &NetworkLocalServer::dataReady, &serverA_, &NetworkLocalServer::writeData);
    serverA_.start("/tmp/frontend");
    serverB_.start("/tmp/backend");
    bool exitFlag = false;
    auto exitFn = std::async(std::launch::async, [&exitFlag]{ std::getchar(); exitFlag = true; });
    QTimer exitTimer;
    exitTimer.setInterval(500);
    exitTimer.setSingleShot(false);
    QObject::connect(&exitTimer, &QTimer::timeout, [&a,&exitFlag] { if (exitFlag) a.quit();} );
    exitTimer.start();
    qDebug() << "<ENTER> TO QUIT";
    auto retvalue = a.exec();
    exitFn.wait();
    serverA_.stop();
    serverB_.stop();
    return retvalue;
}
