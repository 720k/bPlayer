#pragma once

#include "ui_TestWindow.h"

class MainWindow;
class TestWindow : public QMainWindow, private Ui::TestWindow
{
    Q_OBJECT

public:
    explicit TestWindow(QWidget *parent = nullptr);
    void    print(const QString& text);
protected:
    void    closeEvent(QCloseEvent *event) override;
private slots:
    void on_inputEdit_returnPressed();
    void on_pingButton_clicked();
    void pingReady(double microsecs);
    void updateWidgetStatus();
private:
    MainWindow  *mainWindow_ = nullptr;
};

