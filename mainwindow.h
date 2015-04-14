#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "mythread.h"
#include <QTimer>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:

    void on_Quit_Button_clicked();

private:
    Ui::MainWindow *ui;
    LogicControlThread *logic_thread;
    HardWareControlThread *hardware_thread;
    DataProcessThread *dataprocess_thread;
};

#endif // MAINWINDOW_H
