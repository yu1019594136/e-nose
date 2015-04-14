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

#include "ui_hellodialog.h"
#include <QApplication>

#include "GPIO.h"
#include "ds18b20.h"
#include "PWM.h"
#include "tlc1543.h"
#include "stm32_spislave.h"
#include "pid.h"
#include "sht21.h"


/* 此处变量已经在ds18b20.c文件中申明和定义， */
extern char *w1_dev1;
//extern char *w1_dev2;

/* SPI通信缓冲区 */
uint16_t tlc1543_txbuf[1] = {0};	//SPI通信发送缓冲区
uint16_t tlc1543_rxbuf[1] = {0};	//SPI通信接收缓冲区
uint16_t channel = 0;		//要转换的通道号，取值0x00-0x0D，
uint16_t count = 10;
float vol = 0.0;

uint32_t period = 0;
float temperature = 0.0;
float humidity = 0.0;
float set_temp = 50;    //温度预期值45度
int d_temp = 0;
int last_duty = 0;

/* 时间函数相关 */
struct tm *tm_ptr;
time_t the_time;
char filename_time[15]={0};

/* 气泵是否关闭 */
int flag_close_airpump = 0;
/* 这个变量已经在pid.h中申明，该变量由用户期望的电机转速计算而来,作为程序中的被控量 */
//extern unsigned int SET_PERIOD;    //根据用户期望的转速而计算出来的电机转速反馈脉冲信号的期望周期
//extern unsigned int SET_DUTY;      //根据用户期望的转速而计算出来的初始时刻PWM应该设置的占空比，可以减少调节时间

/* 定义GPIO结构体以进行初始化,gpio26,gpio22 */
GPIO_Init_Struct gpio60, gpio67,gpio69,gpio45;

void GPIO_Config()
{
    /* 将gpio30设置成输出（默认输出高电平） LDE灯*/
//    gpio60.pin = 60;
//    gpio60.dir = OUTPUT_PIN;
//    GPIO_Init(&gpio60);

//    /* 连接STM32的复位引脚，用于唤醒STM32 */
//    gpio67.pin = 67;
//    gpio67.dir = OUTPUT_PIN;
//    GPIO_Init(&gpio67);

//    /* 连接STM32的外部中断PA11，进入中断函数后stm32开始待机模式 */
//    gpio69.pin = 69;
//    gpio69.dir = OUTPUT_PIN;
//    GPIO_Init(&gpio69);

    /* 连接一个继电器，用于控制气泵电源电路的通断,默认输出低电平，气泵断电,P8_11 */
    gpio45.pin = 45;
    gpio45.dir = OUTPUT_PIN;
    GPIO_Init(&gpio45);

//    /* 将gpio30设置成输出（默认输出高电平） 蜂鸣器，高电平响*/
//    gpio26.pin = 26;
//    gpio26.dir = OUTPUT_PIN;
//    GPIO_Init(&gpio26);
//    gpio_set_value(&gpio26, LOW);

//    /* 将gpio30设置成输出（默认输出高电平） PWM1*/
//    gpio22.pin = 22;
//    gpio22.dir = OUTPUT_PIN;
//    GPIO_Init(&gpio22);

    /* 将gpio31设置成输出 */
//    gpio31.pin = 31;
//    gpio31.dir = OUTPUT_PIN;
//    GPIO_Init(&gpio31);
}

/* 定义PWM结构体以进行初始化 */
PWM_Init_Struct pwm_9_22,pwm_8_13,pwm_9_42;

void PWM_Config()
{
//    strcpy(pwm_9_22.name,"pwm_test_P9_22.12");
//    pwm_9_22.duty = 25000000;
//    pwm_9_22.period = 50000000;
//    pwm_9_22.polarity = 0;

//    if(PWM_Init(&pwm_9_22))
//        printf("pwm_9_22 Init success !\n");
//    else
//        printf("pwm_9_22 Init failed !\n");

    strcpy(pwm_8_13.name,"pwm_test_P8_13.11");
    pwm_8_13.duty = 0;
    pwm_8_13.period = PWM_period_heater;
    pwm_8_13.polarity = 0;

    if(PWM_Init(&pwm_8_13))
        printf("pwm_8_13 Init success !\n");
    else
        printf("pwm_8_13 Init failed !\n");

//    strcpy(pwm_9_42.name,"pwm_test_P9_42.13");
//    pwm_9_42.duty = 2400;
//    pwm_9_42.period = 6000;
//    pwm_9_42.polarity = 0;

//    if(PWM_Init(&pwm_9_42))
//        printf("pwm_9_42 Init success !\n");
//    else
//        printf("pwm_9_42 Init failed !\n");
}

void shutdown()
{
    pid_t new_pid;
    char *const arg_shutdown[] = {"shutdown","-h","now",NULL};

    new_pid = fork();

    if(new_pid < 0)
        printf("fork() failed!\n");
    else if(new_pid == 0)
    {
        printf("This is the child process.\n");
        sleep(5);
        if(execve("/sbin/shutdown", arg_shutdown, NULL) == -1)
        {
            printf("execve() failed!\n");
        }
    }
    else
    {
        printf("Poweroff in 5 seconds!\n");
    }
}

/* 定时事件代码 */
void timefunc(int sig)
{
    flag_close_airpump = 1;

    signal(SIGALRM, timefunc);              /* 捕获定时信号 */
}

int main()
{
/******时间函数相关****************#include <time.h>********/
    (void)time(&the_time);

//    tm_ptr = localtime(&the_time);

//    sprintf(filename_time, "/root/qt_program/temp_test_%d.%d.%d-%d_%d_%d.txt",tm_ptr->tm_year + 1900, tm_ptr->tm_mon + 1, tm_ptr->tm_mday, tm_ptr->tm_hour, tm_ptr->tm_min, tm_ptr->tm_sec);

      /* 创建文件保存数据 */
//    FILE *fp;
//    if((fp=fopen(filename_time,"w"))==NULL)
//    {
//        printf("cannot open file\n");
//        exit(0);
//    }

//    GPIO_Config();

/******采集温度测试****************#include "ds18b20.h"********/

//    pid_Init();

//    PWM_Config();


    printf("DS18B20 TEST!\n");


    //char temp_str[]="00.000";
    //int temp_int = 0;
    //float temp_float = 0.0;

    //DS18B20_Get_TempString(w1_dev1,temp_str);

    //temp_int = DS18B20_Get_TempInt(w1_dev1);

//    for(i=0; i<1000; i++)
//    {
//        temp_float = DS18B20_Get_TempFloat(w1_dev1);

//        printf("temp = %.3f\n",temp_float);

//        fprintf(fp,"%.3f\n",temp_float);

       //DS18B20_Get_TempString(w1_dev1,temp_str);

        //printf("%s\n",temp_str);

//        d_temp =  pid_calculation(set_temp, temp_float);//根据反馈计算控制量(us)

        //last_duty = last_duty + d_temp;

//        if(last_duty < 0)
//            last_duty = 0;
//        else if(last_duty > PWM_period_heater)
//            last_duty = PWM_period_heater;

//        if(d_temp != last_duty)//d_temp为0时，不修改duty
//            set_pwm_duty(&pwm_8_13, d_temp);

//        printf("current duty = %d\n",d_temp);

//        last_duty = d_temp;

//        usleep(500000);
//        //usleep(500000);//每200ms调节一次
//    }

//    DS18B20_Close();

//    printf("采集温度测试结束!\n\n\n");


/******PWM输出测试****************#include "PWM.h"********/

//    printf("PWM输出测试开始!\n");

    /******pid控制相关****************#include <pid.h>********/
//    pid_Init();

    /* 先准备输入的PWM波再来给气泵上电 */
//    PWM_Config();

    //set_pwm_duty(&pwm_8_13, SET_DUTY);

//    usleep(500000);//等待输出波形稳定

//    printf("PWM输出测试结束!\n\n\n");


//    stm32_sleep(&gpio69);
//    gpio_set_value(&gpio60, HIGH);
//    sleep(1);
//    gpio_set_value(&gpio60, LOW);


    /* 气泵上电 */
//    gpio_set_value(&gpio45, HIGH);


/******定时器相关*****#include <sys/time.h>***#include <sys/select.h>*#include <signal.h>****/
//    struct itimerval value;
//    value.it_value.tv_sec = 60; //本次定时时间5秒
//    value.it_value.tv_usec = 0;//0微秒
//    value.it_interval.tv_sec = 0;//下次定时时间5秒
//    value.it_interval.tv_usec = 0;//0微秒

//    signal(SIGALRM, timefunc);          /* 捕获定时信号 */

//    setitimer(ITIMER_REAL, &value, NULL);//定时开始

//    flag_close_airpump = 0;//必须立马归零


//    count = 5000;

    /* 传送非指令，将得到非数据0xf0f0 */
//    stm32_Transfer(stm32_txbuf, stm32_rxbuf, 2);
//    printf("0x%x\n",stm32_rxbuf[0]);

//    while(flag_close_airpump == 0)
//    {
//        getdata_stm32(command, stm32_rxbuf);

//        if(stm32_rxbuf[2] < 0x00ff)//过滤错误数据
//        {
//            period = stm32_rxbuf[2]*65536 + stm32_rxbuf[3];//stm32每8个脉冲捕获一次,捕获单位为us

//            //printf("period = %d\n", period);

//            d_duty =  pid_calculation(SET_PERIOD, period);//根据反馈计算控制量(us)

//            if(d_duty)
//                set_pwm_duty(&pwm_8_13, SET_DUTY + d_duty);//改变控制量，校正偏差(ns)

//            fprintf(fp, "%d\n",period);//保存反馈量(ns)和控制量(ns)以便分析数据
//        }

//        usleep(2000);//每个一段时间校正一次
//    }

    /* 气泵断电 */
//    gpio_set_value(&gpio45, LOW);

//    printf("air pump has been shutdown!\n");

//      GPIO_Close(&gpio45);




/*******采集电压测试***********#include "tlc1543.h"******/
        /* 片选线用gpio31 */
        tlc1543_Init(31);
        printf("采集电池电压测试开始!\n");


        channel = 0;
        tlc1543_txbuf[0] = (channel<<12)| 0x0c00 ;//写入要转换的通道号,16位精度(0x0c00),12位精度(0x0000/0x0800),8位精度(0x0400)
        tlc1543_Transfer(tlc1543_txbuf, tlc1543_rxbuf, 2);//第一个转换数据不准确，丢弃
        usleep(WAIT_CONVERSION);

        while(count--)
        {
            channel = 1;
            tlc1543_txbuf[0] = (channel<<12)| 0x0c00;//写入要转换的通道号,16位精度(0x0c00),12位精度(0x0000/0x0800),8位精度(0x0400)
            tlc1543_Transfer(tlc1543_txbuf, tlc1543_rxbuf, 2);//数据交换后得到0通道数据
            usleep(WAIT_CONVERSION);
            vol = (tlc1543_rxbuf[0]*5.07)/65535;//16位精度
            //vol = ((tlc1543_rxbuf[0])*5.07)/65536;//12位精度
            //vol = ((tlc1543_rxbuf[0]>>8)*5.07)/256;//8位精度
            printf("WSP2110\t= %.3f V\t",vol);

            channel = 2;
            tlc1543_txbuf[0] = (channel<<12)| 0x0c00;//写入要转换的通道号,16位精度(0x0c00),12位精度(0x0000/0x0800),8位精度(0x0400)
            tlc1543_Transfer(tlc1543_txbuf, tlc1543_rxbuf, 2);//数据交换后得到0通道数据
            usleep(WAIT_CONVERSION);
            vol = (tlc1543_rxbuf[0]*5.07)/65535;//16位精度
            printf("MICS-5526 = %.3f V\t",vol);

            channel = 3;
            tlc1543_txbuf[0] = (channel<<12) | 0x0c00;//写入要转换的通道号,16位精度(0x0c00),12位精度(0x0000/0x0800),8位精度(0x0400)
            tlc1543_Transfer(tlc1543_txbuf, tlc1543_rxbuf, 2);//数据交换后得到0通道数据
            usleep(WAIT_CONVERSION);
            vol = (tlc1543_rxbuf[0]*5.07)/65535;//16位精度
            printf("MICS-5521 = %.3f V\t",vol);

            channel = 4;
            tlc1543_txbuf[0] = (channel<<12) | 0x0c00;//写入要转换的通道号,16位精度(0x0c00),12位精度(0x0000/0x0800),8位精度(0x0400)
            tlc1543_Transfer(tlc1543_txbuf, tlc1543_rxbuf, 2);//数据交换后得到0通道数据
            usleep(WAIT_CONVERSION);
            vol = (tlc1543_rxbuf[0]*5.07)/65535;//16位精度
            printf("TGS2611\t= %.3f V\t",vol);

            channel = 5;
            tlc1543_txbuf[0] = (channel<<12) | 0x0c00;//写入要转换的通道号,16位精度(0x0c00),12位精度(0x0000/0x0800),8位精度(0x0400)
            tlc1543_Transfer(tlc1543_txbuf, tlc1543_rxbuf, 2);//数据交换后得到0通道数据
            usleep(WAIT_CONVERSION);
            vol = (tlc1543_rxbuf[0]*5.07)/65535;//16位精度
            printf("TGS2620\t= %.3f V\t",vol);

            channel = 6;
            tlc1543_txbuf[0] = (channel<<12) | 0x0c00;//写入要转换的通道号,16位精度(0x0c00),12位精度(0x0000/0x0800),8位精度(0x0400)
            tlc1543_Transfer(tlc1543_txbuf, tlc1543_rxbuf, 2);//数据交换后得到0通道数据
            usleep(WAIT_CONVERSION);
            vol = (tlc1543_rxbuf[0]*5.07)/65535;//16位精度
            printf("MICS-5121 = %.3f V\t",vol);

            channel = 7;
            tlc1543_txbuf[0] = (channel<<12) | 0x0c00;//写入要转换的通道号,16位精度(0x0c00),12位精度(0x0000/0x0800),8位精度(0x0400)
            tlc1543_Transfer(tlc1543_txbuf, tlc1543_rxbuf, 2);//数据交换后得到0通道数据
            usleep(WAIT_CONVERSION);
            vol = (tlc1543_rxbuf[0]*5.07)/65535;//16位精度
            printf("MICS-5524 = %.3f V\t",vol);

            channel = 8;
            tlc1543_txbuf[0] = (channel<<12) | 0x0c00;//写入要转换的通道号,16位精度(0x0c00),12位精度(0x0000/0x0800),8位精度(0x0400)
            tlc1543_Transfer(tlc1543_txbuf, tlc1543_rxbuf, 2);//数据交换后得到0通道数据
            usleep(WAIT_CONVERSION);
            vol = (tlc1543_rxbuf[0]*5.07)/65535;//16位精度
            printf("TGS880\t= %.3f V\t",vol);

            channel = 9;
            tlc1543_txbuf[0] = (channel<<12) | 0x0c00;//写入要转换的通道号,16位精度(0x0c00),12位精度(0x0000/0x0800),8位精度(0x0400)
            tlc1543_Transfer(tlc1543_txbuf, tlc1543_rxbuf, 2);//数据交换后得到0通道数据
            usleep(WAIT_CONVERSION);
            vol = (tlc1543_rxbuf[0]*5.07)/65535;//16位精度
            printf("MP502\t= %.3f V\t",vol);

            channel = 10;
            tlc1543_txbuf[0] = (channel<<12) | 0x0c00;//写入要转换的通道号,16位精度(0x0c00),12位精度(0x0000/0x0800),8位精度(0x0400)
            tlc1543_Transfer(tlc1543_txbuf, tlc1543_rxbuf, 2);//数据交换后得到0通道数据
            usleep(WAIT_CONVERSION);
            vol = (tlc1543_rxbuf[0]*5.07)/65535;//16位精度
            printf("TGS2602\t= %.3f V\t",vol);

            channel = 0;
            tlc1543_txbuf[0] = (channel<<12) | 0x0c00;//写入要转换的通道号,16位精度(0x0c00),12位精度(0x0000/0x0800),8位精度(0x0400)
            tlc1543_Transfer(tlc1543_txbuf, tlc1543_rxbuf, 2);//数据交换后得到0通道数据
            usleep(WAIT_CONVERSION);
            vol = (tlc1543_rxbuf[0]*5.07)/65535;//16位精度
            printf("Battery\t= %.3f V\n",vol);

        }


//    printf("temp = %.3f\n",sht21_get_temp());
//    printf("humi = %.3f\n",sht21_get_humidity());
//    sht21_close();
      tlc1543_Close();
//     fclose(fp);

//      pwm_close(&pwm_8_13);

//    stm32_sleep(&gpio69);

//     printf("采集电压测试结束!\n\n\n");


    /* 处理采集到的数据 */

//    SHT15_Calculate(stm32_rxbuf[0], stm32_rxbuf[1], &temperature, &humidity);
//    printf("temperature = %2.1f   humidity = %2.1f \n",temperature, humidity);


//    GPIO_Close(&gpio60);
//    GPIO_Close(&gpio67);
//    GPIO_Close(&gpio69);

/******界面输出测试************************/
//    QApplication a(argc, argv);
//    QDialog w;
//    Ui::HelloDialog ui;
//    ui.setupUi(&w);
//    w.show();
//    return a.exec();

//     shutdown();

     return 0;

}

