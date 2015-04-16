#include <unistd.h>
#include <QWSServer>
#include <QDateTime>
#include <QDebug>
#include "mainwindow.h"
#include "ui_mainwindow.h"

Q_DECLARE_METATYPE(GUI_REALTIME_INFO);

/* 构造函数 */
MainWindow::MainWindow(QWidget *parent) :QMainWindow(parent), ui(new Ui::MainWindow)
{
    /* 根据ui布局设置窗口 */
    ui->setupUi(this);
    QWSServer::setCursorVisible(false);

    /* 注册元类型 */
    qRegisterMetaType <GUI_REALTIME_INFO>("GUI_REALTIME_INFO");

    /* 实例化三个线程并启动,将三个子线程相关的信号关联到GUI主线程的槽函数 */

    logic_thread = new LogicControlThread();

    hardware_thread = new HardWareControlThread();
    connect(hardware_thread, SIGNAL(send_realtime_info(GUI_REALTIME_INFO)), this, SLOT(realtime_info_update(GUI_REALTIME_INFO)), Qt::QueuedConnection);

    dataprocess_thread = new DataProcessThread();

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
    ui->pump_speed_value->setText("0 r/min");
    ui->OFF1->setText("OFF");
    ui->OFF2->setText("OFF");
    ui->OFF3->setText("OFF");
    ui->OFF4->setText("OFF");
    ui->heat_duty_evap_value->setText("0/10 ms");
    ui->heat_duty_reac_value->setText("0/10 ms");
}

MainWindow::~MainWindow()
{
    delete ui;
    delete logic_thread;
    delete hardware_thread;
    delete dataprocess_thread;

    qDebug() << "delete something" << endl;
}

void MainWindow::on_Quit_Button_clicked()//退出应用程序
{
    /* 将活跃状态的线程关闭 */
    if(logic_thread->isRunning())
        logic_thread->stop();

    if(hardware_thread->isRunning())
        hardware_thread->stop();

    if(dataprocess_thread->isRunning())
        dataprocess_thread->stop();

    /* 等待三个子线程退出后再结束程序 */
    sleep(1);

    /* 退出时间循环，结束程序 */
    QApplication *p;
    p->quit();
}

void MainWindow::realtime_info_update(GUI_REALTIME_INFO realtime_info)
{
    ui->evap_temp_value->setText(realtime_info.ds18b20_temp);
    //realtime_info.sht21_humid = "123";
    ui->humidty_value->setText(realtime_info.sht21_humid);
    //realtime_info.sht21_temp = "456";
    ui->temp_value_sht21->setText(realtime_info.sht21_temp);
}

void MainWindow::timerUpdate()
{
    QDateTime datetime = QDateTime::currentDateTime();
    ui->datetime->setText(datetime.toString("yyyy.MM.dd hh:mm"));
}
