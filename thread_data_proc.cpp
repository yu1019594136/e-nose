#include "thread_data_proc.h"
#include <QDebug>
#include <QDateTime>

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

unsigned int i;
unsigned int j;
volatile long sample_count;//按照采样时间和采样频率计算出的采样次数

u_int16_t tlc1543_txbuf[1];	//SPI通信发送缓冲区
u_int16_t tlc1543_rxbuf[1];	//SPI通信接收缓冲区
u_int16_t channel;		//要转换的通道号，取值0x00-0x0D，

u_int16_t **p_data;//创建一个无符号整型的二维数组指针，该全局变量用于数据处理线程和GUI线程之间传递数据

/*********************数据处理线程*****************************/
DataProcessThread::DataProcessThread(QObject *parent) :
    QThread(parent)
{
    stopped = false;
    sample_count = 0;
    sample_flag = false;

    plot_info.width = 1000;
    plot_info.height = 65536;
    plot_info.sample_count_real = 0;

    filename = NULL;
    fp = NULL;
    p_data = NULL;

    tlc1543_txbuf[0] = 0;	//SPI通信发送缓冲区
    tlc1543_rxbuf[0] = 0;	//SPI通信接收缓冲区
    channel = 0;		//要转换的通道号，取值0x00-0x0D，
    i = 0;
    j = 0;

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

                p_data[plot_info.sample_count_real][i-1] = tlc1543_rxbuf[0];//读取缓冲区数据到内存空间

                //fprintf(fp, "%d\t", tlc1543_rxbuf[0]);
                //fprintf(fp, "%.3f\t", (tlc1543_rxbuf[0] * 5.02) / 65536);
                //qDebug("%.3f\t", (tlc1543_rxbuf[0] * 5.02) / 65536;
            }
            channel = 0;
            tlc1543_txbuf[0] = (channel<<12)| 0x0c00;//写入要转换的通道号,16位精度(0x0c00),12位精度(0x0000/0x0800),8位精度(0x0400)
            tlc1543_Transfer(tlc1543_txbuf, tlc1543_rxbuf, 2);//数据交换后得到0通道数据
            usleep(WAIT_CONVERSION);

            p_data[plot_info.sample_count_real][9] = tlc1543_rxbuf[0];//读取缓冲区数据到内存空间

            //fprintf(fp, "%d\n", tlc1543_rxbuf[0]);
            //fprintf(fp, "%.3f\n", (tlc1543_rxbuf[0] * 5.02) / 65536);
            //qDebug("%.3f\n", (tlc1543_rxbuf[0] * 5.02) / 65536);

            plot_info.width = 800;
            plot_info.height = 65536;
            ++plot_info.sample_count_real;

            /* 通知GUI线程根据全局变量sample_count_real进行数据绘图 */
            emit send_to_PlotWidget_plotdata(plot_info);

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

    QDateTime datetime = QDateTime::currentDateTime();

    sample.sample_freq = sample_para.sample_freq;
    sample.sample_time = sample_para.sample_time;
    sample.filename_prefix = QString("/root/qt_program/") + sample_para.filename_prefix + datetime.toString("_yyyy.MM.dd-hh_mm_ss") + ".txt";
    sample.sample_inform_flag = sample_para.sample_inform_flag;

    QByteArray ba = sample.filename_prefix.toLatin1();
    filename = ba.data();

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
    plot_info.sample_count_real = 0;
    /* 计算总的定时次数以及定时时间, 每次定时都会在定时事件代码中采集所有通道 */
    sample_count = sample.sample_time * sample.sample_freq;
    temp_time = 1000.0 / sample.sample_freq;

    /* 分配二维内存空间保存数据，一共sample_count个一维数组，每个一维数组长度10,存储10个电压值 */
    p_data = (u_int16_t **)malloc(sizeof(u_int16_t *) * sample_count);
    for(i = 0; i< sample_count; i++)
    {
        p_data[i] = (u_int16_t *)malloc(sizeof(u_int16_t) * 10);
        memset(p_data[i], 0, sizeof(u_int16_t) * 10);//将分配的内存空间初始化为0
    }

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

        qDebug("sample_count_real = %ld\n", plot_info.sample_count_real);

        /* 将内存空间的数据保存到文件 */
        for(i = 0; i < plot_info.sample_count_real; i++)
        {
            for(j = 0; j < 9; j++)
            {
                fprintf(fp, "%d\t", p_data[i][j]);
            }
            fprintf(fp, "%d\n", p_data[i][9]);
        }

        qDebug() <<"timer stop and data saved in filepath:" << sample.filename_prefix << endl;

        /* 释放空间，清空相关指针变量 */
        plot_info.sample_count_real = 0;

        /* 数据空间p_data被销毁，此时如果再打开绘图选项卡会激活绘图事件函数,造成绘图函数访问不存在的数据空间，
         * 所以完成一次采样和绘图后应该及时关闭绘图代码 */
        emit send_to_PlotWidget_plotdata(plot_info);

        free(p_data);
        p_data = NULL;
        fclose(fp);
        fp = NULL;
        filename = NULL;

        if(sample.sample_inform_flag)
        {
            /* 发送采样完成信号给逻辑线程 */
            emit send_to_logic_sample_done();
        }
        else//此种情况不需要返回信号，系统操作面板中的plot按钮在采集完后需要被使能
        {
            emit send_to_GUI_enable_plot_pushbutton();
        }
    }
}
