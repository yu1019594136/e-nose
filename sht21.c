#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include "sht21.h"

int fd_sht21_humidity1_input = 0;//设备文件
int fd_sht21_temp1_input = 0;//设备文件

void sht21_init()
{
    if((fd_sht21_humidity1_input = open(SHT21_HUMIDITY_FILEPATH,O_RDONLY))<0)
    {
        printf("open %s failed !\n",SHT21_HUMIDITY_FILEPATH);
    }

    if((fd_sht21_temp1_input = open(SHT21_TEMPERTU_FILEPATH,O_RDONLY))<0)
    {
        printf("open %s failed !\n",SHT21_TEMPERTU_FILEPATH);
    }
}

float sht21_get_humidity()
{
    char temp_str[]="00000";

    read(fd_sht21_humidity1_input, temp_str, 5);

    return (atof(temp_str))/1000;

}
float sht21_get_temp()
{
    char temp_str[]="00000";

    read(fd_sht21_temp1_input, temp_str, 5);

    return (atof(temp_str))/1000;
}

void sht21_close()
{
    close(fd_sht21_humidity1_input);
    close(fd_sht21_temp1_input);
}
