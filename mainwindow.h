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

    void on_star_heat_clicked();

    void on_stop_heat_clicked();

    void timerUpdate();

    void ds18b20_temp_update(QString str_temp);

    void on_start_timer_Button_clicked();

    void on_stop_timer_Button_clicked();

    void on_Quit_Button_clicked();

private:
    Ui::MainWindow *ui;
    MyThread *init_heart_thread;//主窗口包含了一个线程，该线程用于开机后进行蒸发室和反应室的预热
    QTimer *timer_test;
    //MyThread *init_heart_thread;
};

#endif // MAINWINDOW_H
