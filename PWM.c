/*
 * PWM.c
 *
 *  Created on: 2015年1月1日
 *      Author: zhouyu
 *      am33xx_pwm
 *      bone_pwm_P8_13.11
 *      bone_pwm_P8_19.12
 */

#include <PWM.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>

int PWM_Init(PWM_Init_Struct *pwm_Init)
{
	int len;
	char temp_str[60] = {0};
    unsigned int old_period,old_duty;

	/* 参数检查 */
	if(pwm_Init->duty > 2147483647 || pwm_Init->period > 2147483647 || pwm_Init->duty > pwm_Init->period)
	{
		printf("Wrong Init parameter!\n");
		return ERROR;
	}

	/* 获取设备文件指针 */
	sprintf(temp_str, "/sys/devices/ocp.3/%s/run",pwm_Init->name);
	if ((pwm_Init->fd_run = open(temp_str, O_RDWR)) < 0)
	{
		printf("can't open %s.\n",temp_str);
		return ERROR;
	}

	sprintf(temp_str, "/sys/devices/ocp.3/%s/duty",pwm_Init->name);
	if ((pwm_Init->fd_duty = open(temp_str, O_RDWR)) < 0)
	{
		printf("can't open %s.\n",temp_str);
		return ERROR;
	}

	sprintf(temp_str, "/sys/devices/ocp.3/%s/period",pwm_Init->name);
	if ((pwm_Init->fd_period = open(temp_str, O_RDWR)) < 0)
	{
		printf("can't open %s.\n",temp_str);
		return ERROR;
	}

	sprintf(temp_str, "/sys/devices/ocp.3/%s/polarity",pwm_Init->name);
	if ((pwm_Init->fd_polarity = open(temp_str, O_RDWR)) < 0)
	{
		printf("can't open %s.\n",temp_str);
		return ERROR;
	}

    /* 先读取原来的period和duty值 */
    read(pwm_Init->fd_period, temp_str,10);
    old_period = atoi(temp_str);
    read(pwm_Init->fd_duty, temp_str,10);
    old_duty = atoi(temp_str);

	/* echo 0 > run */
	if(1 != write(pwm_Init->fd_run, "0", 1))
	{
		printf("fail:echo 0 > run.\n");
		return ERROR;
	}

    if(pwm_Init->duty > old_period)
    {
        /* echo num > period */
        sprintf(temp_str, "%d",pwm_Init->period);
        len = strlen(temp_str);
        if (len != write(pwm_Init->fd_period, temp_str, len))
        {
            printf("fail:echo %d > period.\n",pwm_Init->period);
            return ERROR;
        }

        /* echo num > duty */
        sprintf(temp_str, "%d",pwm_Init->duty);
        len = strlen(temp_str);
        if (len != write(pwm_Init->fd_duty, temp_str, len))
        {
            printf("fail:echo %d > duty.\n",pwm_Init->duty);
            return ERROR;
        }
    }
    else if(pwm_Init->period < old_duty)
    {
        /* echo num > duty */
        sprintf(temp_str, "%d",pwm_Init->duty);
        len = strlen(temp_str);
        if (len != write(pwm_Init->fd_duty, temp_str, len))
        {
            printf("fail:echo %d > duty.\n",pwm_Init->duty);
            return ERROR;
        }

        /* echo num > period */
        sprintf(temp_str, "%d",pwm_Init->period);
        len = strlen(temp_str);
        if (len != write(pwm_Init->fd_period, temp_str, len))
        {
            printf("fail:echo %d > period.\n",pwm_Init->period);
            return ERROR;
        }
    }
    else
    {
        /* echo num > duty */
        sprintf(temp_str, "%d",pwm_Init->duty);
        len = strlen(temp_str);
        if (len != write(pwm_Init->fd_duty, temp_str, len))
        {
            printf("fail:echo %d > duty.\n",pwm_Init->duty);
            return ERROR;
        }

        /* echo num > period */
        sprintf(temp_str, "%d",pwm_Init->period);
        len = strlen(temp_str);
        if (len != write(pwm_Init->fd_period, temp_str, len))
        {
            printf("fail:echo %d > period.\n",pwm_Init->period);
            return ERROR;
        }
    }


	/* echo 0 > polarity */
    if(pwm_Init->polarity)
    {
        if (1 != write(pwm_Init->fd_polarity, "1", 1))
        {
            printf("fail:echo 1 > polarity.\n");
            return ERROR;
        }
    }
    else
    {
        if (1 != write(pwm_Init->fd_polarity, "0", 1))
        {
            printf("fail:echo 1 > polarity.\n");
            return ERROR;
        }
    }


	/* echo 1 > run */
	if(1 != write(pwm_Init->fd_run, "1", 1))
	{
		printf("fail:echo 1 > run.\n");
		return ERROR;
	}

	return SUCCESS;

}
int set_pwm_duty(PWM_Init_Struct *pwm_Init, unsigned int duty)
{
	int len;
	char temp_str[20];
	/* 参数检查 */
	if(duty > 2147483647 || duty > pwm_Init->period)
	{
		printf("Wrong parameter duty!\n");
		return ERROR;
	}

	/* echo 0 > run */
	if(1 != write(pwm_Init->fd_run, "0", 1))
	{
		printf("fail:echo 0 > run.\n");
		return ERROR;
	}

	/* echo num > duty */
	sprintf(temp_str, "%d",duty);
	len = strlen(temp_str);
	if (len != write(pwm_Init->fd_duty, temp_str, len))
	{
		printf("fail:echo %d > duty.\n",duty);
		return ERROR;
	}

	/* echo 1 > run */
	if(1 != write(pwm_Init->fd_run, "1", 1))
	{
		printf("fail:echo 1 > run.\n");
		return ERROR;
	}

	return SUCCESS;

}
int set_pwm_period(PWM_Init_Struct *pwm_Init, unsigned int period)
{
	int len;
	char temp_str[20];
	/* 参数检查 */
	if(period > 2147483647 || period < pwm_Init->duty)
	{
		printf("Wrong parameter period!\n");
		return ERROR;
	}

	/* echo 0 > run */
	if(1 != write(pwm_Init->fd_run, "0", 1))
	{
		printf("fail:echo 0 > run.\n");
		return ERROR;
	}

	/* echo num > period */
	sprintf(temp_str, "%d",period);
	len = strlen(temp_str);
	if (len != write(pwm_Init->fd_period, temp_str, len))
	{
		printf("fail:echo %d > period.\n",period);
		return ERROR;
	}

	/* echo 1 > run */
	if(1 != write(pwm_Init->fd_run, "1", 1))
	{
		printf("fail:echo 1 > run.\n");
		return ERROR;
	}

	return SUCCESS;
}
int set_pwm_polarity(PWM_Init_Struct *pwm_Init, unsigned int polarity)
{
	char temp_str[5];
	/* 参数检查 */
    if(polarity > 1 )
	{
		printf("Wrong parameter polarity!\n");
		return ERROR;
	}

	/* echo 0 > run */
	if(1 != write(pwm_Init->fd_run, "0", 1))
	{
		printf("fail:echo 0 > run.\n");
		return ERROR;
	}

	/* echo num > polarity */
	sprintf(temp_str, "%d",polarity);
	if (1 != write(pwm_Init->fd_polarity, temp_str, 1))
	{
		printf("fail:echo %d > duty.\n",polarity);
		return ERROR;
	}

	/* echo 1 > run */
	if(1 != write(pwm_Init->fd_run, "1", 1))
	{
		printf("fail:echo 1 > run.\n");
		return ERROR;
	}

	return SUCCESS;
}
void pwm_close(PWM_Init_Struct *pwm_Init)
{
	/* echo 0 > run */
    if(1 != write(pwm_Init->fd_run, "0", 1))
    {
        printf("fail:echo 0 > run.\n");
    }
	close(pwm_Init->fd_polarity);
	close(pwm_Init->fd_period);
	close(pwm_Init->fd_duty);
	close(pwm_Init->fd_run);
}
