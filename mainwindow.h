#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include "thread_logic.h"
#include "thread_data_proc.h"
#include "thread_hardware.h"


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

signals:
    /* 通知硬件线程关闭硬件 */
    void send_to_hardware_close_hardware();

private slots:
    void on_Quit_Button_clicked();//用于退出程序
    void timerUpdate();//用于更新时间

public slots:
    /* 接收硬件线程发送来的实时温湿度数据 */
    void recei_fro_hard_realtime_info_update(GUI_REALTIME_INFO realtime_info);

    /* 接收来自硬件线程的关闭硬件操作结果 */
    void result_fro_hardware_close_hardware();

    /* 接收来自硬件线程的恒温实时duty值 */
    void recei_fro_hard_duty_update(int duty);

private:
    Ui::MainWindow *ui;
    LogicControlThread *logic_thread;
    HardWareControlThread *hardware_thread;
    DataProcessThread *dataprocess_thread;
    QTimer *timer;
};

#endif // MAINWINDOW_H
