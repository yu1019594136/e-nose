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
    THERMOSTAT thermostat_signal;

    /* 开机后先让蒸发室从室温预热到35摄氏度 */
    thermostat_signal.thermo_switch = START;
    thermostat_signal.preset_temp = 35.0;
    emit send_to_hard_evapor_thermostat(thermostat_signal);

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
    beep.beep_count = 2;//鸣叫次数
    beep.beep_interval = 500;//单位ms,鸣叫间隔
    emit send_to_hard_beep(beep);
}
