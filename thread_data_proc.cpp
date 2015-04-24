#include "thread_data_proc.h"
#include <QDebug>

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/time.h>
#include <signal.h>
#include <error.h>
#include "tlc1543.h"
#include "HW_interface.h"

/* setitimer定时事件代码
定时事件代码的执行时间一定不能比定时时间还长，否则产生定时丢失，程序运行将出错，数据处理线程的出错将影响到其他线程的工作。
*/
//void timer_func(int signal_type)
//{
//    gettimeofday(&start, NULL);

//    sample_flag = true;

//    sample_count--;

//    if(sample_count == 0)
//    {
//        value.it_value.tv_sec = 0;
//        value.it_value.tv_usec = 0;
//        value.it_interval = value.it_value;

//        setitimer(SIGALRM, &value, NULL);
//        sample_flag = false;
//    }
//    else
//    {
//        signal(SIGALRM, timer_func);
//    }

//    gettimeofday(&end, NULL);

//    diff = 1000000 * (end.tv_sec - start.tv_sec) + end.tv_usec - start.tv_usec;
//    qDebug("diff = %ld\n", diff);
//}

/*********************数据处理线程*****************************/
DataProcessThread::DataProcessThread(QObject *parent) :
    QThread(parent)
{
    stopped = false;
    sample_count = 0;
    sample_count_real = 0;
    sample_flag = false;
    fp = NULL;

    tlc1543_txbuf[0] = 0;	//SPI通信发送缓冲区
    tlc1543_rxbuf[0] = 0;	//SPI通信接收缓冲区
    channel = 0;		//要转换的通道号，取值0x00-0x0D，
    i = 1;

    sample_timer = new QTimer();
    connect(sample_timer, SIGNAL(timeout()), this, SLOT(sample_timeout()));
}

void DataProcessThread::run()
{
    msleep(100);

    while (!stopped)
    {
        while(sample_count)
        {
            for(i = 1; i < 10; i++)
            {
                channel = i;
                tlc1543_txbuf[0] = (channel<<12)| 0x0c00;//写入要转换的通道号,16位精度(0x0c00),12位精度(0x0000/0x0800),8位精度(0x0400)
                tlc1543_Transfer(tlc1543_txbuf, tlc1543_rxbuf, 2);//数据交换后得到0通道数据
                usleep(WAIT_CONVERSION);
                //fprintf(fp, "%d\t", tlc1543_rxbuf[0]);
                fprintf(fp, "%.3f\t", (tlc1543_rxbuf[0] * 5.02) / 65536);
                //qDebug("%.3f\t", (tlc1543_rxbuf[0] * 5.02) / 65536;
            }
            channel = 0;
            tlc1543_txbuf[0] = (channel<<12)| 0x0c00;//写入要转换的通道号,16位精度(0x0c00),12位精度(0x0000/0x0800),8位精度(0x0400)
            tlc1543_Transfer(tlc1543_txbuf, tlc1543_rxbuf, 2);//数据交换后得到0通道数据
            usleep(WAIT_CONVERSION);
            //fprintf(fp, "%d\n", tlc1543_rxbuf[0]);
            fprintf(fp, "%.3f\n", (tlc1543_rxbuf[0] * 5.02) / 65536);
            //qDebug("%.3f\n", (tlc1543_rxbuf[0] * 5.02) / 65536);

            ++sample_count_real;

            while(!sample_flag);
            sample_flag = false;
        }

    }
    stopped = false;
    qDebug("DataProcessThread done!\n");
}

void DataProcessThread::stop()
{
    stopped = true;
}

void DataProcessThread::recei_fro_logic_sample(SAMPLE sample_para)
{
    int temp_time = 0;

    sample.sample_freq = sample_para.sample_freq;
    sample.sample_time = sample_para.sample_time;
    sample.filename_prefix = sample_para.filename_prefix;

    filename = (char *)malloc(sizeof(char) * (strlen(sample.filename_prefix) + 25));//25个字符空间用于存储年月日时分秒以及下划线

    (void)time(&the_time);
    tm_ptr = localtime(&the_time);
    sprintf(filename, "%s_%d.%d.%d-%d_%d_%d.txt", sample.filename_prefix, tm_ptr->tm_year + 1900, tm_ptr->tm_mon + 1, tm_ptr->tm_mday, tm_ptr->tm_hour, tm_ptr->tm_min, tm_ptr->tm_sec);

    /* 创建文件保存数据 */
    if((fp=fopen(filename,"w"))==NULL)
    {
        qDebug("cannot open file\n");
        //exit(0);
    }

    /* 第一次采集的数据不准确丢弃 */
    channel = 0;
    tlc1543_txbuf[0] = (channel<<12)| 0x0c00;//写入要转换的通道号,16位精度(0x0c00),12位精度(0x0000/0x0800),8位精度(0x0400)
    tlc1543_Transfer(tlc1543_txbuf, tlc1543_rxbuf, 2);//数据交换后得到0通道数据
    usleep(WAIT_CONVERSION);

    sample_flag = false;
    /* 计算总的定时次数以及定时时间, 每次定时都会在定时事件代码中采集所有通道 */
    sample_count = sample.sample_time * sample.sample_freq;
    temp_time = 1000.0 / sample.sample_freq;
    qDebug("sample_count = %ld\n",sample_count);
    qDebug("temp_time = %d ms\n",temp_time);

    sample_timer->start(temp_time);
}

void DataProcessThread::sample_timeout()
{
    sample_flag = true;

    sample_count--;

    if(sample_count == 0)
    {
        sample_timer->stop();
        qDebug("sample_count_real = %ld\n", sample_count_real);
        sample_count_real = 0;
        fclose(fp);
        fp = NULL;
        free(filename);
        filename = NULL;
        qDebug("timer stop\n");
    }
}
