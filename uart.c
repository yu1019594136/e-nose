/*
 * uart.c
 *
 *  Created on: 2014年10月4日
 *      Author: zhouyu
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>		//声明系统调用 sleep
#include <linux/types.h>
#include <fcntl.h>		//声明系统调用open, close, read, write，
#include <uart.h>
#include <pthread.h>

int uart_fd;//串口文件指针，指向设备文件
int res;
pthread_t read_thread;//线程创建
pthread_attr_t thread_attr;//线程属性创建
pthread_mutex_t flag_mutex;//互斥锁，保证对uart_read_data的访问是互斥的,即当子线程调用read将得到的数据写入缓冲区uart_read_data时，用户不能从这个缓冲区读数据，两者不能同时发生
char uart_read_data[READ_BUFFER_SIZE]={0};
int flag_receive_data = 0;//用于指示用户是否有新数据收到

/* 形参：无
 * 返回值：无
 * 描述：串口初始化工作包括，打开串口设备文件，获取文件句柄，同时创建第二个线程，该线程负责从串口读取数据，
 */
void uart_init()
{
	char filename[15];

	/* 打开设备文件，获取文件句柄 */
	sprintf(filename, "/dev/ttyO%d", uart_num);
	if ((uart_fd = open(filename, O_RDWR)) < 0)
	{
		printf("uart /dev/ttyO%d open failed!\n",uart_num);
		exit(EXIT_FAILURE);
	}

	/* 创建一个互斥锁，第二个实参NULL使其属行为fast */
    res = pthread_mutex_init(&flag_mutex, NULL);
    if (res != 0)
    {
        perror("Mutex initialization failed");
        exit(EXIT_FAILURE);
    }

	/* 创建一个线程，用于从串口阻塞读取数据 */
    res = pthread_create(&read_thread, NULL, thread_read, NULL);
    if (res != 0)
    {
        perror("Read thread creation failed");
        exit(EXIT_FAILURE);
    }
}

/* 形参：buf，准备发送给串口的数据缓冲区首地址；len，发送到串口的数据长度
 * 返回值：SUCCESS，发送成功；ERROR，发送失败
 * 描述：发送数据到串口
 */
int uart_send(char *buf, int len)
{
	int count=len;
	if (count == write(uart_fd, buf, len))
		return SUCCESS;
	else
		return ERROR;
}

/* 形参：buf，用于接收来自串口的数据缓冲区首地址；len，指定从串口接收数据的长度
 * 返回值：SUCCESS，发送成功；ERROR，发送失败
 * 描述：从串口接收数据
 */
int uart_receive(char *buf, int len)
{
	int i=0;

	/* 用于查询串口是否收到数据，如果有就读取，如果没有直接返回0 */
	if (flag_receive_data == 0)
		return ERROR;

	/* 从读缓冲区复制指定长度的数据到用户指定的地址，不做地址检查 */
	for(i = 0; i < len; i++)
	{
		buf[i]=uart_read_data[i];
	}

	/* 清除缓冲区，准备存放下一次数据 */
	for(i = 0; i < READ_BUFFER_SIZE; i++)
	{
		uart_read_data[i] = 0;
	}

	/* 每次读取数据后都要将flag_receive_data清0 */
	flag_receive_data = 0;

	return SUCCESS;
}

/* 形参：arg,接收主线程传递过来的指针，该指针可能指向任何类型数据
 * 返回值：void*指针，子线程可以通过这个指针返回一些数据
 * 描述：子线程，负责从串口读数据，并把数据结果保存在全局变量uart_read_data中，该线程在用户调用void uart_close()时会自动取消。
 */
void *thread_read(void *arg)
{
	int res;

    /* 设置线程取消状态为可以取消。NULL表示不关心设置之前的状态 */
    res = pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    if (res != 0)
    {
        perror("Thread pthread_setcancelstate failed");
        exit(EXIT_FAILURE);
    }

    /* 设置取消类型为立即取消，即收到取消请求后，该线程马上取消自己。NULL表示不关心设置之前的状态 */
    res = pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
    if (res != 0)
    {
        perror("Thread pthread_setcanceltype failed");
        exit(EXIT_FAILURE);
    }

    /* 不停地阻塞读取串口数据，直到本线程被取消。用户应该及时使用void uart_receive(void *buf, int len)
     * 把数据读走，否则新收到的数据会覆盖原先按收到的数据 */
    while(1)
    {
    	read(uart_fd, uart_read_data, READ_BUFFER_SIZE);

//     	pthread_mutex_lock(&flag_mutex);

    	/*将此标志置1，以通知int uart_receive(void *buf, int len)收到数据，注意，此标志也由该函数uart_receive来清0，无需用户处理 */
    	flag_receive_data = 1;

//    	pthread_mutex_unlock(&flag_mutex);
    }
    /*该线程最后是被取消，所以不会运行到return这一句*/
    return NULL;
}

/* 形参：无
 * 返回值：无
 * 描述：向子线程发送取消请求，以结束子线程，另外关闭串口文件句柄，但无法真正关闭串口设备
 */
void uart_close()
{
	/* 取消读取数据的线程 */
    res = pthread_cancel(read_thread);
    if (res != 0)
    {
        perror("Read thread cancelation failed");
        exit(EXIT_FAILURE);
    }

    /* 关闭文件指针，但无法真正关闭串口设备 */
	close(uart_fd);
}

