#include "thread_logic.h"
#include <string.h>
#include <QDebug>
#include <QDateTime>

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

    system_state = STANDBY;//PREHEAT;
    operation_flag.standby_flag =       UN_SET;
    operation_flag.preheat_flag =       AL_SET;
    operation_flag.evaporation_flag =   AL_SET;
    operation_flag.thermo_flag =        AL_SET;
    operation_flag.sampling_flag =      AL_SET;
    operation_flag.clear_flag =         AL_SET;

    pushButton_state.pushButton_standby = false;
    pushButton_state.pushButton_preheat = false;
    pushButton_state.pushButton_thermo = false;
    pushButton_state.pushButton_evaporation = false;
    pushButton_state.pushButton_sampling = false;
    pushButton_state.pushButton_clear = false;

    /* 采样过程有四个状态，1：吸入气体；2：吸入等待；3：呼出气体；4：呼出等待 */
    hale_count = 0;
    hale_state = 0;
    hale_state_change = false;

    /* 清洗气室标志 1:清洗反应室; 2:清洗蒸发室 */
    clear_state = 0;
    clear_state_change = false;

    pwm_state = 0;

}

void LogicControlThread::run()
{
    /* 等待其他线程启动完成 */
    msleep(500);

    while(!stopped)
    {
        while(system_state == STANDBY)
        {
            if(operation_flag.standby_flag == UN_SET)
            {
                pushButton_state.pushButton_standby = false;
                pushButton_state.pushButton_preheat = true;
                pushButton_state.pushButton_thermo = false;
                pushButton_state.pushButton_evaporation = false;
                pushButton_state.pushButton_sampling = false;
                pushButton_state.pushButton_clear = false;
                emit send_to_GUI_pushButton_state(pushButton_state);

                /* 先清洗反应室 */
                clear_state = 0;
                clear_state_change = true;

                operation_flag.standby_flag = AL_SET;
            }
            if(clear_state_change)
            {
                clear_state++;
                clear_state_change = false;

                if(clear_state == 1)
                {
                    /* 打开气泵, 清洗反应室 */
                    //magnetic_para.M1 = HIGH;
                    magnetic_para.M1 = LOW;
                    //magnetic_para.M2 = HIGH;
                    magnetic_para.M2 = LOW;
                    //magnetic_para.M3 = HIGH;
                    magnetic_para.M3 = LOW;
                    magnetic_para.M4 = HIGH;
                    //magnetic_para.M4 = LOW;
                    emit send_to_hard_magnetic(magnetic_para);

                    msleep(1000);

                    pump_para.pump_switch = HIGH;
                    pump_para.pump_duty = 125000;//全速运转,duty取值范围0-125000
                    pump_para.return_action_mode = STANDBY;//
                    emit send_to_hard_pump(pump_para);

                    /* 定时蒸发室清洗时间 */
                    close_pump_and_reac_timer->start(30 * 1000);
                    qDebug() << "reac_timer->start; evapor_clear_time 30 s." << endl;
                }
                else if(clear_state == 2)
                {
                    /* 打开气泵, 清洗蒸发室*/
                    magnetic_para.M1 = HIGH;
                    //magnetic_para.M1 = LOW;
                    //magnetic_para.M2 = HIGH;
                    magnetic_para.M2 = LOW;
                    //magnetic_para.M3 = HIGH;
                    magnetic_para.M3 = LOW;
                    //magnetic_para.M4 = HIGH;
                    magnetic_para.M4 = LOW;
                    emit send_to_hard_magnetic(magnetic_para);

                    msleep(1000);

                    pump_para.pump_switch = HIGH;
                    pump_para.pump_duty = 125000;//全速运转,duty取值范围0-125000
                    pump_para.return_action_mode = STANDBY;//
                    emit send_to_hard_pump(pump_para);

                    /* 定时蒸发室清洗时间 */
                    close_pump_and_reac_timer->start(30 * 1000);
                    qDebug() << "reac_timer->start; reac_clear_time 30 s." << endl;
                }
                else if(clear_state == 3)//清洗完成
                {
                    clear_state = 0;
                    clear_state_change = false;

                    /* 切换到下一个状态 */
                    system_state = PREHEAT;
                    operation_flag.preheat_flag = UN_SET;
                    operation_flag.standby_flag = AL_SET;
                }
            }
        }

        while(system_state == PREHEAT)
        {
            if(operation_flag.preheat_flag == UN_SET)
            {
                pushButton_state.pushButton_standby = true;
                pushButton_state.pushButton_preheat = false;
                pushButton_state.pushButton_thermo = false;
                pushButton_state.pushButton_evaporation = false;
                pushButton_state.pushButton_sampling = false;
                pushButton_state.pushButton_clear = false;

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
                pushButton_state.pushButton_standby = true;
                pushButton_state.pushButton_preheat = true;
                pushButton_state.pushButton_thermo = false;
                pushButton_state.pushButton_evaporation = false;
                pushButton_state.pushButton_sampling = false;
                pushButton_state.pushButton_clear = false;

                emit send_to_GUI_pushButton_state(pushButton_state);

                /* 通知GUI线程使能set按钮5s */
                user_button_enable.mode = SET_BUTTON;
                user_button_enable.enable_time = 10;//单位s, 5s
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
                pushButton_state.pushButton_standby = true;
                pushButton_state.pushButton_preheat = true;
                pushButton_state.pushButton_thermo = true;
                pushButton_state.pushButton_evaporation = false;
                pushButton_state.pushButton_sampling = false;
                pushButton_state.pushButton_clear = false;
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
                pushButton_state.pushButton_standby = true;
                pushButton_state.pushButton_preheat = true;
                pushButton_state.pushButton_thermo = true;
                pushButton_state.pushButton_evaporation = true;
                pushButton_state.pushButton_sampling = false;
                pushButton_state.pushButton_clear = false;
                emit send_to_GUI_pushButton_state(pushButton_state);

                /* 开始采样 */
                sample_para.sample_freq = system_para_set.sample_freq;//单位Hz,每个通道的采样频率
                sample_para.sample_time = system_para_set.sample_time;//单位s, 每个通道的采样时间长度
                sample_para.filename_prefix = system_para_set.liquor_brand;//数据文件路径以及文件名前缀,
                sample_para.sample_inform_flag = true;
                emit send_to_dataproc_sample(sample_para);

                /* 先采集3s空气中的响应 */
                //msleep(3000);

                /* 读取呼吸次数, 进入状态1 */
                hale_count = system_para_set.hale_count;
                hale_state_change = true;
                hale_state = 0;
                pwm_state = 0;

                qDebug() << "sample start, and thermo stop" << endl;

                operation_flag.sampling_flag = AL_SET;
            }

            if(hale_count)//如果呼吸次数大于0
            {
                if(hale_state_change)
                {
                    hale_state++;//切换到下一个状态,
                    hale_state_change = false;//保证本段代码只执行一次

                    /* 参数检查，四个状态参数中，如果需要跳过某个状态，那么将该参数设置为0即可 */
                    if(hale_state == 1 && system_para_set.inhale_time[pwm_state] == 0)//跳过状态1
                    {
                        hale_state = 2;
                        qDebug() << "jump from hale state 1 to hale state 2" << endl;
                    }
                    if(hale_state == 2 && system_para_set.inhale_wait_time[pwm_state] == 0)//跳过状态2
                    {
                        hale_state = 3;
                        qDebug() << "jump from hale state 2 to hale state 3" << endl;
                    }
                    if(hale_state == 3 && system_para_set.exhale_time[pwm_state] == 0)//跳过状态3
                    {
                        hale_state = 4;
                        qDebug() << "jump from hale state 3 to hale state 4" << endl;
                    }
                    if(hale_state == 4 && system_para_set.exhale_wait_time[pwm_state] == 0)//跳过状态4
                    {
                        hale_state = 5;
                        qDebug() << "jump from hale state 4 to hale state 5" << endl;
                    }

                    if(hale_state == 1 && system_para_set.inhale_time[pwm_state] != 0)
                    {
                        /* 打开采样气路, 吸入样本气体 */
                        magnetic_para.M1 = HIGH;
                        //magnetic_para.M1 = LOW;
                        magnetic_para.M2 = HIGH;
                        //magnetic_para.M2 = LOW;
                        magnetic_para.M3 = HIGH;
                        //magnetic_para.M3 = LOW;
                        magnetic_para.M4 = HIGH;
                        //magnetic_para.M4 = LOW;
                        emit send_to_hard_magnetic(magnetic_para);

                        /* 吸入样本气体n秒钟 */
                        pump_para.pump_switch = HIGH;
                        pump_para.pump_duty = system_para_set.pwm_duty[pwm_state];//全速运转,duty取值范围0-125000
                        pump_para.return_action_mode = SAMPLING;//由于是采样阶段，启泵定时时间达到后将关闭气泵，并封闭反应室
                        emit send_to_hard_pump(pump_para);

                        /* 打入样本气体到反应室后定时封闭反应室 */
                        close_pump_and_reac_timer->start(system_para_set.inhale_time[pwm_state] * 1000);
                        qDebug() << "reac_timer->start, inhale sample time" << system_para_set.inhale_time[pwm_state] << " s." << endl;

                    }
                    else if(hale_state == 2 && system_para_set.inhale_wait_time[pwm_state] != 0)
                    {
                        /* 吸入样本气体后等待...... */
                        close_pump_and_reac_timer->start(system_para_set.inhale_wait_time[pwm_state] * 1000);
                        qDebug() << "reac_timer->start, inhale wait time" << system_para_set.inhale_wait_time[pwm_state] << " s." << endl;
                    }
                    else if(hale_state == 3 && system_para_set.exhale_time[pwm_state] != 0)
                    {
                        /* 打开清洗气路，做呼气动作 */
                        //magnetic_para.M1 = HIGH;
                        magnetic_para.M1 = LOW;
                        //magnetic_para.M2 = HIGH;
                        magnetic_para.M2 = LOW;
                        //magnetic_para.M3 = HIGH;
                        magnetic_para.M3 = LOW;
                        magnetic_para.M4 = HIGH;
                        //magnetic_para.M4 = LOW;
                        emit send_to_hard_magnetic(magnetic_para);

                        /* 呼出样本气体n秒 */
                        pump_para.pump_switch = HIGH;
                        pump_para.pump_duty = 125000;//全速运转,duty取值范围0-125000
                        pump_para.return_action_mode = SAMPLING;//由于是采样阶段，启泵定时时间达到后将关闭气泵，并封闭反应室
                        emit send_to_hard_pump(pump_para);

                        /* 呼出反应室样本气体n秒 */
                        close_pump_and_reac_timer->start(system_para_set.exhale_time[pwm_state] * 1000);
                        qDebug() << "reac_timer->start, exhale time" << system_para_set.exhale_time[pwm_state] << " s." << endl;
                    }
                    else if(hale_state == 4 && system_para_set.exhale_wait_time[pwm_state] != 0)
                    {
                        /* 呼出反应室样本气体n秒后，等待 */
                        close_pump_and_reac_timer->start(system_para_set.exhale_wait_time[pwm_state] * 1000);
                        qDebug() << "reac_timer->start, exhale wait time" << system_para_set.exhale_wait_time[pwm_state] << " s." << endl;
                    }
                    else if(hale_state == 5)//一次呼吸结束
                    {
                        if(hale_count > 0)//如果呼吸次数大于0，则继续下一次
                        {
                            hale_count--;
                            pwm_state++;
                            qDebug() << pwm_state << "th hale sample done!" << endl;
                            hale_state = 0;
                            hale_state_change = true;
                        }
                        else//否则等待采样时间到达后进入下一个系统状态
                        {
                            //nothing;
                            hale_state = 0;
                            pwm_state = 0;
                            hale_state_change = false;

                            qDebug() << "hale_count = 0, hale sample done!" << endl;
                        }
                    }
                }
            }
            else
            {
                //qDebug() << "hale_count = 0, hale sample done!" << endl;
            }
        }

        while(system_state == CLEAR)
        {
            if(operation_flag.clear_flag == UN_SET)
            {
                qDebug() << "clear start" << endl;

                /* 采样完成，清洗开始后可以停止蒸发室的恒温 */
                thermostat_para.thermo_switch = STOP;
                //thermostat_para.preset_temp = system_para_set.preset_temp;
                //thermostat_para.hold_time = 0;//单位ms, 蒸发时间,硬件线程恒温到预设温度后自动启动定时器开始计时蒸发时间
                emit send_to_hard_evapor_thermostat(thermostat_para);

                pushButton_state.pushButton_standby = true;
                pushButton_state.pushButton_preheat = true;
                pushButton_state.pushButton_thermo = true;
                pushButton_state.pushButton_evaporation = true;
                pushButton_state.pushButton_sampling = true;
                pushButton_state.pushButton_clear = false;
                emit send_to_GUI_pushButton_state(pushButton_state);

                /* 开始采样 */
                sample_para.sample_freq = system_para_set.sample_freq;//单位Hz,每个通道的采样频率
                sample_para.sample_time = system_para_set.evapor_clear_time;//单位s, 每个通道的采样时间长度
                sample_para.filename_prefix = "clear_data";//数据文件路径以及文件名前缀,
                sample_para.sample_inform_flag = false;
                emit send_to_dataproc_sample(sample_para);

                /* 先清洗反应室 */
                clear_state = 0;
                clear_state_change = true;

                operation_flag.clear_flag = AL_SET;
            }

            if(clear_state_change)
            {
                clear_state++;
                clear_state_change = false;

                if(clear_state == 1)
                {
                    /* 打开气泵, 清洗反应室 */
                    //magnetic_para.M1 = HIGH;
                    magnetic_para.M1 = LOW;
                    //magnetic_para.M2 = HIGH;
                    magnetic_para.M2 = LOW;
                    //magnetic_para.M3 = HIGH;
                    magnetic_para.M3 = LOW;
                    magnetic_para.M4 = HIGH;
                    //magnetic_para.M4 = LOW;
                    emit send_to_hard_magnetic(magnetic_para);

                    msleep(1000);

                    pump_para.pump_switch = HIGH;
                    pump_para.pump_duty = 125000;//全速运转,duty取值范围0-125000
                    pump_para.return_action_mode = CLEAR;//
                    emit send_to_hard_pump(pump_para);

                    /* 定时蒸发室清洗时间 */
                    close_pump_and_reac_timer->start(system_para_set.evapor_clear_time * 1000);
                    qDebug() << "reac_timer->start; evapor_clear_time " << system_para_set.evapor_clear_time << " s." << endl;
                }
                else if(clear_state == 2)
                {
                    /* 打开气泵, 清洗蒸发室*/
                    magnetic_para.M1 = HIGH;
                    //magnetic_para.M1 = LOW;
                    //magnetic_para.M2 = HIGH;
                    magnetic_para.M2 = LOW;
                    //magnetic_para.M3 = HIGH;
                    magnetic_para.M3 = LOW;
                    //magnetic_para.M4 = HIGH;
                    magnetic_para.M4 = LOW;
                    emit send_to_hard_magnetic(magnetic_para);

                    msleep(1000);

                    pump_para.pump_switch = HIGH;
                    pump_para.pump_duty = 125000;//全速运转,duty取值范围0-125000
                    pump_para.return_action_mode = CLEAR;//
                    emit send_to_hard_pump(pump_para);

                    /* 定时蒸发室清洗时间 */
                    close_pump_and_reac_timer->start(system_para_set.reac_clear_time * 1000);
                    qDebug() << "reac_timer->start; reac_clear_time " << system_para_set.reac_clear_time << " s." << endl;
                }
                else if(clear_state == 3)//清洗完成
                {
                    clear_state = 0;
                    clear_state_change = false;
                }

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
    qDebug() << "reac_timer->stop()" << endl;

    if(pump_para.return_action_mode == STANDBY)
    {
        if(clear_state == 1)
        {
            /* 关闭气泵 */
            pump_para.pump_switch = LOW;
            //pump_para.pump_duty = 125000;//全速运转,duty取值范围0-125000
            //pump_para.hold_time = system_para_set.pump_up_time * 1000;//单位ms
            emit send_to_hard_pump(pump_para);

            qDebug() << "evapor clear done!" << endl;

            clear_state_change = true;
        }
        else if(clear_state == 2)
        {
            /* 关闭气泵 */
            pump_para.pump_switch = LOW;
            //pump_para.pump_duty = 125000;//全速运转,duty取值范围0-125000
            //pump_para.hold_time = system_para_set.pump_up_time * 1000;//单位ms
            emit send_to_hard_pump(pump_para);

            qDebug() << "reac clear done!" << endl;

            clear_state_change = true;
        }
    }
    else if(pump_para.return_action_mode == SAMPLING)//采样阶段打入样本气体后需要封闭反应室
    {
        if(hale_state == 1)
        {
            /* 吸入样本气体结束，密封反应室, 蒸发室进气口密封 */
            //magnetic_para.M1 = HIGH;
            magnetic_para.M1 = LOW;
            //magnetic_para.M2 = HIGH;
            magnetic_para.M2 = LOW;
            magnetic_para.M3 = HIGH;
            //magnetic_para.M3 = LOW;
            //magnetic_para.M4 = HIGH;
            magnetic_para.M4 = LOW;
            emit send_to_hard_magnetic(magnetic_para);

            /* 吸入样本气体结束，关闭气泵 */
            pump_para.pump_switch = LOW;
            //pump_para.pump_duty = 125000;//全速运转,duty取值范围0-125000
            //pump_para.hold_time = system_para_set.pump_up_time * 1000;//单位ms
            emit send_to_hard_pump(pump_para);
        }
        else if (hale_state == 2)
        {

        }
        else if (hale_state == 3)
        {
            /* 呼出样本气体结束，密封反应室, 蒸发室进气口密封 */
            //magnetic_para.M1 = HIGH;
            magnetic_para.M1 = LOW;
            //magnetic_para.M2 = HIGH;
            magnetic_para.M2 = LOW;
            magnetic_para.M3 = HIGH;
            //magnetic_para.M3 = LOW;
            //magnetic_para.M4 = HIGH;
            magnetic_para.M4 = LOW;
            emit send_to_hard_magnetic(magnetic_para);

            /* 呼出样本气体结束，关闭气泵 */
            pump_para.pump_switch = LOW;
            //pump_para.pump_duty = 125000;//全速运转,duty取值范围0-125000
            //pump_para.hold_time = system_para_set.pump_up_time * 1000;//单位ms
            emit send_to_hard_pump(pump_para);
        }
        else if (hale_state == 4)
        {

        }
        else
        {
            //nothing;
        }

        hale_state_change = true;//切换到下一个状态
    }
    else if(pump_para.return_action_mode == CLEAR)//清洗阶段气泵清洗完成后
    {
        if(clear_state == 1)
        {
            /* 关闭气泵 */
            pump_para.pump_switch = LOW;
            //pump_para.pump_duty = 125000;//全速运转,duty取值范围0-125000
            //pump_para.hold_time = system_para_set.pump_up_time * 1000;//单位ms
            emit send_to_hard_pump(pump_para);

            qDebug() << "evapor clear done!" << endl;

            clear_state_change = true;
        }
        else if(clear_state == 2)
        {
            /* 关闭气泵 */
            pump_para.pump_switch = LOW;
            //pump_para.pump_duty = 125000;//全速运转,duty取值范围0-125000
            //pump_para.hold_time = system_para_set.pump_up_time * 1000;//单位ms
            emit send_to_hard_pump(pump_para);

            qDebug() << "reac clear done!" << endl;

            /* 清洗阶段完成后,通知GUI线程使能clear按钮 */
            user_button_enable.mode = CLEAR_BUTTON;
            user_button_enable.enable_time = 0;
            emit send_to_GUI_user_buttton_enable(user_button_enable);

            clear_state_change = true;
        }
    }
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
    qDebug() << "system_para_set.sample_size = " << system_para_set.sample_size << endl;
    qDebug() << "system_para_set.sample_freq = " << system_para_set.sample_freq << endl;
    qDebug() << "system_para_set.sample_time = " << system_para_set.sample_time << endl;
    qDebug() << "system_para_set.sample_style = " << system_para_set.sample_style << endl;
    qDebug() << "system_para_set.evapor_clear_time = " << system_para_set.evapor_clear_time << endl;
    qDebug() << "system_para_set.reac_clear_time = " << system_para_set.reac_clear_time << endl;
    qDebug() << "system_para_set.liquor_brand = " << system_para_set.liquor_brand << endl;

    qDebug() << "system_para_set.hale_count = " << system_para_set.hale_count << endl << endl;

    qDebug() << "system_para_set.pwm_duty[0] = " << system_para_set.pwm_duty[0] << endl;
    qDebug() << "system_para_set.pwm_duty[1] = " << system_para_set.pwm_duty[1] << endl;
    qDebug() << "system_para_set.pwm_duty[2] = " << system_para_set.pwm_duty[2] << endl << endl;

    qDebug() << "system_para_set.inhale_time[0] = " << system_para_set.inhale_time[0] << endl;
    qDebug() << "system_para_set.inhale_time[1] = " << system_para_set.inhale_time[1] << endl;
    qDebug() << "system_para_set.inhale_time[2] = " << system_para_set.inhale_time[2] << endl << endl;

    qDebug() << "system_para_set.inhale_wait_time[0] = " << system_para_set.inhale_wait_time[0] << endl;
    qDebug() << "system_para_set.inhale_wait_time[1] = " << system_para_set.inhale_wait_time[1] << endl;
    qDebug() << "system_para_set.inhale_wait_time[2] = " << system_para_set.inhale_wait_time[2] << endl << endl;

    qDebug() << "system_para_set.exhale_time[0] = " << system_para_set.exhale_time[0] << endl;
    qDebug() << "system_para_set.exhale_time[1] = " << system_para_set.exhale_time[1] << endl;
    qDebug() << "system_para_set.exhale_time[2] = " << system_para_set.exhale_time[2] << endl << endl;

    qDebug() << "system_para_set.exhale_wait_time[0] = " << system_para_set.exhale_wait_time[0] << endl;
    qDebug() << "system_para_set.exhale_wait_time[1] = " << system_para_set.exhale_wait_time[1] << endl;
    qDebug() << "system_para_set.exhale_wait_time[2] = " << system_para_set.exhale_wait_time[2] << endl << endl;

    /* 记录参数到txt文件 */
    QDateTime datetime = QDateTime::currentDateTime();

    QByteArray data_file_path_ba = system_para_set.liquor_brand.toLatin1();
    char *data_file_path_char = data_file_path_ba.data();

    QString f_temp = QString("/root/qt_program/") + datetime.toString("yyyy.MM.dd-hh_mm_ss_") + system_para_set.liquor_brand + "_system_para_set.txt";
    QByteArray para_ba = f_temp.toLatin1();
    char *para_filename = para_ba.data();

    FILE *fp_para;
    fp_para = fopen(para_filename, "w");

    fprintf(fp_para, "system_para_set.preset_temp =\t%f\n", system_para_set.preset_temp);
    fprintf(fp_para, "system_para_set.hold_time =\t%d\n", system_para_set.hold_time);
    fprintf(fp_para, "system_para_set.sample_size =\t%d\n", system_para_set.sample_size);
    fprintf(fp_para, "system_para_set.sample_freq =\t%f\n", system_para_set.sample_freq);
    fprintf(fp_para, "system_para_set.sample_time =\t%d\n", system_para_set.sample_time);
    fprintf(fp_para, "system_para_set.sample_style =\t%d\n", system_para_set.sample_style);
    fprintf(fp_para, "system_para_set.evapor_clear_time =\t%d\n", system_para_set.evapor_clear_time);
    fprintf(fp_para, "system_para_set.reac_clear_time =\t%d\n", system_para_set.reac_clear_time);
    fprintf(fp_para, "system_para_set.data_file_path =\t%s\n\n", data_file_path_char);

    fprintf(fp_para, "system_para_set.hale_count =\t%d\n\n", system_para_set.hale_count);

    fprintf(fp_para, "system_para_set.pwm_duty[0] =\t%d\n", system_para_set.pwm_duty[0]);
    fprintf(fp_para, "system_para_set.pwm_duty[1] =\t%d\n", system_para_set.pwm_duty[1]);
    fprintf(fp_para, "system_para_set.pwm_duty[2] =\t%d\n\n", system_para_set.pwm_duty[2]);

    fprintf(fp_para, "system_para_set.inhale_time[0] =\t%d\n", system_para_set.inhale_time[0]);
    fprintf(fp_para, "system_para_set.inhale_time[1] =\t%d\n", system_para_set.inhale_time[1]);
    fprintf(fp_para, "system_para_set.inhale_time[2] =\t%d\n\n", system_para_set.inhale_time[2]);

    fprintf(fp_para, "system_para_set.inhale_wait_time[0] =\t%d\n", system_para_set.inhale_wait_time[0]);
    fprintf(fp_para, "system_para_set.inhale_wait_time[1] =\t%d\n", system_para_set.inhale_wait_time[1]);
    fprintf(fp_para, "system_para_set.inhale_wait_time[2] =\t%d\n\n", system_para_set.inhale_wait_time[2]);

    fprintf(fp_para, "system_para_set.exhale_time[0] =\t%d\n", system_para_set.exhale_time[0]);
    fprintf(fp_para, "system_para_set.exhale_time[1] =\t%d\n", system_para_set.exhale_time[1]);
    fprintf(fp_para, "system_para_set.exhale_time[2] =\t%d\n\n", system_para_set.exhale_time[2]);

    fprintf(fp_para, "system_para_set.exhale_wait_time[0] =\t%d\n", system_para_set.exhale_wait_time[0]);
    fprintf(fp_para, "system_para_set.exhale_wait_time[1] =\t%d\n", system_para_set.exhale_wait_time[1]);
    fprintf(fp_para, "system_para_set.exhale_wait_time[2] =\t%d\n\n", system_para_set.exhale_wait_time[2]);

    fclose(fp_para);
    fp_para = NULL;
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
        /* 打开清洗气路 */
        //magnetic_para.M1 = HIGH;
        magnetic_para.M1 = LOW;
        //magnetic_para.M2 = HIGH;
        magnetic_para.M2 = LOW;
        //magnetic_para.M3 = HIGH;
        magnetic_para.M3 = LOW;
        magnetic_para.M4 = HIGH;
        //magnetic_para.M4 = LOW;
        emit send_to_hard_magnetic(magnetic_para);

        /* 打开气泵清洗 */
        pump_para.pump_switch = HIGH;
        pump_para.pump_duty = 125000;//全速运转,duty取值范围0-125000
        emit send_to_hard_pump(pump_para);
    }
    else if(user_button_enable_para.mode == PAUSE_BUTTON)
    {
        /* 关闭气泵清洗 */
        pump_para.pump_switch = LOW;
        pump_para.pump_duty = 125000;//全速运转,duty取值范围0-125000
        emit send_to_hard_pump(pump_para);
    }
    else if(user_button_enable_para.mode == PLOT_BUTTON)
    {
        /* 采样绘图 */
        sample_para.sample_freq = 60;//单位Hz,每个通道的采样频率
        sample_para.sample_time = 60;//单位s, 每个通道的采样时间长度
        sample_para.filename_prefix = "clear_test";//数据文件路径以及文件名前缀,
        sample_para.sample_inform_flag = false;//不要通知，否则扰乱循环
        emit send_to_dataproc_sample(sample_para);

        /* 通知GUI线程禁能plot按钮 */
        user_button_enable.mode = PLOT_BUTTON;
        user_button_enable.enable_time = 0;//采样绘图时间结束后，数据处理线程将发送信号给GUI线程将plot使能
        emit send_to_GUI_user_buttton_enable(user_button_enable);
    }
    else if(user_button_enable_para.mode == DONE_BUTTON)
    {
        /* 关闭气泵，安保措施：点击done时气泵可能还在运行 */
        pump_para.pump_switch = LOW;
        pump_para.pump_duty = 125000;//全速运转,duty取值范围0-125000
        emit send_to_hard_pump(pump_para);

        /* 点击done按钮后，禁能clear、pause、plot、done四个按钮 */
        user_button_enable.mode = UNSET;
        user_button_enable.enable_time = 0;
        emit send_to_GUI_user_buttton_enable(user_button_enable);

        /* 根据单次采样还是连续采样决定回到待机状态或者预热状态 */
        if(system_para_set.sample_style == SINGLE )
        {
            system_state = PREHEAT;
            operation_flag.preheat_flag = UN_SET;
            operation_flag.clear_flag = AL_SET;
        }
        else if(system_para_set.sample_style == CONTINUE)
        {
            system_state = THERMO;
            operation_flag.thermo_flag = UN_SET;
            operation_flag.clear_flag = AL_SET;
        }
    }

}
