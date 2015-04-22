#include <unistd.h>
#include <QWSServer>
#include <QDateTime>
#include <QDebug>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "HW_interface.h"

Q_DECLARE_METATYPE(GUI_REALTIME_INFO)

/* 构造函数 */
MainWindow::MainWindow(QWidget *parent) :QMainWindow(parent), ui(new Ui::MainWindow)
{
    /* 根据ui布局设置窗口 */
    ui->setupUi(this);
    QWSServer::setCursorVisible(false);

    /* 注册元类型 */
    qRegisterMetaType <GUI_REALTIME_INFO>("GUI_REALTIME_INFO");
    qRegisterMetaType <THERMOSTAT>("THERMOSTAT");
    qRegisterMetaType <BEEP>("BEEP");
    qRegisterMetaType <PUMP>("PUMP");
    qRegisterMetaType <MAGNETIC>("MAGNETIC");
    qRegisterMetaType <SAMPLE>("SAMPLE");

    /* 实例化三个线程并启动,将三个子线程相关的信号关联到GUI主线程的槽函数 */
    logic_thread = new LogicControlThread();
    hardware_thread = new HardWareControlThread();
    dataprocess_thread = new DataProcessThread();

    /* 实时更新温湿度数据 */
    connect(hardware_thread, SIGNAL(send_to_GUI_realtime_info_update(GUI_REALTIME_INFO)), this, SLOT(recei_fro_hard_realtime_info_update(GUI_REALTIME_INFO)), Qt::QueuedConnection);

    /* 逻辑线程发送蒸发室恒温信号给硬件线程 */
    connect(logic_thread, SIGNAL(send_to_hard_evapor_thermostat(THERMOSTAT)), hardware_thread, SLOT(recei_fro_logic_thermostat(THERMOSTAT)), Qt::QueuedConnection);
    connect(hardware_thread, SIGNAL(send_to_logic_thermostat_done()), logic_thread, SLOT(recei_fro_hardware_thermostat_done()), Qt::QueuedConnection);
    /* 恒温操作时GUI实时更新thermostat_duty */
    connect(hardware_thread, SIGNAL(send_to_GUI_thermostat_duty_update(int)), this, SLOT(recei_fro_hard_thermostat_duty_update(int)), Qt::QueuedConnection);

    /* 逻辑线程发送蜂鸣器控制信号给硬件线程 */
    connect(logic_thread, SIGNAL(send_to_hard_beep(BEEP)), hardware_thread, SLOT(recei_fro_logic_beep(BEEP)), Qt::QueuedConnection);

    /* 逻辑线程发送气泵控制信号给硬件线程 */
    connect(logic_thread, SIGNAL(send_to_hard_pump(PUMP)), hardware_thread, SLOT(recei_fro_logic_pump(PUMP)),Qt::QueuedConnection);
    connect(hardware_thread, SIGNAL(send_to_GUI_pump_duty_update(int)), this, SLOT(recei_fro_hard_pump_duty_update(int)),Qt::QueuedConnection);

    /* 逻辑线程发送电磁阀控制信号给硬件线程 */
    connect(logic_thread, SIGNAL(send_to_hard_magnetic(MAGNETIC)), hardware_thread, SLOT(recei_fro_logic_magnetic(MAGNETIC)), Qt::QueuedConnection);
    connect(hardware_thread, SIGNAL(send_to_GUI_magnetic_update(MAGNETIC)), this, SLOT(recei_fro_hard_magnetic_update(MAGNETIC)), Qt::QueuedConnection);

    /* 逻辑线程发送给数据处理线程的采样控制信号 */
    connect(logic_thread, SIGNAL(send_to_dataproc_sample(SAMPLE)), dataprocess_thread, SLOT(recei_fro_logic_sample(SAMPLE)), Qt::QueuedConnection);

    /* 按下quit按钮后关机 */
    connect(this, SIGNAL(send_to_hardware_close_hardware()), hardware_thread, SLOT(recei_fro_GUI_close_hardware()), Qt::QueuedConnection);
    connect(hardware_thread, SIGNAL(return_to_GUI_close_hardware()), this, SLOT(result_fro_hardware_close_hardware()), Qt::QueuedConnection);

    logic_thread->start();
    hardware_thread->start();
    dataprocess_thread->start();

    /* 开机显示时间 */
    QDateTime datetime = QDateTime::currentDateTime();
    ui->datetime->setText(datetime.toString("yyyy.MM.dd hh:mm"));

    /* 实例化一个定时器并启动 */
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(timerUpdate()));
    timer->start(1000);//一分钟更新一次时间

    /* 显示界面各个标签 */
    ui->logo->setText("Electronic Nose");
    ui->evap_temp_lable->setText("evaporation temp:");
    ui->reactio_temp_lable->setText("reaction temp:");
    ui->humidity_lable->setText("reaction humi:");
    ui->heat_duty_evap_lable->setText("evap heat duty:");
    ui->heat_duty_reac_lable->setText("reac heat duty:");
    ui->M1->setText("M1 status:");
    ui->M2->setText("M2 status:");
    ui->M3->setText("M3 status:");
    ui->M4->setText("M4 status:");
    ui->preset_temp->setText("preset temp:");

    /* 显示系统气泵、电磁阀、加热带的初始状态 */
    ui->pump_speed_value->setText("0.0/125.0 us");
    ui->OFF1->setText("OFF");
    ui->OFF2->setText("OFF");
    ui->OFF3->setText("OFF");
    ui->OFF4->setText("OFF");
    ui->heat_duty_evap_value->setText("0.0/10.0 ms");
    ui->heat_duty_reac_value->setText("0.0/10.0 ms");
}

MainWindow::~MainWindow()
{
    delete ui;
    delete logic_thread;
    delete hardware_thread;
    delete dataprocess_thread;
}

void MainWindow::on_Quit_Button_clicked()
{
    /* 通知硬件线程关闭硬件 */
    emit send_to_hardware_close_hardware();
}

void MainWindow::recei_fro_hard_realtime_info_update(GUI_REALTIME_INFO realtime_info)
{
    ui->evap_temp_value->setText(realtime_info.ds18b20_temp);
    ui->humidty_value->setText(realtime_info.sht21_humid);
    ui->temp_value_sht21->setText(realtime_info.sht21_temp);
}

void MainWindow::timerUpdate()
{
    QDateTime datetime = QDateTime::currentDateTime();
    ui->datetime->setText(datetime.toString("yyyy.MM.dd hh:mm"));
}

/* 接收来自硬件线程的恒温实时duty值 */
void MainWindow::recei_fro_hard_thermostat_duty_update(int duty_info)
{
    /* 将duty换算成ms单位 */
    ui->heat_duty_evap_value->setText(QString::number(duty_info/1000000.0, 'f', 1) + "/10.0 ms");
}

/* 接收来自硬件线程的开启气泵时duty值 */
void MainWindow::recei_fro_hard_pump_duty_update(int duty_info)
{
    /* 将duty换算成us单位 */
    ui->pump_speed_value->setText(QString::number(duty_info/1000.0, 'f', 1) + "/125.0 us");
}

void MainWindow::recei_fro_hard_magnetic_update(MAGNETIC magnetic_info)
{
    if(magnetic_info.M1 == HIGH)
        ui->OFF1->setText("ON");
    else
        ui->OFF1->setText("OFF");

    if(magnetic_info.M2 == HIGH)
        ui->OFF2->setText("ON");
    else
        ui->OFF2->setText("OFF");


    if(magnetic_info.M3 == HIGH)
        ui->OFF3->setText("ON");
    else
        ui->OFF3->setText("OFF");


    if(magnetic_info.M4 == HIGH)
        ui->OFF4->setText("ON");
    else
        ui->OFF4->setText("OFF");
}

void MainWindow::result_fro_hardware_close_hardware()//退出应用程序,执行关机命令
{
    /* 将活跃状态的线程关闭 */
    if(logic_thread->isRunning())
        logic_thread->stop();

    if(hardware_thread->isRunning())
        hardware_thread->stop();

    if(dataprocess_thread->isRunning())
        dataprocess_thread->stop();

    /* 等待三个子线程退出后再结束程序 */
    while(!logic_thread->isFinished());
    while(!hardware_thread->isFinished());
    while(!dataprocess_thread->isFinished());

    /* 启动另一个进程执行关机命令,5s后关机 */
    //Application_quit(5);

    /* 退出时间循环，结束程序 */
    QApplication *p;
    p->quit();
}
