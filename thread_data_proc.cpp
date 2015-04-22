#include "thread_data_proc.h"

//#include <signal.h>
//#include <time.h>
//#include <sys/time.h>
//#include <unistd.h>

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

long sample_count;
long sum = 0;
struct itimerval value;

FILE *fp;
struct tm *tm_ptr;
time_t the_time;
char filename[20];

uint16_t tlc1543_txbuf[1] = {0};	//SPI通信发送缓冲区
uint16_t tlc1543_rxbuf[1] = {0};	//SPI通信接收缓冲区
uint16_t channel = 0;		//要转换的通道号，取值0x00-0x0D，
int i = 1;

/* setitimer定时事件代码
 *注意，定时事件代码的消耗时间不能比定时时间还长，否则将会有定时中断丢失，程序运行结果会有错误
 */
void timer_func(int signal_type)
{
    sample_count--;

    for(i = 1; i < 10; i++)
    {
        channel = i;
        tlc1543_txbuf[0] = (channel<<12)| 0x0c00;//写入要转换的通道号,16位精度(0x0c00),12位精度(0x0000/0x0800),8位精度(0x0400)
        tlc1543_Transfer(tlc1543_txbuf, tlc1543_rxbuf, 2);//数据交换后得到0通道数据
        fprintf(fp, "%d\t", tlc1543_rxbuf[0]);
        usleep(WAIT_CONVERSION);
    }
    channel = 0;
    tlc1543_txbuf[0] = (channel<<12)| 0x0c00;//写入要转换的通道号,16位精度(0x0c00),12位精度(0x0000/0x0800),8位精度(0x0400)
    tlc1543_Transfer(tlc1543_txbuf, tlc1543_rxbuf, 2);//数据交换后得到0通道数据
    fprintf(fp, "%d\n", tlc1543_rxbuf[0]);

    if(sample_count == 0)
    {
        /* 关闭定时器 */
        value.it_value.tv_sec = 0;
        value.it_value.tv_usec = 0;
        value.it_interval = value.it_value;

        setitimer(ITIMER_REAL, &value, NULL);

        /* 关闭文件句柄，保存数据文件，同步写到硬盘中 */
        sync();
        fclose(fp);
        fp = NULL;
    }
    else
    {
        signal(SIGALRM, timer_func);
    }

}

/*********************数据处理线程*****************************/
DataProcessThread::DataProcessThread(QObject *parent) :
    QThread(parent)
{
    stopped = false;
}

void DataProcessThread::run()
{
    msleep(100);
    //collect_data();

    while (!stopped)
    {
        sleep(10);
    }
    stopped = false;
}

void DataProcessThread::stop()
{
    stopped = true;
}

void DataProcessThread::recei_fro_logic_sample(SAMPLE sample_para)
{
    sample.sample_freq = sample_para.sample_freq;
    sample.sample_time = sample_para.sample_time;
    sample.filename_prefix = sample_para.filename_prefix;

    /* 计算总的定时次数, 每次定时都会在定时事件代码中采集所有通道 */
    sample_count = sample.sample_time * sample.sample_freq;

    value.it_value.tv_sec = 10.0 / sample.sample_freq;//每个通道的周期乘以总通道数,得到定时器定时时间
    value.it_value.tv_usec = (int)(10000000.0 / sample.sample_freq) % 1000000;//每个通道的周期乘以总通道数,得到定时器定时时间
    value.it_interval = value.it_value;

    qDebug("value.it_value.tv_sec = %d\n",value.it_value.tv_sec);
    qDebug("value.it_value.tv_usec = %d\n",value.it_value.tv_usec);
    qDebug("sample_count = %ld\n",sample_count);

    (void)time(&the_time);
    tm_ptr = localtime(&the_time);
    sprintf(filename, "/root/qt_program/%s_%d.%d.%d-%d_%d_%d.txt",sample.filename_prefix, tm_ptr->tm_year + 1900, tm_ptr->tm_mon + 1, tm_ptr->tm_mday, tm_ptr->tm_hour, tm_ptr->tm_min, tm_ptr->tm_sec);

    /* 创建文件保存数据 */
    if((fp=fopen(filename,"w"))==NULL)
    {
        qDebug("cannot open file\n");
        exit(0);
    }

    /* 第一次采集的数据不准确丢弃 */
    channel = 0;
    tlc1543_txbuf[0] = (channel<<12)| 0x0c00;//写入要转换的通道号,16位精度(0x0c00),12位精度(0x0000/0x0800),8位精度(0x0400)
    tlc1543_Transfer(tlc1543_txbuf, tlc1543_rxbuf, 2);//数据交换后得到0通道数据

    /* 设置定时信号 */
    signal(SIGALRM, timer_func);

    /* 启动定时器 */
    setitimer(ITIMER_REAL, &value, NULL);
}


