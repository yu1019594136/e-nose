/*
 * GPIO.c
 *
 *  Created on: 2014年9月24日
 *      Author: zhouyu
 */

#include <GPIO.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <malloc.h>

/* 形参：GPIO_Init，该结构体指针指向的内存，包含了初始化一个GPIO口所需的信息，由用户传递过来
 * 返回值：SUCCESS，初始化成功；ERROR，初始化失败
 * 描述：初始化指定引脚标号的GPIO口，输入口还是输出口，并保存指向/sys/class/gpio/gpio引脚号/value的文件句柄，
 * 		以便于用作输出口时，可以方便的输出
 */
int GPIO_Init(GPIO_Init_Struct *GPIO_Init)
{
	int fd;
	char temp_str[50]={0};

	/* 参数检查，gpio的值最大128 GPIO_Direction只能为输入或者输出 */
	if(GPIO_Init->pin > 128 || (GPIO_Init->dir != OUTPUT_PIN && GPIO_Init->dir != INPUT_PIN))
	{
		printf("the wrong parameter: gpio_pin > 128 or wrong direction ！\n");
		return ERROR;
	}

	/* step1、向export导出GPIO口 */
	if((fd=open("/sys/class/gpio/export",O_WRONLY))<0)
	{
		printf("open /sys/class/gpio/export failed !\n");
		return ERROR;
	}
	sprintf(temp_str, "%d", GPIO_Init->pin);//将整形参数转换成字符串

	write(fd, temp_str, strlen(temp_str));

	close(fd);

	/* step2、设置GPIO口为输入输出  */
	sprintf(temp_str, "/sys/class/gpio/gpio%d/direction", GPIO_Init->pin);//将整形参数转换成字符串
	if((fd=open(temp_str,O_WRONLY))<0)
	{
		printf("open %s failed !\n", temp_str);
		return ERROR;
	}

	if (GPIO_Init->dir == INPUT_PIN)
	{
		write(fd, "in", 2);
		/* step3、由于每次读取gpio文件都要重新打开文件，以使得读取前文件指针每次都是在文件最开始位置，所以当用作输入口时，文件指针没必要保存 */
		/*sprintf(temp_str, "/sys/class/gpio/gpio%d/value", GPIO_Init->pin);//将整形参数转换成字符串
		if((GPIO_Init->fd_value = open(temp_str,O_RDONLY))<0)
		{
			printf("open %s failed !\n", temp_str);
			return ERROR;
		}*/
	}
	else if(GPIO_Init->dir == OUTPUT_PIN)
	{
		write(fd, "out", 3);
//		/* step3、获取GPIO口的value文件句柄用于设置IO口,写方式文件句柄 */
//		sprintf(temp_str, "/sys/class/gpio/gpio%d/value", GPIO_Init->pin);//将整形参数转换成字符串
//		if((GPIO_Init->fd_value = open(temp_str, O_WRONLY))<0)
//		{
//			printf("open %s failed !\n", temp_str);
//			return ERROR;
//		}
//		write(GPIO_Init->fd_value, "1", 1);//如果配置为输出口，则默认输出高电平
	}
	else
	{
		printf("the wrong parameter: dir = INPUT_PIN or OUTPUT_PIN ?\n");
		return ERROR;
	}

	/* 4、关闭指向direction的文件句柄 */
	close(fd);

	return SUCCESS;
}

/* 形参：GPIO_Init，该结构体指针指向的内存，包含了关闭一个GPIO口所需的信息
 * 返回值：SUCCESS，关闭成功；ERROR，关闭化失败
 * 描述：关闭一个GPIO口需要先关闭value文件句柄，再向unexport写入该引脚号以让系统关闭该GPIO口设备
 *
 */
int GPIO_Close(GPIO_Init_Struct *GPIO_Init)
{
	int fd;
	char temp_str[50]={0};

	/* 1、如果时输出口则要关闭value文件句柄*/
//	if(GPIO_Init->dir == OUTPUT_PIN)
//	close(GPIO_Init->fd_value);

	/* 2、撤销GPIO口的导出*/
	if((fd=open("/sys/class/gpio/unexport",O_WRONLY))<0)
	{
		printf("open /sys/class/gpio/unexport failed !\n");
		return ERROR;
	}
	sprintf(temp_str, "%d", GPIO_Init->pin);//将整形参数转换成字符串

	write(fd, temp_str, strlen(temp_str));

	close(fd);

	return SUCCESS;
}

/* 形参：GPIO_Init，该结构体指针指向的内存，包含了关闭一个GPIO口所需的信息；dir，指示引脚方向该参数取值为INPUT_PIN或者OUTPUT_PIN
 * 返回值：SUCCESS，操作成功；ERROR，操作失败
 * 描述：若dir为INPUT_PIN，则该引脚设置成输入脚
 *		若dir为OUTPUT_PIN，则该引脚设置成输出脚
 */
int gpio_set_dir(GPIO_Init_Struct *GPIO_Init, int dir)
{
	int fd=0;
	char temp_str[50]={0};

	/* 如果已经是想要设置的状态，则立即退出 */
	if(GPIO_Init->dir == dir)
		return SUCCESS;

	/* 先关闭之前的fd_value文件句柄，因为输入和输出口的文件句柄属性不同，分别为只读和只写！ */
//	if(GPIO_Init->dir == OUTPUT_PIN)
//	close(GPIO_Init->fd_value);

	/* 2、根据direction_flag重新设置输入输出口 */
	sprintf(temp_str, "/sys/class/gpio/gpio%d/direction", GPIO_Init->pin);//将整形参数转换成字符串
	if((fd=open(temp_str,O_WRONLY))<0)
	{
		printf("open %s failed !\n", temp_str);
		return ERROR;
	}

	/* 更新结构体的引脚输入输出状态 */
	GPIO_Init->dir = dir;

	if (GPIO_Init->dir == INPUT_PIN)
	{
		write(fd, "in", 2);

//		/* step3、获取GPIO口的value文件句柄用于读取IO口,读方式文件句柄 */
//		sprintf(temp_str, "/sys/class/gpio/gpio%d/value", GPIO_Init->pin);//将整形参数转换成字符串
//		if((GPIO_Init->fd_value=open(temp_str,O_RDONLY))<0)
//		{
//			printf("open %s failed !\n", temp_str);
//			return ERROR;
//		}
	}
	else if(GPIO_Init->dir == OUTPUT_PIN)
	{
		write(fd, "out", 3);

//		/* step3、获取GPIO口的value文件句柄用于设置IO口,写方式文件句柄 */
//		sprintf(temp_str, "/sys/class/gpio/gpio%d/value", GPIO_Init->pin);//将整形参数转换成字符串
//		if((GPIO_Init->fd_value=open(temp_str,O_WRONLY))<0)
//		{
//			printf("open %s failed !\n", temp_str);
//			return ERROR;
//		}
	}
	else
	{
		printf("the wrong parameter: dir = INPUT_PIN or OUTPUT_PIN ?\n");
		return ERROR;
	}

	/* 4、关闭指向direction的文件句柄 */
	close(fd);

	return SUCCESS;
}

/* 形参：GPIO_Init，该结构体指针指向的内存，包含了一个GPIO口所需的信息；value，该参数取值可以为LOW或者HIGH
 * 返回值：SUCCESS，操作成功；ERROR，操作失败
 * 描述：若dir == INPUT_PIN，且value取值为LOW，则输出低电平
 *		若dir == INPUT_PIN，且value取值为HIGH，则输出高电平
 */
/* 注意：函数做成输入口不可以设置其值（输入口的value文件句柄属性为只读，不能写入，强行写入会失败） */
int gpio_set_value(GPIO_Init_Struct *GPIO_Init, int value)
{
	char temp_str[50]={0};
	if ( GPIO_Init->dir == INPUT_PIN)
	{
        printf("you can not set a input pin! pin = %d.\n",GPIO_Init->pin);
		return ERROR;
	}

	sprintf(temp_str, "/sys/class/gpio/gpio%d/value", GPIO_Init->pin);//将整形参数转换成字符串
	if((GPIO_Init->fd_value = open(temp_str, O_WRONLY))<0)
	{
		printf("open %s failed !\n", temp_str);
		return ERROR;
	}
	if (value == LOW)
	{
		write(GPIO_Init->fd_value, "0", 1);
	}
	else if(value == HIGH)
	{
		write(GPIO_Init->fd_value, "1", 1);
	}
	else
	{
		printf("the wrong parameter: value = LOW or HIGH ?\n");
		return ERROR;
	}

	close(GPIO_Init->fd_value);

	return SUCCESS;
}

/* 形参：GPIO_Init，该结构体指针指向的内存，包含了一个GPIO口所需的信息；*value，函数读取后的结果，存放在用户指定的这个地址中
 * 返回值：SUCCESS，操作成功；ERROR，操作失败
 * 描述：该函数用于读取输入口GPIO的引脚电平，该函数做成输出口不可以读取其值（输出口的value文件句柄属性为只写，不能读取，强行读取会失败），虽然事实上输出口也可以读取
 *
 */
int gpio_get_value(GPIO_Init_Struct *GPIO_Init, int *value)
{
	char temp;
	char temp_str[50]={0};

	if ( GPIO_Init->dir == OUTPUT_PIN)
	{
        printf("you can not read a output pin! pin = %d.\n",GPIO_Init->pin);
		return ERROR;
	}

	sprintf(temp_str, "/sys/class/gpio/gpio%d/value", GPIO_Init->pin);//将整形参数转换成字符串
	if((GPIO_Init->fd_value=open(temp_str,O_RDONLY))<0)
	{
		printf("open %s failed !\n", temp_str);
		return ERROR;
	}

	read(GPIO_Init->fd_value, &temp,1);

	close(GPIO_Init->fd_value);

	if(temp == '1')
		*value = 1;
	else
		*value = 0;
	return SUCCESS;
}
