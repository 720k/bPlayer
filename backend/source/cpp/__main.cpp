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
             {{"sp", "stream-port"},QCoreApplication::translate("main", "Stream Port"),QCoreApplication::translate("main", "Port")},
    });
    parser.process(*dynamic_cast<QCoreApplication*>(&app));
    if (parser.isSet("stream-port"))    app.setStreamPortPath(parser.value("stream-port"));
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

