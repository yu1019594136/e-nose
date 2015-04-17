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

signals:
    /* 发送给硬件控制线程的恒温信号 */
    void send_to_hard_evapor_thermostat(THERMOSTAT thermostat_signal);

public slots:
    /* 接收来自硬件线程的恒温操作结果 */
    void result_fro_hard_evapor_thermostat(RESULT result);

private slots:

};

#endif // THREAD_LOGIC_H
