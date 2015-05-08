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

enum PIN_DIRECTION{
    INPUT_PIN = 0,
    OUTPUT_PIN
};

enum PIN_VALUE{
    LOW = 0,
    HIGH
};

/* 函数调用的返回结果 */
enum RESULT{
    ERROR = 0,
    SUCCESS
};

/* 线程之间用于传递信息的结构体 */
enum SWITCH{
    CLOSE = 0,
    OPEN = 1,
};

enum THERMO{
    STOP = 0,
    START
};

typedef struct{
    int beep_count;//鸣叫次数
    int beep_interval;//鸣叫时间间隔
} BEEP;

typedef struct{
    int pump_switch;//气泵电路开关，HIGH? LOW?
    int pump_duty;//PWM占空比
    //int hold_time;//开启时间
} PUMP;

typedef struct{
    int M1;//电磁阀1的状态，下同HIGH? LOW?
    int M2;
    int M3;
    int M4;
} MAGNETIC;

typedef struct{
    long width;
    long height;
    unsigned long sample_count_real;//实际操作的采样次数
} PLOT_INFO;

#ifdef __cplusplus
}
#endif

#endif /* COMMON_H_ */
