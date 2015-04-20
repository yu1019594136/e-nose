#include "thread_logic.h"
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
    msleep(100);

    /* 开机后先让蒸发室从室温预热到35摄氏度 */
    thermostat_para.thermo_switch = START;
    thermostat_para.preset_temp = 35.0;
    thermostat_para.hold_time = 30000;//单位ms
    emit send_to_hard_evapor_thermostat(thermostat_para);

    /* 发送蜂鸣器控制信号给硬件线程 */
//    beep_para.beep_count = 2;//鸣叫次数
//    beep_para.beep_interval = 500;//单位ms,鸣叫间隔
//    emit send_to_hard_beep(beep_para);

    /* 打开气泵清洗气路 */
//    pump_para.pump_switch = OPEN;
//    pump_para.pump_duty = 125000;//全速运转
//    pump_para.hold_time = 10000;//单位ms
//    emit send_to_hard_pump(pump_para);

    while(!stopped)
    {
        sleep(1);
    }

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
