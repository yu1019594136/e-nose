#include "thread_logic.h"
#include <string.h>
#include "QDebug"

/*********************逻辑控制线程*****************************/
LogicControlThread::LogicControlThread(QObject *parent) :
    QThread(parent)
{
    stopped = false;
}

void LogicControlThread::run()
{
    /* 等待其他线程启动完成 */
    msleep(500);

    /* 开机后先让蒸发室从室温预热到35摄氏度 */
    thermostat_para.thermo_switch = STOP;
    thermostat_para.preset_temp = 35.0;
    thermostat_para.hold_time = 30000;//单位ms, 蒸发时间
    emit send_to_hard_evapor_thermostat(thermostat_para);

    /* 发送蜂鸣器控制信号给硬件线程 */
//    beep_para.beep_count = 2;//鸣叫次数
//    beep_para.beep_interval = 500;//单位ms,鸣叫间隔
//    emit send_to_hard_beep(beep_para);

    /* 发送电磁阀控制信号给硬件线程 */
    //magnetic_para.M1 = HIGH;
    magnetic_para.M1 = LOW;
    //magnetic_para.M2 = HIGH;
    magnetic_para.M2 = LOW;
    //magnetic_para.M3 = HIGH;
    magnetic_para.M3 = LOW;
    //magnetic_para.M4 = HIGH;
    magnetic_para.M4 = LOW;
    emit send_to_hard_magnetic(magnetic_para);

    msleep(500);

    /* 打开气泵清洗气路 */
    pump_para.pump_switch = LOW;
    pump_para.pump_duty = 125000;//全速运转,duty取值范围0-125000
    pump_para.hold_time = 20000;//单位ms
    emit send_to_hard_pump(pump_para);

    sample_para.sample_freq = 50;//单位Hz,每个通道的采样频率
    sample_para.sample_time = 180;//单位s, 每个通道的采样时间长度
    sample_para.filename_prefix = "/root/qt_program/test_data";//数据文件路径以及文件名前缀,
    emit send_to_dataproc_sample(sample_para);

    while(!stopped)
    {
        //sleep(1);
    }

    qDebug("logicthread down!\n");
    stopped = false;
}
void LogicControlThread::stop()
{
    stopped = true;
}

void LogicControlThread::recei_fro_hardware_thermostat_done()
{
    /* 发送蜂鸣器控制信号给硬件线程 */
    beep_para.beep_count = 2;//鸣叫次数
    beep_para.beep_interval = 500;//单位ms,鸣叫间隔
    emit send_to_hard_beep(beep_para);
}
