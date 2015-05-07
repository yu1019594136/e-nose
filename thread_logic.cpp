#include "thread_logic.h"
#include <string.h>
#include "QDebug"

/*********************逻辑控制线程*****************************/
LogicControlThread::LogicControlThread(QObject *parent) :
    QThread(parent)
{
    stopped = false;
    read_para_flag = false;

    /* 实例化一个定时器，用于打入样本气体到反应室时可以定时封闭反应室 */
    reac_close_timer = new QTimer();
    connect(reac_close_timer, SIGNAL(timeout()), this, SLOT(close_reac_room()));

    system_state = PREHEAT;

    operation_flag.standby_flag = AL_SET;
    operation_flag.preheat_flag = UN_SET;
    operation_flag.evaporation_flag = AL_SET;
    operation_flag.thermo_flag = AL_SET;
    operation_flag.sampling_flag = AL_SET;
    operation_flag.clear_flag = AL_SET;

    pushButton_state.pushButton_standby = false;
    pushButton_state.pushButton_preheat = false;
    pushButton_state.pushButton_thermo = false;
    pushButton_state.pushButton_evaporation = false;
    pushButton_state.pushButton_sampling = false;
    pushButton_state.pushButton_clear = false;

    pushButton_state.pushButton_set = false;
    pushButton_state.pushButton_al_set = false;
    pushButton_state.pushButton_open = false;
    pushButton_state.pushButton_close = false;
    pushButton_state.pushButton_clear2 = false;
    pushButton_state.pushButton_pause = false;
    pushButton_state.pushButton_plot = false;
    pushButton_state.pushButton_done = false;
}

void LogicControlThread::run()
{
    /* 等待其他线程启动完成 */
    msleep(500);

    /* 发送蜂鸣器控制信号给硬件线程 */
//    beep_para.beep_count = 2;//鸣叫次数
//    beep_para.beep_interval = 500;//单位ms,鸣叫间隔
//    emit send_to_hard_beep(beep_para);

    /* 发送电磁阀控制信号给硬件线程 */
//    magnetic_para.M1 = HIGH;
//    //magnetic_para.M1 = LOW;
//    //magnetic_para.M2 = HIGH;
//    magnetic_para.M2 = LOW;
//    //magnetic_para.M3 = HIGH;
//    magnetic_para.M3 = LOW;
//    //magnetic_para.M4 = HIGH;
//    magnetic_para.M4 = LOW;
//    emit send_to_hard_magnetic(magnetic_para);

    /* 打开气泵清洗气路 */
//    pump_para.pump_switch = HIGH;
//    pump_para.pump_duty = 125000;//全速运转,duty取值范围0-125000
//    pump_para.hold_time = 5000;//单位ms
//    emit send_to_hard_pump(pump_para);

//    sample_para.sample_freq = 100;//单位Hz,每个通道的采样频率
//    sample_para.sample_time = 5;//单位s, 每个通道的采样时间长度
//    sample_para.filename_prefix = "/root/qt_program/test_data";//数据文件路径以及文件名前缀,
//    emit send_to_dataproc_sample(sample_para);

    while(!stopped)
    {
        while(system_state == STANDBY)
        {
            if(operation_flag.standby_flag == UN_SET)
            {
                pushButton_state.pushButton_preheat = true;
                emit send_to_GUI_pushButton_state(pushButton_state);

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
                pushButton_state.pushButton_standby = true;
                pushButton_state.pushButton_preheat = false;

                emit send_to_GUI_pushButton_state(pushButton_state);

                /* 开机后先让蒸发室从室温预热到35摄氏度 */
                thermostat_para.thermo_switch = START;
                thermostat_para.preset_temp = 35.0;
                thermostat_para.hold_time = 0;//单位ms, 蒸发时间,硬件线程对于预设温度为35度不做定时,holdtime参数不起作用
                emit send_to_hard_evapor_thermostat(thermostat_para);

                operation_flag.preheat_flag = AL_SET;
            }
        }

        while (system_state == THERMO)
        {
            if(operation_flag.thermo_flag == UN_SET)
            {
                pushButton_state.pushButton_preheat = true;
                emit send_to_GUI_pushButton_state(pushButton_state);

                /* set按钮使能计时开始5s */
                emit send_to_GUI_set_enable(5);

                operation_flag.thermo_flag = AL_SET;
            }

            /* 读取参数后再来恒温 */
            if(read_para_flag)
            {
                qDebug() << "read_para_flag = " << read_para_flag << endl;

                read_para_flag = false;

                /* 让蒸发室从35度恒温到预设温度 */
                thermostat_para.thermo_switch = START;
                thermostat_para.preset_temp = system_para_set.preset_temp;
                thermostat_para.hold_time = system_para_set.hold_time * 1000;//单位ms, 蒸发时间,硬件线程恒温到预设温度后自动启动定时器开始计时蒸发时间
                emit send_to_hard_evapor_thermostat(thermostat_para);
            }

        }

        while(system_state == EVAPORATION)
        {
            if(operation_flag.evaporation_flag == UN_SET)
            {
                qDebug() << "evaporation start" << endl;

                pushButton_state.pushButton_thermo = true;
                emit send_to_GUI_pushButton_state(pushButton_state);

                /* 关闭蒸发室 */
                //magnetic_para.M1 = HIGH;
                magnetic_para.M1 = LOW;
                magnetic_para.M2 = HIGH;
                //magnetic_para.M2 = LOW;
                //magnetic_para.M3 = HIGH;
                magnetic_para.M3 = LOW;
                //magnetic_para.M4 = HIGH;
                magnetic_para.M4 = LOW;
                emit send_to_hard_magnetic(magnetic_para);

                operation_flag.evaporation_flag = AL_SET;
            }
            msleep(1000);
            qDebug() << "evaporating ..." << endl;

        }

        while(system_state == SAMPLING)
        {
            if(operation_flag.sampling_flag == UN_SET)
            {
                pushButton_state.pushButton_evaporation = true;
                emit send_to_GUI_pushButton_state(pushButton_state);

                /* 蒸发室需要清洗，所以停止恒温 */
                thermostat_para.thermo_switch = STOP;
                //thermostat_para.preset_temp = system_para_set.preset_temp;
                //thermostat_para.hold_time = system_para_set.hold_time * 1000;//单位ms, 蒸发时间,硬件线程恒温到预设温度后自动启动定时器开始计时蒸发时间
                emit send_to_hard_evapor_thermostat(thermostat_para);

                /* 打开采样气路 */
                magnetic_para.M1 = HIGH;
                //magnetic_para.M1 = LOW;
                magnetic_para.M2 = HIGH;
                //magnetic_para.M2 = LOW;
                magnetic_para.M3 = HIGH;
                //magnetic_para.M3 = LOW;
                //magnetic_para.M4 = HIGH;
                magnetic_para.M4 = LOW;
                emit send_to_hard_magnetic(magnetic_para);

                /* 打入样本气体10秒钟 */
                pump_para.pump_switch = HIGH;
                pump_para.pump_duty = 125000;//全速运转,duty取值范围0-125000
                pump_para.hold_time = system_para_set.pump_up_time * 1000;//单位ms
                emit send_to_hard_pump(pump_para);

                /* 打入样本气体到反应室后可以定时封闭反应室 */
                reac_close_timer->start(pump_para.hold_time);

                /* 同时开始采样 */
                sample_para.sample_freq = system_para_set.sample_freq;//单位Hz,每个通道的采样频率
                sample_para.sample_time = system_para_set.sample_time;//单位s, 每个通道的采样时间长度
                sample_para.filename_prefix = system_para_set.data_file_path;//数据文件路径以及文件名前缀,
                emit send_to_dataproc_sample(sample_para);

                operation_flag.sampling_flag = AL_SET;
            }
        }

        while(system_state == CLEAR)
        {
            if(operation_flag.clear_flag == UN_SET)
            {
                qDebug() << "clear start" << endl;

                pushButton_state.pushButton_sampling = true;
                emit send_to_GUI_pushButton_state(pushButton_state);

                operation_flag.clear_flag = AL_SET;
            }

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
    beep_para.beep_interval = 300;//单位ms,鸣叫间隔
    emit send_to_hard_beep(beep_para);

    qDebug() << "evaporation done!" << endl;
}

/* 接收来自数据处理线程的采样完成信号 */
void LogicControlThread::recei_fro_hardware_sample_done()
{
    /* 切换到下一个状态 */
    system_state = CLEAR;
    operation_flag.sampling_flag = AL_SET;
    operation_flag.clear_flag = UN_SET;

    /* 发送蜂鸣器控制信号给硬件线程 */
    beep_para.beep_count = 4;//鸣叫次数
    beep_para.beep_interval = 250;//单位ms,鸣叫间隔
    emit send_to_hard_beep(beep_para);

    qDebug() << "sample done!" << endl;

}

void LogicControlThread::recei_fro_GUI_system_para_set(SYSTEM_PARA_SET system_para_set_info)
{
    system_para_set = system_para_set_info;

    read_para_flag = true;

    qDebug() << "system_para_set.preset_temp = " << system_para_set.preset_temp << endl;
    qDebug() << "system_para_set.hold_time = " << system_para_set.hold_time << endl;
    qDebug() << "system_para_set.pump_up_time = " << system_para_set.pump_up_time << endl;
    qDebug() << "system_para_set.sample_freq = " << system_para_set.sample_freq << endl;
    qDebug() << "system_para_set.sample_time = " << system_para_set.sample_time << endl;
    qDebug() << "system_para_set.sample_style = " << system_para_set.sample_style << endl;
    qDebug() << "system_para_set.evapor_clear_time = " << system_para_set.evapor_clear_time << endl;
    qDebug() << "system_para_set.reac_clear_time = " << system_para_set.reac_clear_time << endl;
    qDebug() << "system_para_set.data_file_path = " << system_para_set.data_file_path << endl;
}
/* 封闭反应室 */
void LogicControlThread::close_reac_room()
{
    msleep(1000);
    /* 封闭反应室 */
    magnetic_para.M1 = HIGH;
    //magnetic_para.M1 = LOW;
    //magnetic_para.M2 = HIGH;
    magnetic_para.M2 = LOW;
    magnetic_para.M3 = HIGH;
    //magnetic_para.M3 = LOW;
    //magnetic_para.M4 = HIGH;
    magnetic_para.M4 = LOW;

    emit send_to_hard_magnetic(magnetic_para);

}
