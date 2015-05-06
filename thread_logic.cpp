#include "thread_logic.h"
#include <string.h>
#include "QDebug"

/*********************逻辑控制线程*****************************/
LogicControlThread::LogicControlThread(QObject *parent) :
    QThread(parent)
{
    stopped = false;

    system_state = PREHEAT;

    operation_flag.standby_flag = AL_SET;
    operation_flag.preheat_flag = UN_SET;
    operation_flag.evaporation_flag = AL_SET;
    operation_flag.thermo_flag = AL_SET;
    operation_flag.sampling_flag = AL_SET;
    operation_flag.clear_flag = AL_SET;
}

void LogicControlThread::run()
{
    /* 等待其他线程启动完成 */
    msleep(500);

    /* 发送蜂鸣器控制信号给硬件线程 */
//    beep_para.beep_count = 2;//鸣叫次数
//    beep_para.beep_interval = 500;//单位ms,鸣叫间隔
//    emit send_to_hard_beep(beep_para);

//    /* 发送电磁阀控制信号给硬件线程 */
//    //magnetic_para.M1 = HIGH;
//    magnetic_para.M1 = LOW;
//    //magnetic_para.M2 = HIGH;
//    magnetic_para.M2 = LOW;
//    //magnetic_para.M3 = HIGH;
//    magnetic_para.M3 = LOW;
//    //magnetic_para.M4 = HIGH;
//    magnetic_para.M4 = LOW;
//    emit send_to_hard_magnetic(magnetic_para);

//    /* 打开气泵清洗气路 */
//    pump_para.pump_switch = LOW;
//    pump_para.pump_duty = 125000;//全速运转,duty取值范围0-125000
//    pump_para.hold_time = 20000;//单位ms
//    emit send_to_hard_pump(pump_para);

//    sample_para.sample_freq = 100;//单位Hz,每个通道的采样频率
//    sample_para.sample_time = 5;//单位s, 每个通道的采样时间长度
//    sample_para.filename_prefix = "/root/qt_program/test_data";//数据文件路径以及文件名前缀,
//    emit send_to_dataproc_sample(sample_para);

    while(!stopped)
    {
        //sleep(1);
        while(system_state == STANDBY)
        {
            if(operation_flag.standby_flag == UN_SET)
            {
                emit send_to_GUI_systemstate(system_state);

                /* 关闭加热带 */
                thermostat_para.thermo_switch = STOP;
                emit send_to_hard_evapor_thermostat(thermostat_para);

                /* 关闭气泵 */
                pump_para.pump_switch = LOW;
                emit send_to_hard_pump(pump_para);

                operation_flag.standby_flag = AL_SET;
            }
        }

        while(system_state == PREHEAT)
        {
            if(operation_flag.preheat_flag == UN_SET)
            {
                emit send_to_GUI_systemstate(system_state);

                /* 开机后先让蒸发室从室温预热到35摄氏度 */
                thermostat_para.thermo_switch = START;
                thermostat_para.preset_temp = 35.0;
                thermostat_para.hold_time = 30000;//单位ms, 蒸发时间
                emit send_to_hard_evapor_thermostat(thermostat_para);

                operation_flag.preheat_flag = AL_SET;
            }
        }

        while (system_state == THERMO)
        {
            if(operation_flag.thermo_flag == UN_SET)
            {
                emit send_to_GUI_systemstate(system_state);

                /* 开机后先让蒸发室从室温预热到35摄氏度 */
                thermostat_para.thermo_switch = START;
                thermostat_para.preset_temp = 45.0;
                thermostat_para.hold_time = 30000;//单位ms, 蒸发时间
                emit send_to_hard_evapor_thermostat(thermostat_para);

                operation_flag.thermo_flag = AL_SET;
            }

        }

        while(system_state == EVAPORATION)
        {

        }

        while(system_state == SAMPLING)
        {

        }

        while(system_state == CLEAR)
        {

        }
    }

    qDebug("logicthread down!\n");
    stopped = false;
}
void LogicControlThread::stop()
{
    stopped = true;
    system_state = QUIT;
}

void LogicControlThread::recei_fro_hardware_preheat_done()
{
    /* 切换到下一个状态 */
    system_state = THERMO;
    operation_flag.thermo_flag = UN_SET;
    operation_flag.preheat_flag = AL_SET;

    /* 发送蜂鸣器控制信号给硬件线程 */
    beep_para.beep_count = 1;//鸣叫次数
    beep_para.beep_interval = 500;//单位ms,鸣叫间隔
    emit send_to_hard_beep(beep_para);

    qDebug() << "preheat done!" << endl;
}

void LogicControlThread::recei_fro_hardware_thermostat_done()
{
    /* 切换到下一个状态 */
    system_state = EVAPORATION;
    operation_flag.evaporation_flag = UN_SET;
    operation_flag.thermo_flag = AL_SET;

    /* 发送蜂鸣器控制信号给硬件线程 */
    beep_para.beep_count = 2;//鸣叫次数
    beep_para.beep_interval = 500;//单位ms,鸣叫间隔
    emit send_to_hard_beep(beep_para);

    qDebug() << "thermo done!" << endl;
}

void LogicControlThread::recei_fro_hardware_evapoartion_done()
{
    /* 切换到下一个状态 */
    system_state = SAMPLING;
    operation_flag.sampling_flag = UN_SET;
    operation_flag.evaporation_flag = AL_SET;

    /* 发送蜂鸣器控制信号给硬件线程 */
    beep_para.beep_count = 3;//鸣叫次数
    beep_para.beep_interval = 500;//单位ms,鸣叫间隔
    emit send_to_hard_beep(beep_para);

    qDebug() << "evaporation done!" << endl;
}

void LogicControlThread::recei_fro_GUI_system_para_set(SYSTEM_PARA_SET system_para_set_info)
{
    system_para_set = system_para_set_info;

//    qDebug() << "system_para_set.preset_temp = " << system_para_set.preset_temp << endl;
//    qDebug() << "system_para_set.hold_time = " << system_para_set.hold_time << endl;
//    qDebug() << "system_para_set.sample_freq = " << system_para_set.sample_freq << endl;
//    qDebug() << "system_para_set.sample_time = " << system_para_set.sample_time << endl;
//    qDebug() << "system_para_set.sample_style = " << system_para_set.sample_style << endl;
//    qDebug() << "system_para_set.evapor_clear_time = " << system_para_set.evapor_clear_time << endl;
//    qDebug() << "system_para_set.reac_clear_time = " << system_para_set.reac_clear_time << endl;
//    qDebug() << "system_para_set.data_file_path = " << system_para_set.data_file_path << endl;
}
