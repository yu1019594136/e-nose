#include <QWSServer>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>

/* 构造函数 */
MainWindow::MainWindow(QWidget *parent) :QMainWindow(parent), ui(new Ui::MainWindow)
{
    /* 根据ui布局设置窗口 */
    ui->setupUi(this);

    ui->show_temp->setText("DS18B20 TEMP");

    /* 鼠标不可见 */
    QWSServer::setCursorVisible(false);

    /* 实例化一个线程 */
    init_heart_thread = new MyThread(this);
    connect(init_heart_thread, SIGNAL(send_ds18b20_temp(QString)), this, SLOT(ds18b20_temp_update(QString)), Qt::QueuedConnection);

    /* 实例化一个定时器 */
    timer_test = new QTimer();
    connect(timer_test, SIGNAL(timeout()), this, SLOT(timerUpdate()));

}

MainWindow::~MainWindow()
{
    delete ui;
    delete init_heart_thread;
    delete timer_test;
}


void MainWindow::on_star_heat_clicked()
{
    init_heart_thread->start();
    ui->star_heat->setEnabled(false);
    ui->stop_heat->setEnabled(true);
}

void MainWindow::on_stop_heat_clicked()
{
    if(init_heart_thread->isRunning())
        init_heart_thread->stop();
    ui->star_heat->setEnabled(true);
    ui->stop_heat->setEnabled(false);
}
void MainWindow::timerUpdate()
{
    static int i = 0;
    qDebug() << "i = " << i++ << endl;
}

void MainWindow::ds18b20_temp_update(QString str_temp)
{
    ui->show_temp->setText(str_temp);
}
void MainWindow::on_start_timer_Button_clicked()
{
    timer_test->start(1000);
    ui->stop_timer_Button->setEnabled(true);
    ui->start_timer_Button->setEnabled(false);
}

void MainWindow::on_stop_timer_Button_clicked()
{
     timer_test->stop();
     ui->start_timer_Button->setEnabled(true);
     ui->stop_timer_Button->setEnabled(false);
}

void MainWindow::on_Quit_Button_clicked()//退出应用程序
{
    QApplication *p;
    p->quit();
}
