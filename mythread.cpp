#include "mythread.h"
#include <QDebug>
#include <QString>
#include "mainwindow.h"
#include "ui_mainwindow.h"

/* 硬件相关接口 */
#include "ds18b20.h"
#include "sht21.h"

/*********************逻辑控制线程*****************************/
LogicControlThread::LogicControlThread(QObject *parent) :
    QThread(parent)
{
    /*  */

    stopped = false;
}

void LogicControlThread::run()
{
//    while (!stopped)
//    {
        //dosomething();
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
    char temp_str[]="00.000";
    GUI_REALTIME_INFO realtime_info;

    while (!stopped)
    {
        /* 不断采集蒸发室和反应室温度以及湿度数据，发送给GUI线程进行数据更新 */
        DS18B20_Get_TempString("28-0000025ff821", temp_str);
        realtime_info.ds18b20_temp = temp_str;

        sht21_get_temp_string(temp_str);
        realtime_info.sht21_temp = temp_str;

        sht21_get_humidity_string(temp_str);
        realtime_info.sht21_humid = temp_str;

        emit send_realtime_info(realtime_info);

        msleep(500);
    }

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
