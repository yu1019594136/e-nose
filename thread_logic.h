#ifndef THREAD_LOGIC_H
#define THREAD_LOGIC_H

#include <QThread>
#include <QString>
#include <QTimer>
#include "common.h"

/*********************逻辑控制线程*****************************/
class LogicControlThread : public QThread
{
    Q_OBJECT
public:
    explicit LogicControlThread(QObject *parent = 0);
    void stop();

protected:
    void run();

private:
    volatile bool stopped;
    BEEP beep;//

signals:
    /* 发送给硬件控制线程的恒温信号 */
    void send_to_hard_evapor_thermostat(THERMOSTAT thermostat_signal);

    /* 发送给硬件线程的蜂鸣器控制信号 */
    void send_to_hard_beep(BEEP beep);

public slots:
    /* 处理来自硬件线程恒温完成的信号 */
    void recei_fro_hardware_thermostat_done();

private slots:

};

#endif // THREAD_LOGIC_H
