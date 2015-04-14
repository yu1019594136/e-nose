#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/time.h>
#include <signal.h>
#include <error.h>

#include <QApplication>
#include <QDebug>
#include "mainwindow.h"

#include "HW_interface.h"
#include "sht21.h"
#include "ds18b20.h"
#include "pid.h"
#include "PWM.h"

extern PWM_Init_Struct pwm_9_42_zhenfashi;//该PWM输出用于控制蒸发室加热带

extern THERMOSTATIC Thermostatic_room_temp_35;//室温恒温到35摄氏度的相关参数,下同
extern THERMOSTATIC Thermostatic_35_45;//35摄氏度恒温到45摄氏度的相关参数,下同
extern THERMOSTATIC Thermostatic_35_50;
extern THERMOSTATIC Thermostatic_35_55;
extern THERMOSTATIC Thermostatic_35_60;
extern THERMOSTATIC Thermostatic_35_65;
extern THERMOSTATIC Thermostatic_35_70;
extern THERMOSTATIC Thermostatic_35_75;

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    MainWindow mainwindow;
    mainwindow.showFullScreen();
    return app.exec();

//    float feedback_quantity = 0.0;//反馈量是ds18b20传感器温度
//    int controlled_quantity = 0;//控制量是PWM的duty
//    int last_duty = 0;
//    unsigned int count = 700;

//    while(count--)
//    {
//        feedback_quantity = DS18B20_Get_TempFloat(w1_dev1);

//        printf("temp = %.3f\n",feedback_quantity);

//        controlled_quantity =  pid_calculation(&Thermostatic_room_temp_35, feedback_quantity);

//        if(controlled_quantity != last_duty)//d_temp为0时，不修改duty
//            set_pwm_duty(&pwm_9_42_zhenfashi, controlled_quantity);

//        printf("current duty = %d\n",controlled_quantity);

//        last_duty = controlled_quantity;

//        usleep(500000);
//    }
//     return 0;
}

