#ifndef THREAD_HARDWARE_H
#define THREAD_HARDWARE_H

#include <QThread>
#include <QString>
#include <QTimer>
#include "common.h"

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
    QTimer *beep_timer;//鸣叫鸣叫定时控制
    BEEP beep_info;

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
    void recei_fro_logic_thermostat(THERMOSTAT thermostat_signal);

    /* 处理来自GUI线程关闭硬件信号 */
    void recei_fro_GUI_close_hardware();

    /* 处理来自逻辑线程的蜂鸣器控制信号 */
    void recei_fro_logic_beep(BEEP beep);

private slots:
    /* 蜂鸣器定时溢出 */
    void beep_timeout();

};

#endif // THREAD_HARDWARE_H
