/*
 * common.h
 *
 *  Created on: 2014年10月4日
 *      Author: zhouyu
 */

#ifndef COMMON_H_
#define COMMON_H_

#ifdef __cplusplus
extern "C"
{
#endif

/* 函数调用的返回结果 */
enum RESULT{
    ERROR = 0,
    SUCCESS
};

/* 线程之间用于传递信息的结构体 */
enum SWITCH{
    CLOSE = 0,
    OPEN
};

enum THERMO{
    STOP = 0,
    START
};

typedef struct{
    int thermo_switch;
    float preset_temp;
} THERMOSTAT;

#ifdef __cplusplus
}
#endif

#endif /* COMMON_H_ */
