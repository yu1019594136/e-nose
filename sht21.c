#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include "sht21.h"

int fd_sht21_humidity1_input = 0;//设备文件
int fd_sht21_temp1_input = 0;//设备文件

float sht21_get_humidity_float()
{
    char temp_str[]="00000";

    if((fd_sht21_humidity1_input = open(SHT21_HUMIDITY_FILEPATH,O_RDONLY))<0)
    {
        printf("open %s failed !\n",SHT21_HUMIDITY_FILEPATH);
    }

    read(fd_sht21_humidity1_input, temp_str, 5);

    close(fd_sht21_humidity1_input);

    return (atof(temp_str))/1000;

}
float sht21_get_temp_float()
{
    char temp_str[]="00000";

    if((fd_sht21_temp1_input = open(SHT21_TEMPERTU_FILEPATH,O_RDONLY))<0)
    {
        printf("open %s failed !\n",SHT21_TEMPERTU_FILEPATH);
    }

    read(fd_sht21_temp1_input, temp_str, 5);

    close(fd_sht21_temp1_input);

    return (atof(temp_str))/1000;
}


void sht21_get_humidity_string(char *humid_str)
{
    int i;
    if((fd_sht21_humidity1_input = open(SHT21_HUMIDITY_FILEPATH,O_RDONLY))<0)
    {
        printf("open %s failed !\n",SHT21_HUMIDITY_FILEPATH);
    }

    read(fd_sht21_humidity1_input, humid_str, 5);

    for(i = 4; i > 1; i--)
    {
        humid_str[i+1] = humid_str[i];
    }
    humid_str[2] = '.';	//加入小数点

    close(fd_sht21_humidity1_input);
}

void sht21_get_temp_string(char *temp_str)
{
    int i;
    if((fd_sht21_temp1_input = open(SHT21_TEMPERTU_FILEPATH,O_RDONLY))<0)
    {
        printf("open %s failed !\n",SHT21_TEMPERTU_FILEPATH);
    }

    read(fd_sht21_temp1_input, temp_str, 5);

    for(i = 4; i > 1; i--)
    {
        temp_str[i+1] = temp_str[i];
    }
    temp_str[2] = '.';	//加入小数点

    close(fd_sht21_temp1_input);
}

