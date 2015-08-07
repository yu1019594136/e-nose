#include "thread_hardware.h"
#include <QDebug>
#include <QDateTime>

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

    stopped = false;
    start_heat_high_speed = false;
    flag_record_temp = false;
}

void HardWareControlThread::run()
{
    GUI_REALTIME_INFO realtime_info;
    float temp_float = 0.0;
    //float last_temp_float = 0.0;//用于检测温度是否稳定的辅助变量，如果当前温度比上一次温度低，即认为温度稳定，可以开始恒温
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

        /* 开启全速加热，全速加热将根据信号中的stop_temp自动停止 */
        if(start_heat_high_speed)
        {
            if(temp_float > stop_temp * 1.0)
            {
                /* 停止全速加热 */
                set_pwm_duty(&pwm_9_42_zhenfashi, 0);

                start_heat_high_speed = false;
                qDebug() << "high speed heat stop!" << endl;
            }
        }

        /* 开始进行温度数据记录 */
        if(flag_record_temp)
        {
            fprintf(fp_record_temp, "%f\n", temp_float);
        }

        /* 恒温控制 **&& ((thermostat.wait_temp_stable && temp_stable) || !thermostat.wait_temp_stable )****************************************/
        if(thermostat.thermo_switch == START)//开始恒温
        {
            /* 检测温度是否稳定 */

            /* 根据ds18b20反馈和预设温度计算控制量 */
            duty = pid_calculation(&Thermostatic[para_num], temp_float);

            /* 达到预期温度后通知逻辑线程,仅仅通知一次 */
            if(duty == 0 && flag_inform_duty_0 == 0)
            {
                flag_inform_duty_0 = 1;

                /* 预热完成还是恒温完成 */
                if(abs(thermostat.preset_temp - 35.0) < 0.01)
                    emit send_to_logic_preheat_done();
                else
                {
                    if(thermostat.thermo_inform_flag)
                        emit send_to_logic_thermostat_done();
                }
            }

            if(duty != last_duty)
            {
                set_pwm_duty(&pwm_9_42_zhenfashi, duty);
                /* 恒温时将实时的duty值发送给GUI线程 */
                emit send_to_GUI_thermostat_duty_update(duty);
            }

            last_duty = duty;

        }
        msleep(200);
    }

    /* 操作结束前，
     * 关闭各个功能硬件电路，恢复系统配置；
     */
    close_hardware();

    stopped = false;
    qDebug("hardwarethread down!\n");
}

void HardWareControlThread::stop()
{
    stopped = true;
}

void HardWareControlThread::recei_fro_logic_thermostat(THERMOSTAT thermostat_para)
{
    thermostat.thermo_switch = thermostat_para.thermo_switch;
    thermostat.preset_temp = thermostat_para.preset_temp;
    thermostat.thermo_inform_flag = thermostat_para.thermo_inform_flag;

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
    else
    {
        Beep_Switch(LOW);//调用函数打开蜂鸣器
    }
}

void HardWareControlThread::recei_fro_logic_pump(PUMP pump_para)
{
    pump.pump_switch = pump_para.pump_switch;
    pump.pump_duty = pump_para.pump_duty;
    pump.return_action_mode = pump_para.return_action_mode;

    if(pump.pump_switch == HIGH)
    {
        /* 配置PWM波 */
        set_pwm_duty(&pwm_8_13_airpump, pump.pump_duty);

        /* 接通气泵电路 */
        Pump_S_Switch(HIGH);

//        /* 启动计时 */
//        pump_timer->start(pump.hold_time);

        /* 更新气泵硬件信息到GUI线程 */
        emit send_to_GUI_pump_duty_update(pump.pump_duty);
    }
    else
    {
        /* 断开气泵电路 */
        Pump_S_Switch(LOW);

        /* 配置PWM波 */
        set_pwm_duty(&pwm_8_13_airpump, 0);

        /* 更新气泵硬件信息到GUI线程 */
        emit send_to_GUI_pump_duty_update(0);
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

void HardWareControlThread::recei_fro_logic_start_heat_high_speed(int stop_temp_para)
{
    stop_temp = stop_temp_para;

    /* 开启全速加热 */
    set_pwm_duty(&pwm_9_42_zhenfashi, 10000000);

    qDebug() << "start high speed heat! stop high speed heat at " << stop_temp << endl;

    start_heat_high_speed = true;
}

void HardWareControlThread::recei_fro_logic_start_record_temp()
{
    flag_record_temp = true;

    QDateTime datetime = QDateTime::currentDateTime();
    QString filepath = datetime.toString("yyyy.MM.dd-hh_mm_ss_") + "temp_record.txt";
    char *filename;
    QByteArray ba;

    ba = filepath.toLatin1();
    filename = ba.data();

    if((fp_record_temp = fopen(filename, "w")) != NULL)
    {
        qDebug() << "open temp record file success, start to record temp!" << endl;
    }
    else
    {
        qDebug() << "open temp record file failed! temp recording cancle!" << endl;
        flag_record_temp = false;
    }
}

/* 采样完成后，停止记录温度数据 */
void HardWareControlThread::recei_fro_logic_stop_record_temp()
{
    flag_record_temp = false;
    qDebug() << "stop_record_temp" << endl;

    /* 关闭文件句柄 */
    if(fp_record_temp)
        fclose(fp_record_temp);

    fp_record_temp = NULL;
}
