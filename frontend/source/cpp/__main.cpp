#include "MainWindow.h"

#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setWindowIcon(QIcon(":/image/app"));
    MainWindow mainWindow;
    mainWindow.show();
    return app.exec();
}
