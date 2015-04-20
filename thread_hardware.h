#ifndef THREAD_HARDWARE_H
#define THREAD_HARDWARE_H

#include <QThread>
#include <QString>
#include <QTimer>
#include "common.h"
#include "pid.h"

/* 需要实时更新的信息 */
typedef struct{
    QString ds18b20_temp;
    QString sht21_temp;
    QString sht21_humid;
} GUI_REALTIME_INFO;

/*********************硬件控制线程*****************************/
class HardWareControlThread : public QThread
{
    Q_OBJECT
public:
    explicit HardWareControlThread(QObject *parent = 0);
    void stop();

protected:
    void run();

private:
    volatile bool stopped;

    THERMOSTAT thermostat;//恒温控制参数
    BEEP beep;//蜂鸣器控制参数
    PUMP pump;//气泵控制相关参数
    MAGNETIC magnetic;//电磁阀状态控制相关参数

    PARA_NUM para_num ;//选用哪一套恒温控制参数
    int flag_inform_duty_0;

    QTimer *beep_timer;//鸣叫鸣叫定时控制
    QTimer *pump_timer;//气泵开启时间

signals:
    /* 将实时采集的温湿度信号发送给GUI线程 */
    void send_to_GUI_realtime_info_update(GUI_REALTIME_INFO);

    /* 恒温时将实时的duty值发送给GUI线程 */
    void send_to_GUI_duty_update(int duty);

    /* 恒温完成 */
    void send_to_logic_thermostat_done();

    /* 返回给逻辑线程恒温的操作结果 */
    void return_to_logic_thermostat(RESULT result);

    /* 返回给GUI线程关闭硬件的操作结果 */
    void return_to_GUI_close_hardware();

public slots:
    /* 处理来自逻辑线程的恒温信号 */
    void recei_fro_logic_thermostat(THERMOSTAT thermostat_para);

    /* 处理来自逻辑线程的蜂鸣器控制信号 */
    void recei_fro_logic_beep(BEEP beep_para);

    /* 处理来自逻辑控制线程的气泵控制信号 */
    void recei_fro_logic_pump(PUMP pump_para);

    /* 处理来自GUI线程关闭硬件信号 */
    void recei_fro_GUI_close_hardware();

private slots:
    /* 蜂鸣器定时溢出 */
    void beep_timeout();

    /* 气泵定时溢出 */
    void pump_timeout();

};

#endif // THREAD_HARDWARE_H