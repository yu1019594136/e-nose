#ifndef THREAD_DATA_PROC_H
#define THREAD_DATA_PROC_H

#include <QThread>
#include <QString>
#include <QTimer>
#include "common.h"


/* setitimer定时事件代码 */
void timer_func(int signal_type);

/*********************数据处理线程*****************************/
class DataProcessThread : public QThread
{
    Q_OBJECT
public:
    explicit DataProcessThread(QObject *parent = 0);
    void stop();

protected:
    void run();

private:
    volatile bool stopped;
    SAMPLE sample;//采样控制参数

signals:

public slots:
    /* 处理来自逻辑线程的采样控制信号 */
    void recei_fro_logic_sample(SAMPLE sample_para);

};

#endif // THREAD_DATA_PROC_H
