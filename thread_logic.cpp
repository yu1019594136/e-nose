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
    thermostat_signal.thermo_switch = OPEN;
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

void LogicControlThread::result_fro_hard_evapor_thermostat(RESULT result)
{
    if(result == SUCCESS)
        qDebug() << "result from hardware_thread: SUCCESS" << endl;
    else
        qDebug() << "result from hardware_thread: ERROR" << endl;
}
