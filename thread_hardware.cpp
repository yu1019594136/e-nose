#include "thread_hardware.h"

/* 硬件相关接口 */
#include "ds18b20.h"
#include "sht21.h"

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
