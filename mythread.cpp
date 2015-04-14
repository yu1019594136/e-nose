#include "mythread.h"
#include <QDebug>
#include <QString>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "ds18b20.h"

MyThread::MyThread(QObject *parent) :
    QThread(parent)
{
    stopped = false;
}

void MyThread::run()
{
    char ds18b20_temp_str[]="00.000";
    QString temp;

    while(!stopped)
    {
        DS18B20_Get_TempString("28-0000025ff821", ds18b20_temp_str);
        temp = ds18b20_temp_str;
        emit send_ds18b20_temp(temp);
        //msleep(500);
    }

    /* 线程停止后再显示回原来的"DS18B20 TEMP" */
    emit send_ds18b20_temp("DS18B20 TEMP");

    stopped = false;
}

void MyThread::stop()
{
    stopped = true;
}
