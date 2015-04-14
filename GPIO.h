/*
 * GPIO.h
 *
 *  Created on: 2014年9月24日
 *      Author: zhouyu
 */

#ifndef GPIO_H_
#define GPIO_H_

#include <common.h>

#ifdef __cplusplus
extern "C"
{
#endif

enum PIN_DIRECTION{
	INPUT_PIN = 0,
	OUTPUT_PIN
};

enum PIN_VALUE{
	LOW = 0,
	HIGH
};

typedef struct {
	unsigned int pin;
	int fd_value;
	int dir;
} GPIO_Init_Struct;

int GPIO_Init(GPIO_Init_Struct *GPIO_Init);
int GPIO_Close(GPIO_Init_Struct *GPIO_Init);
int gpio_set_dir(GPIO_Init_Struct *GPIO_Init, int dir);
int gpio_set_value(GPIO_Init_Struct *GPIO_Init, int value);
int gpio_get_value(GPIO_Init_Struct *GPIO_Init, int *value);

#ifdef __cplusplus
}
#endif

#endif /* GPIO_H_ */
