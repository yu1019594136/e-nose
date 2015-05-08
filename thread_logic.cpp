#include "thread_logic.h"
#include <string.h>
#include "QDebug"

/*********************逻辑控制线程*****************************/
LogicControlThread::LogicControlThread(QObject *parent) :
    QThread(parent)
{
    stopped = false;
    read_para_flag = false;
    open_clicked_flag = false;
    close_clicked_flag = false;

    user_button_enable.mode = UN_SET;
    user_button_enable.enable_time = 0;

    /* 实例化一个定时器，用于打入样本气体到反应室时可以定时封闭反应室 */
    close_pump_and_reac_timer = new QTimer();
    connect(close_pump_and_reac_timer, SIGNAL(timeout()), this, SLOT(close_pump_and_reac()));

    /* 实例化一个定时器用于蒸发时间控制 */
    evaporation_timer = new QTimer();
    connect(evaporation_timer, SIGNAL(timeout()), this, SLOT(evapoartion_timeout_done()));

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
                thermostat_para.thermo_inform_flag = false;
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

                /* 通知GUI线程使能set按钮5s */
                user_button_enable.mode = SET_BUTTON;
                user_button_enable.enable_time = 5;//单位s, 5s
                emit send_to_GUI_user_buttton_enable(user_button_enable);

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
                thermostat_para.thermo_inform_flag = true;
                emit send_to_hard_evapor_thermostat(thermostat_para);
            }

        }

        while(system_state == EVAPORATION)
        {
            if(operation_flag.evaporation_flag == UN_SET)
            {
                pushButton_state.pushButton_thermo = true;
                emit send_to_GUI_pushButton_state(pushButton_state);

                /* 通知GUI线程使能open按钮 */
                user_button_enable.mode = OPEN_BUTTON;
                user_button_enable.enable_time = 0;
                emit send_to_GUI_user_buttton_enable(user_button_enable);

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

            /* 用户点击open后打开盖子 */
            if(open_clicked_flag)
            {
                open_clicked_flag = false;

                /* 用户需要打开盖子加入样本，此时温度传感器取出，反馈不准确故先停止恒温 */
                thermostat_para.thermo_switch = STOP;
                //thermostat_para.preset_temp = system_para_set.preset_temp;
                //thermostat_para.hold_time = 0;//单位ms, 蒸发时间,硬件线程恒温到预设温度后自动启动定时器开始计时蒸发时间
                //thermostat_para.thermo_inform_flag = false;
                emit send_to_hard_evapor_thermostat(thermostat_para);

                qDebug() << "open_clicked" << endl;
            }

            /* 用户点击close后盖上盖子 */
            if(close_clicked_flag)
            {
                close_clicked_flag = false;

                /* 开始蒸发 */
                thermostat_para.thermo_switch = START;
                thermostat_para.preset_temp = system_para_set.preset_temp;
                thermostat_para.thermo_inform_flag = false;//不要通知，否则形成死循环
                emit send_to_hard_evapor_thermostat(thermostat_para);

                /* 开始蒸发计时 */
                evaporation_timer->start(system_para_set.hold_time * 1000);

                qDebug() << "close_clicked" << endl;
                qDebug() << "evaporation timer start:" << system_para_set.hold_time << " s."<< endl;
            }
        }

        while(system_state == SAMPLING)
        {
            if(operation_flag.sampling_flag == UN_SET)
            {
                pushButton_state.pushButton_evaporation = true;
                emit send_to_GUI_pushButton_state(pushButton_state);

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
                emit send_to_hard_pump(pump_para);

                /* 打入样本气体到反应室后定时封闭反应室 */
                close_pump_and_reac_timer->start(system_para_set.pump_up_time * 1000);
                qDebug() << "close_pump_and_reac_timer->start;" << system_para_set.pump_up_time << " s." << endl;

                /* 同时开始采样 */
                sample_para.sample_freq = system_para_set.sample_freq;//单位Hz,每个通道的采样频率
                sample_para.sample_time = system_para_set.sample_time;//单位s, 每个通道的采样时间长度
                sample_para.filename_prefix = system_para_set.data_file_path;//数据文件路径以及文件名前缀,
                emit send_to_dataproc_sample(sample_para);

                /* 蒸发完成，采样开始后可以停止蒸发室的恒温 */
                thermostat_para.thermo_switch = STOP;
                //thermostat_para.preset_temp = system_para_set.preset_temp;
                //thermostat_para.hold_time = 0;//单位ms, 蒸发时间,硬件线程恒温到预设温度后自动启动定时器开始计时蒸发时间
                emit send_to_hard_evapor_thermostat(thermostat_para);
                qDebug() << "sample start, and thermo stop" << endl;

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

                /* 通知GUI线程使能clear按钮 */
                user_button_enable.mode = CLEAR_BUTTON;
                user_button_enable.enable_time = 0;
                emit send_to_GUI_user_buttton_enable(user_button_enable);

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

void LogicControlThread::evapoartion_timeout_done()
{
    /* 关闭定时器 */
    evaporation_timer->stop();
    qDebug() << "evaporation_timer->stop()" << endl;

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

/* 关闭气泵,封闭反应室 */
void LogicControlThread::close_pump_and_reac()
{
    /* 关闭定时器 */
    close_pump_and_reac_timer->stop();
    qDebug() << "close_pump_and_reac_timer->stop()" << endl;

    /* 关闭气泵 */
    pump_para.pump_switch = LOW;
    //pump_para.pump_duty = 125000;//全速运转,duty取值范围0-125000
    //pump_para.hold_time = system_para_set.pump_up_time * 1000;//单位ms
    emit send_to_hard_pump(pump_para);

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

/* 用户在系统操作面板按下按钮后应该通知逻辑线程产生动作 */
void LogicControlThread::recei_fro_GUI_user_button_action(USER_BUTTON_ENABLE user_button_enable_para)
{
    if(user_button_enable_para.mode == OPEN_BUTTON)
    {
        open_clicked_flag = true;
    }
    else if(user_button_enable_para.mode == CLOSE_BUTTON)
    {
        close_clicked_flag = true;
    }
    else if(user_button_enable_para.mode == CLEAR_BUTTON)
    {

    }
    else if(user_button_enable_para.mode == PAUSE_BUTTON)
    {

    }
    else if(user_button_enable_para.mode == PLOT_BUTTON)
    {

    }
    else if(user_button_enable_para.mode == DONE_BUTTON)
    {

    }

}
