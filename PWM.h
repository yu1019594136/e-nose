/*
 * PWM.h
 *
 *  Created on: 2015年1月1日
 *      Author: zhouyu
 */

#ifndef PWM_H_
#define PWM_H_

#include <common.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct {
	char name[20];
	unsigned int duty;
	unsigned int period;
	unsigned int polarity;
	int fd_run;
	int fd_duty;
	int fd_period;
	int fd_polarity;
}PWM_Init_Struct;

/* 注意当/sys/devices/ocp.3/pwm_test_P8_13.11/polarity中的值
为1，表示PWM波为负逻辑，即duty表示一个周期中低电平的持续时间；
为0，表示PWM波为正逻辑，即duty表示一个周期中高电平的持续时间 */

int PWM_Init(PWM_Init_Struct *pwm_Init);
int set_pwm_duty(PWM_Init_Struct *pwm_Init, unsigned int duty);
int set_pwm_period(PWM_Init_Struct *pwm_Init, unsigned int period);
int set_pwm_polarity(PWM_Init_Struct *pwm_Init, unsigned int polarity);
void pwm_close(PWM_Init_Struct *pwm_Init);

#ifdef __cplusplus
}
#endif

#endif /* PWM_H_ */
