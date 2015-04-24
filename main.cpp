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

#include <QApplication>
#include <QTextCodec>
#include <QDebug>
#include "mainwindow.h"
#include "tlc1543.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    MainWindow mainwindow;
    mainwindow.showFullScreen();
    return app.exec();

//    FILE *fp;
//    struct tm *tm_ptr;
//    time_t the_time;
//    char filename[20];

//    uint16_t tlc1543_txbuf[1] = {0};	//SPI通信发送缓冲区
//    uint16_t tlc1543_rxbuf[1] = {0};	//SPI通信接收缓冲区
//    uint16_t channel = 0;		//要转换的通道号，取值0x00-0x0D，
//    int i = 1;
//    int count = 100;

//    /* 时间函数相关 */
//    struct timeval start;
//    struct timeval end;
//    struct timeval end1;
//    long diff = 0;

//    (void)time(&the_time);
//    tm_ptr = localtime(&the_time);
//    sprintf(filename, "/root/qt_program/%s_%d.%d.%d-%d_%d_%d.txt","test", tm_ptr->tm_year + 1900, tm_ptr->tm_mon + 1, tm_ptr->tm_mday, tm_ptr->tm_hour, tm_ptr->tm_min, tm_ptr->tm_sec);

//    /* 创建文件保存数据 */
//    if((fp=fopen(filename,"w"))==NULL)
//    {
//        //qDebug("cannot open file\n");
//        exit(0);
//    }

//    /* 片选线用gpio31 */
//    tlc1543_Init(31);

//    channel = 0;
//    tlc1543_txbuf[0] = (channel<<12)| 0x0c00;//写入要转换的通道号,16位精度(0x0c00),12位精度(0x0000/0x0800),8位精度(0x0400)
//    tlc1543_Transfer(tlc1543_txbuf, tlc1543_rxbuf, 2);//数据交换后得到0通道数据
//    usleep(WAIT_CONVERSION);

//    gettimeofday(&start, NULL);

//    while(count--)
//    {
//        for(i = 1; i < 10; i++)
//        {
//            channel = i;
//            tlc1543_txbuf[0] = (channel<<12)| 0x0c00;//写入要转换的通道号,16位精度(0x0c00),12位精度(0x0000/0x0800),8位精度(0x0400)
//            tlc1543_Transfer(tlc1543_txbuf, tlc1543_rxbuf, 2);//数据交换后得到0通道数据
//            //fprintf(fp, "%d\t", tlc1543_rxbuf[0]);
//            fprintf(fp, "%.3f\t", (tlc1543_rxbuf[0] * 5.02) / 65536);
//            //qDebug("%.3f\t", (tlc1543_rxbuf[0] * 5.02) / 65536);
//            usleep(WAIT_CONVERSION);
//        }
//        channel = 0;
//        tlc1543_txbuf[0] = (channel<<12)| 0x0c00;//写入要转换的通道号,16位精度(0x0c00),12位精度(0x0000/0x0800),8位精度(0x0400)
//        tlc1543_Transfer(tlc1543_txbuf, tlc1543_rxbuf, 2);//数据交换后得到0通道数据
//        //fprintf(fp, "%d\n", tlc1543_rxbuf[0]);
//        fprintf(fp, "%.3f\n", (tlc1543_rxbuf[0] * 5.02) / 65536);
//        //qDebug("%.3f\n", (tlc1543_rxbuf[0] * 5.02) / 65536);
//        usleep(WAIT_CONVERSION);
//    }

//    gettimeofday(&end, NULL);

//    diff = 1000000 * (end.tv_sec - start.tv_sec) + end.tv_usec - start.tv_usec;

//    printf("diff = %ld us\n",diff);

//    /* 关闭tlc1543 */
//    tlc1543_Close();

//    fclose(fp);
//    sync();

//    return 0;
}

