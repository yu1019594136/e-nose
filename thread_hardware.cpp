#include "thread_hardware.h"
#include <QDebug>

/* 硬件相关接口 */
#include "HW_interface.h"
#include "ds18b20.h"
#include "sht21.h"
#include "pid.h"
#include "PWM.h"

/* 硬件相关变量 */
extern THERMOSTATIC Thermostatic[8];
extern PWM_Init_Struct pwm_8_13_airpump;//该PWM输出用于控制气泵转速
extern PWM_Init_Struct pwm_9_22_fanyingshi;//该PWM输出用于控制反应室加热带
extern PWM_Init_Struct pwm_9_42_zhenfashi;//该PWM输出用于控制蒸发室加热带

/*********************硬件控制线程*****************************/
HardWareControlThread::HardWareControlThread(QObject *parent) :
    QThread(parent)
{
    /* 构造函数数据成员初始化 */
    thermostat.thermo_switch = STOP;
    thermostat.preset_temp = 35.0;

    /* 实例化一个定时器用于蜂鸣器 */
    beep_timer = new QTimer();
    connect(beep_timer, SIGNAL(timeout()), this, SLOT(beep_timeout()));

    stopped = false;
}

void HardWareControlThread::run()
{
    GUI_REALTIME_INFO realtime_info;
    float temp_float = 0.0;
    char temp_str[]="00.000";
    int duty = 0;
    int last_duty = 0;
    PARA_NUM para_num = temp_unset;//缺省值
    int flag_inform_duty_0 = 0;

    /* 上电后系统初始化各个硬件电路，配置操作系统状态，等待用户操作 */
    init_hardware();

    while (!stopped)
    {
        /* 不断采集蒸发室和反应室温度以及湿度数据，发送给GUI线程进行数据更新 */
        temp_float = DS18B20_Get_TempFloat("28-0000025ff821");
        realtime_info.ds18b20_temp = QString::number(temp_float, 'f', 3);//将数字的浮点数转化成字符串，精度小数点后3位

        sht21_get_temp_string(temp_str);
        realtime_info.sht21_temp = temp_str;

        sht21_get_humidity_string(temp_str);
        realtime_info.sht21_humid = temp_str;

        emit send_to_GUI_realtime_info_update(realtime_info);

        /* 恒温控制 */
        if(thermostat.thermo_switch == STOP)//停止恒温
        {
            para_num = temp_unset;//恢复缺省值
            Heat_S_Switch(CLOSE);//关闭加热电路开关
            flag_inform_duty_0 = 0;
            set_pwm_duty(&pwm_9_22_fanyingshi, 0);//
            emit send_to_GUI_duty_update(0);
        }
        else if(thermostat.thermo_switch == START)//开始恒温
        {
            if(para_num == temp_unset)
            {
                if(thermostat.preset_temp > 70)
                    para_num = temp_35_75;
                else if(thermostat.preset_temp > 65)
                    para_num = temp_35_70;
                else if(thermostat.preset_temp > 60)
                    para_num = temp_35_65;
                else if(thermostat.preset_temp > 55)
                    para_num = temp_35_60;
                else if(thermostat.preset_temp > 50)
                    para_num = temp_35_55;
                else if(thermostat.preset_temp > 45)
                    para_num = temp_35_50;
                else if(thermostat.preset_temp > 40)
                    para_num = temp_35_45;
                else
                    para_num = temp_room_35;

                /* 打开加热电路开关 */
                Heat_S_Switch(OPEN);

                /* 达到预期温度后通知逻辑线程 */
                flag_inform_duty_0 = 0;
            }

            duty = pid_calculation(&Thermostatic[para_num], temp_float);

            /* 达到预期温度后通知逻辑线程 */
            if(duty == 0 && flag_inform_duty_0 == 0)
            {
                flag_inform_duty_0 = 1;
                emit send_to_logic_thermostat_done();
            }

            if(duty != last_duty)
                set_pwm_duty(&pwm_9_42_zhenfashi, duty);

            last_duty = duty;

            /* 恒温时将实时的duty值发送给GUI线程 */
            emit send_to_GUI_duty_update(duty);
        }

        msleep(200);
    }

    stopped = false;
}

void HardWareControlThread::stop()
{
    stopped = true;
}

void HardWareControlThread::recei_fro_logic_thermostat(THERMOSTAT thermostat_signal)
{
    thermostat.thermo_switch = thermostat_signal.thermo_switch;
    thermostat.preset_temp = thermostat_signal.preset_temp;
}

void HardWareControlThread::recei_fro_GUI_close_hardware()
{
    close_hardware();
    emit return_to_GUI_close_hardware();
}

void HardWareControlThread::recei_fro_logic_beep(BEEP beep)
{
    beep_info.beep_count = beep.beep_count * 2;
    beep_info.beep_interval = beep.beep_interval;
    beep_timer->start(beep.beep_interval);//启动蜂鸣器的定时器
    Beep_Switch(OPEN);
    beep_info.beep_count--;
}

void HardWareControlThread::beep_timeout()
{
    if(beep_info.beep_count > 0)
    {
        if(beep_info.beep_count%2)
            Beep_Switch(CLOSE);
        else
            Beep_Switch(OPEN);

        beep_info.beep_count--;
    }
    else//鸣叫次数为0则关闭定时器
    {
        beep_timer->stop();
        Beep_Switch(CLOSE);
    }
}
