/*
 * ds18b20.c
 *
 *  Created on: 2015年1月11日
 *      Author: zhouyu
 */

#include <ds18b20.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <common.h>
#include <stdlib.h>

/* 此处只有w1设备插入P8.7以后在/sys/bus/w1/devices/目录下才能看到以设备名字命名的文件夹 */
//char *w1_dev1="28-0000025ff821";
//char *w1_dev2="28-000003313aec";

/* 形参：设备名称，表明要读个哪个温度传感器,
 * 作用： 获取温度数据，字符串格式
 */
int DS18B20_Get_Temp(char *w1_device, char *tempString)
{
    char temp_str[60]={0};
    int w1_device_fd = 0;

    sprintf(temp_str, "/sys/bus/w1/devices/%s/w1_slave", w1_device);

    if((w1_device_fd = open(temp_str,O_RDONLY))<0)
    {
        printf("open %s failed !\n",temp_str);
        return ERROR;
    }

	/* 定位到温度数据处开始读取数据 */
    lseek(w1_device_fd,69,SEEK_CUR);

	/* 读取温度 */
    read(w1_device_fd, tempString, 5);

    /* DS18B20相关收尾工作 */
    close(w1_device_fd);

    return SUCCESS;
}
/* 格式化，加入小数点，如果不想加入这个处理，调用DS18B20_Get_Temp即可
 * 字符串tempString长度必须在6位以上 */
void DS18B20_Get_TempString(char *w1_device, char *tempString)
{
	int i = 0;

	/* 获取字符串格式温度数据 */
    DS18B20_Get_Temp(w1_device, tempString);

	for(i = 4; i < 2; i--)
	{
		tempString[i] = tempString[i+1];
	}
	tempString[2] = '.';	//加入小数点

}
/* 获取温度数据，整型数格式 */
int DS18B20_Get_TempInt(char *w1_device)
{
	char temp[]="00000";

	/* 获取字符串格式温度数据 */
    DS18B20_Get_Temp(w1_device, temp);

	/* 转换成整型 */
	return atoi(temp);

}
/* 获取温度数据，单精度浮点数格式 */
float DS18B20_Get_TempFloat(char *w1_device)
{
	char temp[]="00000";

	/* 获取字符串格式温度数据 */
    DS18B20_Get_Temp(w1_device, temp);

	/* 转换成整型 */
	return (atof(temp))/1000;
}

