#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "thread_logic.h"
#include "thread_data_proc.h"
#include "thread_hardware.h"
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
    void on_Quit_Button_clicked();//用于退出程序
    void timerUpdate();//用于更新时间
    void realtime_info_update(GUI_REALTIME_INFO realtime_info);

private:
    Ui::MainWindow *ui;
    LogicControlThread *logic_thread;
    HardWareControlThread *hardware_thread;
    DataProcessThread *dataprocess_thread;
    QTimer *timer;
};

#endif // MAINWINDOW_H
