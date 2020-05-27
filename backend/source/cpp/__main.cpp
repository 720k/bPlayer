#include "ConsoleApplication.h"
#include <QLocalSocket>
#include <QTimer>
#include <QDebug>
#include <QScopedPointer>

#include <iostream>
#include <future>
#include <QCommandLineParser>

int main(int argc, char *argv[])    {
    ConsoleApplication app(argc, argv);
    QCommandLineParser parser;
    parser.setApplicationDescription("BPlayer-backend");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addOptions({
             {{"p", "port"},QCoreApplication::translate("main", "Port"),QCoreApplication::translate("main", "Port")},
    });
    parser.process(*dynamic_cast<QCoreApplication*>(&app));
    if (parser.isSet("port")) {
        QString portName = parser.value("port");
        app.setPortName(portName);
    }
    bool exitFlag = false;
    auto exitFn = std::async(std::launch::async, [&exitFlag]{ std::getchar(); exitFlag = true; });
    QTimer exitTimer;
    exitTimer.setInterval(500);
    exitTimer.setSingleShot(false);
    QObject::connect(&exitTimer, &QTimer::timeout, [&app,&exitFlag]{ if (exitFlag) app.quit();} );
    exitTimer.start();
    qDebug() << "*** JUST PRESS <ENTER> TO QUIT THE APPLICATION ***";
    auto retValue = app.run();
    exitFn.wait();
    return retValue;
}

