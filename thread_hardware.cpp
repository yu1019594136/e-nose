#include "thread_hardware.h"
#include <QDebug>

/* 硬件相关接口 */
#include "HW_interface.h"
#include "pid.h"
#include "PWM.h"

/*********************硬件控制线程*****************************/
HardWareControlThread::HardWareControlThread(QObject *parent) :
    QThread(parent)
{
    /* 构造函数数据成员初始化 */
    thermostat.thermo_switch = CLOSE;
    thermostat.preset_temp = 35.0;
    stopped = false;
}

void HardWareControlThread::run()
{

    GUI_REALTIME_INFO realtime_info;

    /* 上电后系统初始化各个硬件电路，配置操作系统状态，等待用户操作 */
    init_hardware();

    while (!stopped)
    {

        get_realtime_info(&realtime_info);

        emit send_to_GUI_realtime_info_update(realtime_info);

        /* 恒温控制 */
        if(thermostat.thermo_switch == OPEN)//开始恒温
        {
            //qDebug() << "thermostat.preset_temp = " << thermostat.preset_temp << endl;

            //emit return_to_logic_thermostat(SUCCESS);//操作成功
        }
        else//停止恒温
        {

            //emit return_to_logic_thermostat(SUCCESS);//操作成功
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
