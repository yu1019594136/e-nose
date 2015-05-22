#include <pid.h>
#include <stdio.h>
#include <stdlib.h>         //abs()

float Kp = 0.0;         //PID调节的比例常数,比例项系数
float Ti = 0.005;       //PID调节的积分常数
float Td = 0.005;       //PID调节的微分常数
float dt = 0.001;       //采样周期,dt

float Ki = 0.0;         //积分项系数
float Kd = 0.0;         //微分项系数

float error = 0.0;              //偏差e[k]
float last_error = 0.0;         //偏差e[k-1]
float sum_error = 0.0;          //偏差累加和

float uk = 0.0;         //u[k]

THERMOSTATIC Thermostatic[8];

void pid_Init(void)
{
    Kp = 0;
    Ki = 0;//(Kp * dt) / Ti
    Kd = 0;//(Kp * Td) / dt
//    SET_PERIOD = 10000000 / SET_SPEED;     //根据用户期望的转速而计算出来的电机转速反馈脉冲信号的期望周期(us),作为PID控制器的被控量
//    SET_DUTY = (unsigned int)(0.3 * PWM_period);    //根据用户期望的转速而计算出来的初始时刻PWM应该设置的占空比(ns)，可以减少调节时间
//    printf("SET_PERIOD = %d us\tSET_DUTY = %d ns\n",SET_PERIOD, SET_DUTY);

    /* 载入所有相关的恒温控制参数 */
    Thermostatic[0].preset_temp = 35.0;
    Thermostatic[0].borderline[0] = 4000000;
    Thermostatic[0].borderline[1] = 4000000;
    Thermostatic[0].borderline[2] = 4000000;
    Thermostatic[0].borderline[3] = 2000000;//1000000
    Thermostatic[0].borderline[4] = 800000;//500000

    Thermostatic[1].preset_temp = 45.0;
    Thermostatic[1].borderline[0] = 800000;
    Thermostatic[1].borderline[1] = 800000;
    Thermostatic[1].borderline[2] = 800000;
    Thermostatic[1].borderline[3] = 800000;
    Thermostatic[1].borderline[4] = 800000;

    Thermostatic[2].preset_temp = 50.0;
    Thermostatic[2].borderline[0] = 2000000;
    Thermostatic[2].borderline[1] = 2000000;
    Thermostatic[2].borderline[2] = 2000000;
    Thermostatic[2].borderline[3] = 2000000;
    Thermostatic[2].borderline[4] = 1000000;

    Thermostatic[3].preset_temp = 55.0;
    Thermostatic[3].borderline[0] = 4000000;
    Thermostatic[3].borderline[1] = 4000000;
    Thermostatic[3].borderline[2] = 4000000;
    Thermostatic[3].borderline[3] = 2000000;
    Thermostatic[3].borderline[4] = 1000000;

    Thermostatic[4].preset_temp = 60.0;
    Thermostatic[4].borderline[0] = 6000000;
    Thermostatic[4].borderline[1] = 6000000;
    Thermostatic[4].borderline[2] = 5000000;
    Thermostatic[4].borderline[3] = 3000000;
    Thermostatic[4].borderline[4] = 1500000;

    Thermostatic[5].preset_temp = 65.0;
    Thermostatic[5].borderline[0] = 7000000;
    Thermostatic[5].borderline[1] = 5000000;
    Thermostatic[5].borderline[2] = 4000000;
    Thermostatic[5].borderline[3] = 3000000;
    Thermostatic[5].borderline[4] = 2500000;

    Thermostatic[6].preset_temp = 70.0;
    Thermostatic[6].borderline[0] = 6000000;
    Thermostatic[6].borderline[1] = 4000000;
    Thermostatic[6].borderline[2] = 3000000;
    Thermostatic[6].borderline[3] = 2500000;
    Thermostatic[6].borderline[4] = 2500000;

    Thermostatic[7].preset_temp = 75.0;
    Thermostatic[7].borderline[0] = 5000000;
    Thermostatic[7].borderline[1] = 3000000;
    Thermostatic[7].borderline[2] = 2500000;
    Thermostatic[7].borderline[3] = 2500000;
    Thermostatic[7].borderline[4] = 2500000;
}

int pid_calculation(THERMOSTATIC *thermostatic, float Feedbackvalue)
{
    int adjust = 0;  //调节器输出调整量
    //float abs_error = 0.0;
    //float d_error = 0.0;

    error = thermostatic->preset_temp - Feedbackvalue;

//    printf("error = %.2f\n", error);

//    sum_error += error;

//    d_error = error - last_error;

//    last_error = error;//准备下一次pid计算的last_error数据

//    abs_error = abs(error);//求误差绝对值

    if( error > BORDERLINE0)
    {
        adjust = thermostatic->borderline[0];//超出误差限将不会进行调节
    }
    else if(error > BORDERLINE1)
    {
        adjust = thermostatic->borderline[1];
    }
    else if(error > BORDERLINE2)
    {
        adjust = thermostatic->borderline[2];
    }
    else if(error > BORDERLINE3)
    {
        adjust = thermostatic->borderline[3];
    }
    else if(error > BORDERLINE4)
    {
        adjust = thermostatic->borderline[4];
    }
    else
    {
        adjust = 0;
    }

    return adjust;
}
