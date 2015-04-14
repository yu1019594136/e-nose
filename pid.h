#ifndef PID_H
#define PID_H

/*
   面向用户时，被控对象是电机转速r/min，但是在程序里面，用户期望
的转速会被转换成预期的电机转速脉冲反馈信号的周期，该周期作为真正的
被控对象.

电机转速 = C（r/min）
每秒钟电机反馈的脉冲个数 = （C/60）*6 = C/10 (个)
电机反馈的脉冲脉冲信号的频率和周期分别为 f = C/10 (Hz), T = 10/C (s)

stm32的PA1引脚用于捕获电机的反馈脉冲，对于输入的脉冲信号进行8分频，即stm32每8个脉冲计时为一个大周期
故stm32捕获到的周期为 80/C (s) = （8*10^7）/C (us)
那么原电机反馈的脉冲信号的周期为 T = 10^7/ C (us)


最后转速转换成脉冲信号周期的的公式:
T = 10^7 / C

T单位us，C单位r/min

*/

#ifdef __cplusplus
extern "C"
{
#endif

//#define SET_SPEED   800              //单位r/min
#define PWM_period_heater 10000000   //加热带PWM波周期为0.5s = 500000000 ns
#define PWM_period_airpump 125000      //气泵电机控制PWM周期为125us = 125000 ns
//#define ERROR_upper_limit   3500     //
#define ERROR_lower_limit   0.5         //温度误差绝对值下限,1摄氏度左右的温度波动都在允许范围内
//#define SPPED_upper_limit   2700    //单位r/min,电机全速运行时，只能在这个速度左右
//#define SPPED_lower_limit   800     //单位r/min,运行太慢气流很微弱

typedef struct
{
    float preset_temp;
    unsigned long borderline[5];
} THERMOSTATIC;

enum BORDERLINE{
    BORDERLINE4 = 0,
    BORDERLINE3 = 10,
    BORDERLINE2 = 15,
    BORDERLINE1 = 20,
    BORDERLINE0 = 25
};

void pid_Init(void);
int pid_calculation(THERMOSTATIC *thermostatic, float Feedbackvalue);


#ifdef __cplusplus
}
#endif

#endif // PID_H
