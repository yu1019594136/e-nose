/*
 * i2c.c
 *
 *  Created on: 2014年9月23日
 *      Author: zhouyu
 */
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>		//声明系统调用open, close, read, write， sleep
#include <i2c.h>

int i2c_fd;				//用于保存文件句柄
int i2cbus_num=1;		//1:the slave device is on the bus i2c-1; 0:the slave device is on the bus i2c-0

/* 形惨：i2c总线号，设备是在总线0(i2c-0)还是总线1(i2c-1)上
 * 返回值：打开设备文件后的到的设备文件句柄
 * 作用：创建设备节点，并获得文件句柄。
 */
void i2cOpen()
{
	char filename[15];				//name of the device file
	snprintf(filename,15,"/dev/i2c-%d",i2cbus_num);
	i2c_fd = open(filename, O_RDWR);
    if (i2c_fd < 0)
	{
		printf("i2cOpen failed !\n");
		exit(EXIT_FAILURE);
	}
}

/* 形参：无
 * 返回值：无
 * 作用：关闭文件句柄指向的设备
 */
void i2cClose()
{
	close(i2c_fd);
}

/* 形参：欲要进行通信的从设备地址
 * 返回值：无
 * 作用：
 */
void i2cSetAddress(int address)
{
	if (ioctl(i2c_fd, I2C_SLAVE, address) < 0)
	{
		printf("i2cSetAddress failed !\n");
		exit(EXIT_FAILURE);
	}
}
/* 形参：要读取的寄存器地址，读取之后要存放数据的缓冲区，要读取的字节个数
 * 返回值：读取到的字节个数
 * 作用：从某个地址开始连续读取len个字节数据
 */
int Readi2c(char REG_Address, char *buf, int len)
{
	int temp=0;
	char addr=REG_Address;
	write(i2c_fd, &addr, 1);//将文件读取位置定位到REG_Address处
	temp=read(i2c_fd, buf, len);
	return temp;
}
/* 形参：要写入的寄存器地址，要写入该寄存器的数据
 * 返回值：无
 * 作用：一般用于配置从设备
 */
void Writei2c(char REG_Address,char REG_data)
{
	char temp[2]={REG_Address,REG_data};
	if(write(i2c_fd, temp, 2)!=2)
		printf("Writei2c failed !\n");
}
