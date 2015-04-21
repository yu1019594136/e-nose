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

    /* 实例化一个定时器用于蜂鸣器鸣叫间隔控制 */
    beep_timer = new QTimer();
    connect(beep_timer, SIGNAL(timeout()), this, SLOT(beep_timeout()));

    /* 实例化一个定时器用于气泵开启时间控制 */
    pump_timer = new QTimer();
    connect(pump_timer, SIGNAL(timeout()), this, SLOT(pump_timeout()));

    stopped = false;
}

void HardWareControlThread::run()
{
    GUI_REALTIME_INFO realtime_info;
    float temp_float = 0.0;
    char temp_str[]="00.000";
    int duty = 0;
    int last_duty = 0;

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

        /* 恒温控制 ******************************************/
        if(thermostat.thermo_switch == START)//开始恒温
        {
            /* 检测温度是否稳定 */

            /* 根据ds18b20反馈和预设温度计算控制量 */
            duty = pid_calculation(&Thermostatic[para_num], temp_float);

            /* 达到预期温度后通知逻辑线程,仅仅通知一次 */
            if(duty == 0 && flag_inform_duty_0 == 0)
            {
                flag_inform_duty_0 = 1;
                emit send_to_logic_thermostat_done();
            }

            if(duty != last_duty)
                set_pwm_duty(&pwm_9_42_zhenfashi, duty);

            last_duty = duty;

            /* 恒温时将实时的duty值发送给GUI线程 */
            emit send_to_GUI_thermostat_duty_update(duty);
        }

        msleep(200);
    }

    stopped = false;
}

void HardWareControlThread::stop()
{
    stopped = true;
}

void HardWareControlThread::recei_fro_logic_thermostat(THERMOSTAT thermostat_para)
{
    thermostat.thermo_switch = thermostat_para.thermo_switch;
    thermostat.preset_temp = thermostat_para.preset_temp;
    thermostat.hold_time = thermostat_para.hold_time;

    if(thermostat.thermo_switch == STOP)//停止恒温
    {
        Heat_S_Switch(LOW);//关闭加热电路开关
        set_pwm_duty(&pwm_9_42_zhenfashi, 0);
        flag_inform_duty_0 = 0;
        emit send_to_GUI_thermostat_duty_update(0);
    }
    else if(thermostat.thermo_switch == START)//开始恒温
    {
        /* 恒温参数配置 */
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
        Heat_S_Switch(HIGH);

        flag_inform_duty_0 = 0;
    }
}

void HardWareControlThread::recei_fro_logic_beep(BEEP beep_para)
{
    beep.beep_count = beep_para.beep_count * 2;
    beep.beep_interval = beep_para.beep_interval;

    if(beep.beep_count > 0)//避免0次数或者负参数的干扰
    {
        beep_timer->start(beep_para.beep_interval);//启动蜂鸣器的定时器
        Beep_Switch(HIGH);//调用函数打开蜂鸣器
        beep.beep_count--;
    }
}

void HardWareControlThread::recei_fro_logic_pump(PUMP pump_para)
{
    pump.pump_switch = pump_para.pump_switch;
    pump.pump_duty = pump_para.pump_duty;
    pump.hold_time = pump_para.hold_time;

    if(pump.pump_switch == HIGH)
    {
        /* 配置PWM波 */
        set_pwm_duty(&pwm_8_13_airpump, pump.pump_duty);

        /* 接通气泵电路 */
        Pump_S_Switch(pump.pump_switch);

        /* 启动计时 */
        pump_timer->start(pump.hold_time);

        /* 更新气泵硬件信息到GUI线程 */
        emit send_to_GUI_pump_duty_update(pump.pump_duty);
    }
}

void HardWareControlThread::recei_fro_logic_magnetic(MAGNETIC magnetic_para)
{
    magnetic.M1 = magnetic_para.M1;
    magnetic.M2 = magnetic_para.M2;
    magnetic.M3 = magnetic_para.M3;
    magnetic.M4 = magnetic_para.M4;

    /* 控制相应的电磁阀 */
    M1_Switch(magnetic.M1);
    M2_Switch(magnetic.M2);
    M3_Switch(magnetic.M3);
    M4_Switch(magnetic.M4);

    emit send_to_GUI_magnetic_update(magnetic);
}

void HardWareControlThread::recei_fro_GUI_close_hardware()
{
    close_hardware();

    /* 告诉逻辑线程硬件已经关闭 */
    emit return_to_GUI_close_hardware();
}

void HardWareControlThread::beep_timeout()
{
    if(beep.beep_count > 0)
    {
        if(beep.beep_count%2)
            Beep_Switch(LOW);
        else
            Beep_Switch(HIGH);

        beep.beep_count--;
    }
    else//鸣叫次数为0则关闭定时器
    {
        beep_timer->stop();
        Beep_Switch(LOW);
    }
}

void HardWareControlThread::pump_timeout()
{
    /* 关闭定时器 */
    pump_timer->stop();

    /* 断开气泵电路 */
    Pump_S_Switch(LOW);

    /* 配置PWM波 */
    set_pwm_duty(&pwm_8_13_airpump, 0);

    /* 更新气泵硬件信息到GUI线程 */
    emit send_to_GUI_pump_duty_update(0);
}
