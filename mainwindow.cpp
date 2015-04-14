#include <QWSServer>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>

/* 构造函数 */
MainWindow::MainWindow(QWidget *parent) :QMainWindow(parent), ui(new Ui::MainWindow)
{
    /* 根据ui布局设置窗口 */
    ui->setupUi(this);

    /* 实例化三个线程并启动 */
    logic_thread = new LogicControlThread();
    hardware_thread = new HardWareControlThread();
    dataprocess_thread = new DataProcessThread();

    logic_thread->start();
    hardware_thread->start();
    dataprocess_thread->start();

    /*  */
}

MainWindow::~MainWindow()
{
    delete ui;
    delete logic_thread;
    delete hardware_thread;
    delete dataprocess_thread;
}

void MainWindow::on_Quit_Button_clicked()//退出应用程序
{
    QApplication *p;
    p->quit();
}
