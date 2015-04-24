/*
 * spidev.c
 *
 *  Created on: 2014年10月5日
 *      Author: zhouyu
 *    refernce: linux-3.16\Documentation\spi\spidev-test.c
 */


#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <spidev.h>
#include <common.h>

/* 形参：spi_fd,用户空间访问SPI设备文件指针；message_len，spi_ioc_transfer结构体数组长度，具体参考spidev.h；spi_ioc_transfer *tr，具体参考spidev.h
 * 返回值：SUCCESS，操作成功；ERROR，操作失败
 * 描述：该函数进行SPI全双工通信，通过系统调用ioctl完成。
 */
int SPI_Transfer(int spi_fd, int message_len, struct spi_ioc_transfer *tr)
{
	int ret;
     ret = ioctl(spi_fd, SPI_IOC_MESSAGE(message_len), tr);
		if (ret < 1)
		{
			printf("can't send spi message");
			return ERROR;
		}
		else
			return SUCCESS;
}
/* 形参：该函数有待测试，，，，
 * 返回值：
 * 描述：
 */
int SPI_Write(int spi_fd, uint8_t *TxBuf, int len)
{
	int ret;
	ret = write(spi_fd, TxBuf, len);
	if (ret < 0)
	{
		printf("write spi error!\n");
		return ERROR;
	}
	else
		return SUCCESS;
}
/* 形参：该函数有待测试，，，，
 * 返回值：
 * 描述：
 */
int SPI_Read(int spi_fd, uint8_t *RxBuf, int len)
{
	int ret;
	ret = read(spi_fd, RxBuf, len);
	if (ret < 0)
	{
		printf("read spi error!\n");
		return ERROR;
	}
	else
		return SUCCESS;
}

