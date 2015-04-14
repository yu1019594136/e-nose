/*
 * uart.h
 *
 *  Created on: 2014年10月4日
 *      Author: zhouyu
 */

#ifndef UART_H_
#define UART_H_
#include <common.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define READ_BUFFER_SIZE 50	//定义一次读取数据的最大长度，因为设备文件是字符设备，故不能用lseek来获取准备读取的数据长度,若上位机发过来的数据第一没读取完，那么第二次调用read会把上次剩余的和本次输入的一起读取。
#define uart_num 1			//准备使用的串口号，串口1

void uart_init();//波特率9600 8N1
int uart_send(char *buf, int len);//从串口发送len个字节长度的数据
int uart_receive(char *buf, int len);//从串口读取len个字节长度的数据
void uart_close();//关闭文件指针，但无法真正关闭串口设备
void *thread_read(void *arg);//读线程入口函数

#ifdef __cplusplus
}
#endif

#endif /* UART_H_ */
