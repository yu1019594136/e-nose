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
    int thermo_switch;//加热带电路开关，HIGH? LOW?
    float preset_temp;//预设温度
    int hold_time;//蒸发时间
} THERMOSTAT;

typedef struct{
    int beep_count;//鸣叫次数
    int beep_interval;//鸣叫时间间隔
} BEEP;

typedef struct{
    int pump_switch;//气泵电路开关，HIGH? LOW?
    int pump_duty;//PWM占空比
    int hold_time;//开启时间
} PUMP;

typedef struct{
    int M1;//电磁阀1的状态，下同HIGH? LOW?
    int M2;
    int M3;
    int M4;
} MAGNETIC;

typedef struct{
    float sample_freq;//每个通道的采样频率
    long sample_time;//每个通道的时间长度
    char *filename_prefix;//数据文件名前缀
} SAMPLE;


#ifdef __cplusplus
}
#endif

#endif /* COMMON_H_ */
