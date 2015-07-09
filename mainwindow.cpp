#include <unistd.h>
#include <QWSServer>
#include <QDateTime>
#include <QMetaType>
#include <QDebug>
#include <QPainter>
#include <QFile>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "HW_interface.h"

Q_DECLARE_METATYPE(GUI_REALTIME_INFO)
Q_DECLARE_METATYPE(THERMOSTAT)
Q_DECLARE_METATYPE(BEEP)
Q_DECLARE_METATYPE(PUMP)
Q_DECLARE_METATYPE(MAGNETIC)
Q_DECLARE_METATYPE(SAMPLE)
Q_DECLARE_METATYPE(PLOT_INFO)
Q_DECLARE_METATYPE(SYSTEM_PARA_SET)
Q_DECLARE_METATYPE(PUSHBUTTON_STATE)
Q_DECLARE_METATYPE(USER_BUTTON_ENABLE)

/* 构造函数 */
MainWindow::MainWindow(QWidget *parent) :QMainWindow(parent), ui(new Ui::MainWindow)
{
    /* 根据ui布局设置窗口 */
    ui->setupUi(this);

    /* 实例化一个tab页用于绘图 */
    plot_widget = new Plot_Widget(this);
    ui->Qtabwidget->addTab(plot_widget, "plot data");

    /* 从item_file.txt中读取条目添加到UI的comboBox */
    QString temp;
    QStringList q_str_list;
    QFile item_file(ITEM_FILE_PATH);

    if(!item_file.open(QIODevice::ReadOnly | QIODevice::Text))
        qDebug() << "can not open the file:" << item_file.fileName() << endl;
    else
    {
        while(!item_file.atEnd())
        {
            temp = item_file.readLine();
            temp.remove(QChar('\n'));
            q_str_list.append(temp);
        }
        ui->comboBox_data_filepath->addItems(q_str_list);
        item_file.close();
    }
    /* 在最后一个条目中加入分隔符 */
    ui->comboBox_data_filepath->insertSeparator(ui->comboBox_data_filepath->count() - 1);
    ui->comboBox_data_filepath->clearEditText();

    /* 从default_config_para.txt文件读取默认配置参数 */
    FILE *fp_default_para;
    unsigned int temp_uint;
    int temp_int;
    int temp_current_index;
    float temp_float;
    char temp_string[1024];
    QString temp_QString;


    if((fp_default_para = fopen(DEFAULT_CONFIG_PARA_PATH, "r")) != NULL)
    {
        fscanf(fp_default_para, "system_para_set.preset_temp =\t%f\n", &temp_float);
        ui->set_evapor_temp->setValue(temp_float);

        fscanf(fp_default_para, "system_para_set.hold_time =\t%d\n", &temp_int);
        ui->set_evapor_time->setValue(temp_int);

        fscanf(fp_default_para, "system_para_set.sample_size =\t%d\n", &temp_int);
        ui->set_pump_up_time->setValue(temp_int);

        fscanf(fp_default_para, "system_para_set.sample_freq =\t%f\n", &temp_float);
        ui->set_sample_rate->setValue(temp_float);

        fscanf(fp_default_para, "system_para_set.sample_time =\t%d\n", &temp_int);
        ui->set_sample_time->setValue(temp_int);

        fscanf(fp_default_para, "system_para_set.sample_style =\t%d\n", &temp_int);
        if(temp_int == SINGLE)
        {
            ui->radioButton_single->setChecked(true);
            ui->radioButton_continue->setChecked(false);
        }
        else if(temp_int == CONTINUE)
        {
            ui->radioButton_single->setChecked(false);
            ui->radioButton_continue->setChecked(true);
        }

        fscanf(fp_default_para, "system_para_set.evapor_clear_time =\t%d\n", &temp_int);
        ui->set_evapor_clear->setValue(temp_int);

        fscanf(fp_default_para, "system_para_set.reac_clear_time =\t%d\n", &temp_int);
        ui->set_reac_clear->setValue(temp_int);

        fscanf(fp_default_para, "system_para_set.data_file_path =\t%s\n\n", temp_string);
        temp_QString = temp_string;

        temp_current_index = ui->comboBox_data_filepath->findText(temp_QString);
        if(temp_current_index != -1)//如果QComboBox中该表项存在匹配表项，那么将其设置为当前表项
        {
            ui->comboBox_data_filepath->setCurrentIndex(temp_current_index);
        }
        else//如果QComboBox中该表项不存在匹配表项，那么将其添加到已经存在的列表的最前面
        {
            ui->comboBox_data_filepath->insertItem(0, temp_QString);
            ui->comboBox_data_filepath->setCurrentIndex(0);
        }

        fscanf(fp_default_para, "system_para_set.hale_count =\t%d\n\n", &temp_int);
        ui->spinBox->setValue(temp_int);

        fscanf(fp_default_para, "system_para_set.inhale_time[0] =\t%d\n", &temp_uint);
        ui->spinBox_2->setValue(temp_uint);

        fscanf(fp_default_para, "system_para_set.pwm_duty[0] =\t%d\n", &temp_uint);
        ui->spinBox_14->setValue(temp_uint);

        fscanf(fp_default_para, "system_para_set.inhale_wait_time[0] =\t%d\n", &temp_uint);
        ui->spinBox_5->setValue(temp_uint);

        fscanf(fp_default_para, "system_para_set.exhale_time[0] =\t%d\n", &temp_uint);
        ui->spinBox_4->setValue(temp_uint);

        fscanf(fp_default_para, "system_para_set.exhale_wait_time[0] =\t%d\n\n", &temp_uint);
        ui->spinBox_3->setValue(temp_uint);

        fscanf(fp_default_para, "system_para_set.inhale_time[1] =\t%d\n", &temp_uint);
        ui->spinBox_6->setValue(temp_uint);

        fscanf(fp_default_para, "system_para_set.pwm_duty[1] =\t%d\n", &temp_uint);
        ui->spinBox_15->setValue(temp_uint);

        fscanf(fp_default_para, "system_para_set.inhale_wait_time[1] =\t%d\n", &temp_uint);
        ui->spinBox_7->setValue(temp_uint);

        fscanf(fp_default_para, "system_para_set.exhale_time[1] =\t%d\n", &temp_uint);
        ui->spinBox_8->setValue(temp_uint);

        fscanf(fp_default_para, "system_para_set.exhale_wait_time[1] =\t%d\n\n", &temp_uint);
        ui->spinBox_9->setValue(temp_uint);

        fscanf(fp_default_para, "system_para_set.inhale_time[2] =\t%d\n", &temp_uint);
        ui->spinBox_10->setValue(temp_uint);

        fscanf(fp_default_para, "system_para_set.pwm_duty[2] =\t%d\n", &temp_uint);
        ui->spinBox_16->setValue(temp_uint);

        fscanf(fp_default_para, "system_para_set.inhale_wait_time[2] =\t%d\n", &temp_uint);
        ui->spinBox_11->setValue(temp_uint);

        fscanf(fp_default_para, "system_para_set.exhale_time[2] =\t%d\n", &temp_uint);
        ui->spinBox_12->setValue(temp_uint);

        fscanf(fp_default_para, "system_para_set.exhale_wait_time[2] =\t%d\n\n", &temp_uint);
        ui->spinBox_13->setValue(temp_uint);

        fscanf(fp_default_para, "system_para_set.inhale_time[3] =\t%d\n", &temp_uint);
        ui->spinBox_18->setValue(temp_uint);

        fscanf(fp_default_para, "system_para_set.pwm_duty[3] =\t%d\n", &temp_uint);
        ui->spinBox_17->setValue(temp_uint);

        fscanf(fp_default_para, "system_para_set.inhale_wait_time[3] =\t%d\n", &temp_uint);
        ui->spinBox_19->setValue(temp_uint);

        fscanf(fp_default_para, "system_para_set.exhale_time[3] =\t%d\n", &temp_uint);
        ui->spinBox_20->setValue(temp_uint);

        fscanf(fp_default_para, "system_para_set.exhale_wait_time[3] =\t%d\n\n", &temp_uint);
        ui->spinBox_21->setValue(temp_uint);

        qDebug("use the para in file default_config_para.txt.\n");

        fclose(fp_default_para);
        fp_default_para = NULL;
    }
    else
    {
        qDebug("Warning: can't open the default_config_para.txt, use the default config para in e-nose program.\n");
    }

    /* 隐藏鼠标 */
    QWSServer::setCursorVisible(false);

    /* 注册元类型 */
    qRegisterMetaType <GUI_REALTIME_INFO>("GUI_REALTIME_INFO");
    qRegisterMetaType <THERMOSTAT>("THERMOSTAT");
    qRegisterMetaType <BEEP>("BEEP");
    qRegisterMetaType <PUMP>("PUMP");
    qRegisterMetaType <MAGNETIC>("MAGNETIC");
    qRegisterMetaType <SAMPLE>("SAMPLE");
    qRegisterMetaType <PLOT_INFO>("PLOT_INFO");
    qRegisterMetaType <SYSTEM_PARA_SET>("SYSTEM_PARA_SET");
    qRegisterMetaType <PUSHBUTTON_STATE>("PUSHBUTTON_STATE");
    qRegisterMetaType <USER_BUTTON_ENABLE>("USER_BUTTON_ENABLE");

    /* 实例化三个线程并启动,将三个子线程相关的信号关联到GUI主线程的槽函数 */
    logic_thread = new LogicControlThread();
    hardware_thread = new HardWareControlThread();
    dataprocess_thread = new DataProcessThread();

    /* 实时更新温湿度数据 */
    connect(hardware_thread, SIGNAL(send_to_GUI_realtime_info_update(GUI_REALTIME_INFO)), this, SLOT(recei_fro_hard_realtime_info_update(GUI_REALTIME_INFO)), Qt::QueuedConnection);

    /* 逻辑线程发送蒸发室恒温信号给硬件线程 */
    connect(logic_thread, SIGNAL(send_to_hard_evapor_thermostat(THERMOSTAT)), hardware_thread, SLOT(recei_fro_logic_thermostat(THERMOSTAT)), Qt::QueuedConnection);
    connect(hardware_thread, SIGNAL(send_to_logic_preheat_done()), logic_thread, SLOT(recei_fro_hardware_preheat_done()), Qt::QueuedConnection);
    connect(hardware_thread, SIGNAL(send_to_logic_thermostat_done()), logic_thread, SLOT(recei_fro_hardware_thermostat_done()), Qt::QueuedConnection);
    //connect(hardware_thread, SIGNAL(send_to_logic_evaporation_done()), logic_thread, SLOT(recei_fro_hardware_evapoartion_done()), Qt::QueuedConnection);
    /* 恒温操作时GUI实时更新thermostat_duty */
    connect(hardware_thread, SIGNAL(send_to_GUI_thermostat_duty_update(int)), this, SLOT(recei_fro_hard_thermostat_duty_update(int)), Qt::QueuedConnection);
    /* GUI实时更新按钮状态 */
    connect(logic_thread, SIGNAL(send_to_GUI_pushButton_state(PUSHBUTTON_STATE)), this, SLOT(recei_fro_logic_pushButton_state(PUSHBUTTON_STATE)),Qt::QueuedConnection);

    /* 逻辑线程发送蜂鸣器控制信号给硬件线程 */
    connect(logic_thread, SIGNAL(send_to_hard_beep(BEEP)), hardware_thread, SLOT(recei_fro_logic_beep(BEEP)), Qt::QueuedConnection);

    /* 逻辑线程发送气泵控制信号给硬件线程 */
    connect(logic_thread, SIGNAL(send_to_hard_pump(PUMP)), hardware_thread, SLOT(recei_fro_logic_pump(PUMP)),Qt::QueuedConnection);
    connect(hardware_thread, SIGNAL(send_to_GUI_pump_duty_update(int)), this, SLOT(recei_fro_hard_pump_duty_update(int)),Qt::QueuedConnection);

    /* 逻辑线程发送电磁阀控制信号给硬件线程 */
    connect(logic_thread, SIGNAL(send_to_hard_magnetic(MAGNETIC)), hardware_thread, SLOT(recei_fro_logic_magnetic(MAGNETIC)), Qt::QueuedConnection);
    connect(hardware_thread, SIGNAL(send_to_GUI_magnetic_update(MAGNETIC)), this, SLOT(recei_fro_hard_magnetic_update(MAGNETIC)), Qt::QueuedConnection);

    /* 逻辑线程发送给数据处理线程的采样控制信号 */
    connect(logic_thread, SIGNAL(send_to_dataproc_sample(SAMPLE)), dataprocess_thread, SLOT(recei_fro_logic_sample(SAMPLE)), Qt::QueuedConnection);

    /* plot_widget对象接收来自数据处理线程的采样数据绘图命令 */
    connect(dataprocess_thread, SIGNAL(send_to_PlotWidget_plotdata(PLOT_INFO)), plot_widget, SLOT(recei_fro_datapro_dataplot(PLOT_INFO)), Qt::QueuedConnection);

    /* 通知GUI线程按钮开始计时 */
    connect(logic_thread, SIGNAL(send_to_GUI_user_buttton_enable(USER_BUTTON_ENABLE)), this, SLOT(recei_fro_logic_user_buttton_enable(USER_BUTTON_ENABLE)), Qt::QueuedConnection);

    /* 参数面板中的参数并发送给逻辑线程 */
    connect(this, SIGNAL(send_to_logic_system_para_set(SYSTEM_PARA_SET)), logic_thread, SLOT(recei_fro_GUI_system_para_set(SYSTEM_PARA_SET)), Qt::QueuedConnection);

    /* 数据处理线程采样完成后发送信号给逻辑线程 */
    connect(dataprocess_thread, SIGNAL(send_to_logic_sample_done()), logic_thread, SLOT(recei_fro_dataproc_sample_done()), Qt::QueuedConnection);

    /* 用户在系统操作面板按下按钮后应该通知逻辑线程产生动作 */
    connect(this, SIGNAL(send_to_logic_user_button_action(USER_BUTTON_ENABLE)), logic_thread, SLOT(recei_fro_GUI_user_button_action(USER_BUTTON_ENABLE)), Qt::QueuedConnection);

    /* 此种情况不需要返回信号，系统操作面板中的plot按钮在采集完后需要被使能 */
//    connect(dataprocess_thread, SIGNAL(send_to_GUI_enable_plot_pushbutton()), this, SLOT(plot_pushbutton_enable()), Qt::QueuedConnection);

    /* 用户通过点击plot1和plot2来选择绘制哪个数据 */
    connect(this, SIGNAL(send_to_plot_widget(int)), plot_widget, SLOT(recei_fro_GUI_PLOT_DATA_TYPE(int)), Qt::QueuedConnection);

    /* 释放两块内存块，将两个指针清零，发送信号给plot_widget对象，执行一次空指针绘图 */
    connect(logic_thread, SIGNAL(send_to_dataproc_reset_memory()), dataprocess_thread, SLOT(recei_from_logic_reset_memory()), Qt::QueuedConnection);

    hardware_thread->start();
    dataprocess_thread->start();
    logic_thread->start();

    /* 开机显示时间 */
    QDateTime datetime = QDateTime::currentDateTime();
    ui->datetime->setText(datetime.toString("yyyy.MM.dd hh:mm"));

    /* 实例化一个定时器  */
    pushButton_enable_timer = new QTimer(this);
    connect(pushButton_enable_timer, SIGNAL(timeout()), this, SLOT(on_pushButton_al_set_clicked()));

    /* 实例化一个定时器并启动 */
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(timerUpdate()));
    timer->start(1000 * 60);//一分钟更新一次时间

    /* 显示界面各个标签 */
    ui->logo->setText("Electronic Nose");
    ui->evap_temp_lable->setText("evaporation temp:");
    ui->reactio_temp_lable->setText("reaction temp:");
    ui->humidity_lable->setText("reaction humi:");
    ui->heat_duty_evap_lable->setText("evap heat duty:");
    ui->heat_duty_reac_lable->setText("reac heat duty:");
    ui->M1->setText("M1 status:");
    ui->M2->setText("M2 status:");
    ui->M3->setText("M3 status:");
    ui->M4->setText("M4 status:");
    ui->preset_temp->setText("preset temp:");

    ui->label_13->setText("hale count");
    ui->label_9->setText("inhale");
    ui->label_14->setText("pwm duty");
    ui->label_10->setText("inhale wait");
    ui->label_11->setText("exhale");
    ui->label_12->setText("exhale wait");

    /* 显示系统气泵、电磁阀、加热带的初始状态 */
    ui->pump_speed_value->setText("0.0/125.0 us");
    ui->OFF1->setText("OFF");
    ui->OFF2->setText("OFF");
    ui->OFF3->setText("OFF");
    ui->OFF4->setText("OFF");
    ui->heat_duty_evap_value->setText("0.0/10.0 ms");
    ui->heat_duty_reac_value->setText("0.0/10.0 ms");

    /* 默认单次采样 */
    ui->radioButton_single->setChecked(true);
    ui->radioButton_continue->setChecked(false);

    /* 程序启动后跳至选项卡0 */
    ui->Qtabwidget->setCurrentIndex(0);

    ui->pushButton_set->setEnabled(false);
    ui->pushButton_al_set->setEnabled(false);
    ui->pushButton_open->setEnabled(false);
    ui->pushButton_close->setEnabled(false);
    ui->pushButton_clear_2->setEnabled(false);
    ui->pushButton_pause->setEnabled(false);
    ui->pushButton_plot->setEnabled(false);
    ui->pushButton_plot_2->setEnabled(false);
    ui->pushButton_done->setEnabled(false);

    user_button_enable.mode = UNSET;
    user_button_enable.enable_time = 0;
}

MainWindow::~MainWindow()
{
    /* 将数据表项全部重新写入文件进行保存 */
    QFile item_file("/root/qt_program/item_file.txt");
    QString temp;
    char *item_data;
    QByteArray ba;

    int i;

    if(!item_file.open(QIODevice::WriteOnly |QIODevice::Truncate | QIODevice::Text))
        qDebug() << "can not open the file1:" << item_file.fileName() << endl;
    else
    {
        qDebug() << "count = " << ui->comboBox_data_filepath->count() << endl;

        for(i = 0; i < ui->comboBox_data_filepath->count(); i++)
        {
            if(ui->comboBox_data_filepath->itemText(i) != "")
            {
                temp = ui->comboBox_data_filepath->itemText(i) + "\n";
                ba = temp.toLatin1();
                item_data = ba.data();
                item_file.write(item_data);
            }
        }
        item_file.close();
    }
    item_data = NULL;

    delete ui;
    delete logic_thread;
    delete hardware_thread;
    delete dataprocess_thread;
}

void MainWindow::on_Quit_Button_clicked()
{
    /* 将活跃状态的线程关闭 */
    if(logic_thread->isRunning())
        logic_thread->stop();
    while(!logic_thread->isFinished());

    if(dataprocess_thread->isRunning())
        dataprocess_thread->stop();
    while(!dataprocess_thread->isFinished());

    if(hardware_thread->isRunning())
        hardware_thread->stop();
    while(!hardware_thread->isFinished());

    /* 5s内应用程序关机 */
//    Application_quit(5);

    /* 退出事件循环，结束程序 */
    QApplication *p;
    p->quit();
}

void MainWindow::recei_fro_hard_realtime_info_update(GUI_REALTIME_INFO realtime_info)
{
    ui->evap_temp_value->setText(realtime_info.ds18b20_temp);
    ui->humidty_value->setText(realtime_info.sht21_humid);
    ui->temp_value_sht21->setText(realtime_info.sht21_temp);
}

void MainWindow::timerUpdate()
{
    QDateTime datetime = QDateTime::currentDateTime();
    ui->datetime->setText(datetime.toString("yyyy.MM.dd hh:mm"));
}

/* 接收来自硬件线程的恒温实时duty值 */
void MainWindow::recei_fro_hard_thermostat_duty_update(int duty_info)
{
    /* 将duty换算成ms单位 */
    ui->heat_duty_evap_value->setText(QString::number(duty_info/1000000.0, 'f', 1) + "/10.0 ms");
}

/* 接收来自硬件线程的开启气泵时duty值 */
void MainWindow::recei_fro_hard_pump_duty_update(int duty_info)
{
    /* 将duty换算成us单位 */
    ui->pump_speed_value->setText(QString::number(duty_info/1000.0, 'f', 1) + "/125.0 us");
}

void MainWindow::recei_fro_hard_magnetic_update(MAGNETIC magnetic_info)
{
    if(magnetic_info.M1 == HIGH)
        ui->OFF1->setText("ON");
    else
        ui->OFF1->setText("OFF");

    if(magnetic_info.M2 == HIGH)
        ui->OFF2->setText("ON");
    else
        ui->OFF2->setText("OFF");


    if(magnetic_info.M3 == HIGH)
        ui->OFF3->setText("ON");
    else
        ui->OFF3->setText("OFF");


    if(magnetic_info.M4 == HIGH)
        ui->OFF4->setText("ON");
    else
        ui->OFF4->setText("OFF");
}

/* 接收来自逻辑线程的系统状态信息 */
void MainWindow::recei_fro_logic_pushButton_state(PUSHBUTTON_STATE pushButton_state_para)
{
    ui->pushButton_standby->setEnabled(pushButton_state_para.pushButton_standby);
    ui->pushButton_preheat->setEnabled(pushButton_state_para.pushButton_preheat);
    ui->pushButton_thermo->setEnabled(pushButton_state_para.pushButton_thermo);
    ui->pushButton_evaporation->setEnabled(pushButton_state_para.pushButton_evaporation);
    ui->pushButton_sample->setEnabled(pushButton_state_para.pushButton_sampling);
    ui->pushButton_clear->setEnabled(pushButton_state_para.pushButton_clear);
}

/* user按钮使能计时开始 */
void MainWindow::recei_fro_logic_user_buttton_enable(USER_BUTTON_ENABLE user_button_enable_para)
{
    if(user_button_enable_para.mode == SET_BUTTON_ENABLE_T)//set按钮按下后使能10s
    {
        pushButton_enable_timer->start(user_button_enable_para.enable_time * 1000);
        ui->pushButton_set->setEnabled(true);
        qDebug() << "set pushbutton timer start, " << endl;
    }
    else if(user_button_enable_para.mode == OPEN_BUTTON_ENABLE)//恒温完成，逻辑线程发送信号给GUI线程使能open按钮
    {
        ui->pushButton_open->setEnabled(true);
        qDebug() << "open pushbutton enabled" << endl;
    }
    else if(user_button_enable_para.mode == CLEAR_BUTTON_ENABLE)//清洗完成后，逻辑线程发送信号给GUI线程使能clear、禁能pause、使能done按钮
    {
        ui->pushButton_clear_2->setEnabled(true);
        ui->pushButton_pause->setEnabled(false);
        ui->pushButton_done->setEnabled(true);
        qDebug() << "clear pause plot1/plot2 done pushbutton enabled" << endl;
    }
    else if(user_button_enable_para.mode == PLOT1_ENABLE_PLOT2_DISABLE)//反应室清洗完成后，逻辑线程发送信号, plot1使能，plot2禁能，且此时绘图选项卡绘制清洗数据
    {
        ui->pushButton_plot->setEnabled(true);
        ui->pushButton_plot_2->setEnabled(false);
        qDebug() << "plot1 pushbutton enabled" << endl;
        qDebug() << "plot2 pushbutton disabled" << endl;
        qDebug() << "plot clear data" << endl;
    }
    else if(user_button_enable_para.mode == PLOT1_DISABLE_PLOT2_DISABLE)
    {
        ui->pushButton_plot->setEnabled(false);
        ui->pushButton_plot_2->setEnabled(false);
        qDebug() << "plot1 pushbutton disabled" << endl;
        qDebug() << "plot2 pushbutton disabled" << endl;
        qDebug() << "plot no data!" << endl;
    }
    else if(user_button_enable_para.mode == UNSET)//点击done按钮后，禁能clear、pause、done按钮
    {
        ui->pushButton_clear_2->setEnabled(false);
        ui->pushButton_pause->setEnabled(false);
        ui->pushButton_done->setEnabled(false);
    }

}

/* 按下al-set按键后读取参数面板中的参数并发送给逻辑线程 */
void MainWindow::on_pushButton_al_set_clicked()
{
    qDebug() << "al_set_clicked()" << endl;

    /* 如果定时器还在计时就关掉 */
    if(pushButton_enable_timer->isActive())
        pushButton_enable_timer->stop();

    /* 禁能al_set按钮 */
    ui->pushButton_set->setEnabled(false);
    ui->pushButton_al_set->setEnabled(false);

    system_para_set.preset_temp = ui->set_evapor_temp->value();
    system_para_set.hold_time = ui->set_evapor_time->value();
    system_para_set.sample_size = ui->set_pump_up_time->value();
    system_para_set.sample_freq = ui->set_sample_rate->value();
    system_para_set.sample_time = ui->set_sample_time->value();

    if(ui->radioButton_single->isChecked())
        system_para_set.sample_style = SINGLE;
    if(ui->radioButton_continue->isChecked())
        system_para_set.sample_style = CONTINUE;

    system_para_set.evapor_clear_time = ui->set_evapor_clear->value();
    system_para_set.reac_clear_time = ui->set_reac_clear->value();
    system_para_set.liquor_brand = ui->comboBox_data_filepath->currentText();

    /* 呼吸时间设 */
    system_para_set.pwm_duty[0] = ui->spinBox_14->value();
    system_para_set.pwm_duty[1] = ui->spinBox_15->value();
    system_para_set.pwm_duty[2] = ui->spinBox_16->value();
    system_para_set.pwm_duty[3] = ui->spinBox_17->value();

    system_para_set.inhale_time[0] = ui->spinBox_2->value();
    system_para_set.inhale_time[1] = ui->spinBox_6->value();
    system_para_set.inhale_time[2] = ui->spinBox_10->value();
    system_para_set.inhale_time[3] = ui->spinBox_18->value();

    system_para_set.inhale_wait_time[0] = ui->spinBox_5->value();
    system_para_set.inhale_wait_time[1] = ui->spinBox_7->value();
    system_para_set.inhale_wait_time[2] = ui->spinBox_11->value();
    system_para_set.inhale_wait_time[3] = ui->spinBox_19->value();

    system_para_set.exhale_time[0] = ui->spinBox_4->value();
    system_para_set.exhale_time[1] = ui->spinBox_8->value();
    system_para_set.exhale_time[2] = ui->spinBox_12->value();
    system_para_set.exhale_time[3] = ui->spinBox_20->value();

    system_para_set.exhale_wait_time[0] = ui->spinBox_3->value();
    system_para_set.exhale_wait_time[1] = ui->spinBox_9->value();
    system_para_set.exhale_wait_time[2] = ui->spinBox_13->value();
    system_para_set.exhale_wait_time[3] = ui->spinBox_21->value();

    system_para_set.hale_count = ui->spinBox->value();

    emit send_to_logic_system_para_set(system_para_set);
}

/* 测试采集完成后使能done按钮 */
//void MainWindow::plot_pushbutton_enable()
//{
//    ui->pushButton_plot->setEnabled(true);
//    ui->pushButton_done->setEnabled(true);
//}

void MainWindow::on_pushButton_set_clicked()
{
    qDebug() << "set_clicked()" << endl;

    /* set按钮使能计时, 如果计时器还在计时，则停止计时器 */
    if(pushButton_enable_timer->isActive())
        pushButton_enable_timer->stop();

    /* 点击设置按钮后跳至参数设定选项卡 */
    ui->Qtabwidget->setCurrentIndex(2);

    /* 使能al_set按钮, 禁能set按钮 */
    ui->pushButton_al_set->setEnabled(true);
    ui->pushButton_set->setEnabled(false);
}

void MainWindow::on_pushButton_open_clicked()
{
    qDebug() << "open clicked()" << endl;

    ui->pushButton_open->setEnabled(false);
    ui->pushButton_close->setEnabled(true);
    user_button_enable.mode = OPEN_BUTTON_CLICKED;
    emit send_to_logic_user_button_action(user_button_enable);
}

void MainWindow::on_pushButton_close_clicked()
{
    qDebug() << "close clicked()" << endl;

    ui->pushButton_close->setEnabled(false);
    user_button_enable.mode = CLOSE_BUTTON_CLICKED;
    emit send_to_logic_user_button_action(user_button_enable);
}
void MainWindow::on_pushButton_clear_2_clicked()
{
    qDebug() << "clear clicked()" << endl;

    ui->pushButton_clear_2->setEnabled(false);
    ui->pushButton_pause->setEnabled(true);

    user_button_enable.mode = CLEAR_BUTTON_CLICKED;
    emit send_to_logic_user_button_action(user_button_enable);
}

void MainWindow::on_pushButton_pause_clicked()
{
    qDebug() << "pause clicked()" << endl;

    ui->pushButton_clear_2->setEnabled(true);
    ui->pushButton_pause->setEnabled(false);

    user_button_enable.mode = PAUSE_BUTTON_CLICKED;
    emit send_to_logic_user_button_action(user_button_enable);
}

void MainWindow::on_pushButton_plot_clicked()
{
    qDebug() << "plot1 clicked()" << endl;
    ui->pushButton_plot->setEnabled(false);
    ui->pushButton_plot_2->setEnabled(true);
    qDebug() << "plot sample data" << endl;
//    user_button_enable.mode = PLOT1_BUTTON_CLICKED;
//    emit send_to_logic_user_button_action(user_button_enable);

    /* 发送信号给plot_Widget对象，开始执行绘图 */
    emit send_to_plot_widget(SAMPLE_DATA);

    /* 点击plot1按钮后跳至绘图选项卡 */
    ui->Qtabwidget->setCurrentIndex(5);
}

void MainWindow::on_pushButton_plot_2_clicked()
{
    qDebug() << "plot2 clicked()" << endl;
    ui->pushButton_plot->setEnabled(true);
    ui->pushButton_plot_2->setEnabled(false);
    qDebug() << "plot clear data" << endl;
//    user_button_enable.mode = PLOT2_BUTTON_CLICKED;
//    emit send_to_logic_user_button_action(user_button_enable);

    /* 发送信号给plot_Widget对象，开始执行绘图 */
    emit send_to_plot_widget(CLEAR_DATA);

    /* 点击plot2按钮后跳至绘图选项卡 */
    ui->Qtabwidget->setCurrentIndex(5);
}
void MainWindow::on_pushButton_done_clicked()
{
    qDebug() << "done clicked()" << endl;

    user_button_enable.mode = DONE_BUTTON_CLICKED;
    emit send_to_logic_user_button_action(user_button_enable);
}


void MainWindow::on_pushButton_clear_current_clicked()
{
    if(ui->comboBox_data_filepath->count() == 1 || ui->comboBox_data_filepath->currentText() == "Insert item")
        qDebug() << "you can not delete (Insert item)" << endl;
    else if(ui->comboBox_data_filepath->currentText() == "")
        qDebug() << "you delete nothing" << endl;
    else
    {
        qDebug() << ui->comboBox_data_filepath->currentText() << " was deleted !" << endl;
        ui->comboBox_data_filepath->removeItem(ui->comboBox_data_filepath->currentIndex());
    }
    ui->comboBox_data_filepath->clearEditText();

    qDebug() << "count = " << ui->comboBox_data_filepath->count() << endl;
}

void MainWindow::on_pushButton_clear_all_clicked()
{
    ui->comboBox_data_filepath->clear();
    ui->comboBox_data_filepath->insertItem(0, tr("Insert item"));
    ui->comboBox_data_filepath->insertSeparator(0);
    ui->comboBox_data_filepath->clearEditText();
    qDebug() << "count = " << ui->comboBox_data_filepath->count() << endl;
}
