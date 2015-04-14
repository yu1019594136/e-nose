#include "mythread.h"
#include <QDebug>
#include <QString>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "ds18b20.h"

/*********************逻辑控制线程*****************************/
LogicControlThread::LogicControlThread(QObject *parent) :
    QThread(parent)
{
    stopped = false;
}

void LogicControlThread::run()
{
//    while (!stopped)
//    {
        dosomething();
//    }

    stopped = false;
}
void LogicControlThread::stop()
{
    stopped = true;
}
/*********************硬件控制线程*****************************/
HardWareControlThread::HardWareControlThread(QObject *parent) :
    QThread(parent)
{
    stopped = false;
}

void HardWareControlThread::run()
{
//    while (!stopped)
//    {
        dosomething();
//    }

    stopped = false;
}

void HardWareControlThread::stop()
{
    stopped = true;
}

/*********************数据处理线程*****************************/
DataProcessThread::DataProcessThread(QObject *parent) :
    QThread(parent)
{
    stopped = false;
}

void DataProcessThread::run()
{
//    while (!stopped)
//    {
        dosomething();
//    }

    stopped = false;
}

void DataProcessThread::stop()
{
    stopped = true;
}

/*********************dosomething()*****************************/
void dosomething()
{
    qDebug() << "I,am do something!" << endl;
}
