#ifndef THREAD_DATA_PROC_H
#define THREAD_DATA_PROC_H

#include <QThread>
#include <QString>
#include <QTimer>
#include "common.h"
#include <sys/types.h>


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
    PLOT_INFO plot_info;//绘图尺寸

    volatile bool sample_flag;

    FILE *fp;
    struct tm *tm_ptr;
    time_t the_time;
    char *filename;

    QTimer *sample_timer;

signals:
    void send_to_PlotWidget_plotdata(PLOT_INFO plot_info);

public slots:
    /* 处理来自逻辑线程的采样控制信号 */
    void recei_fro_logic_sample(SAMPLE sample_para);

private slots:
    /* 采样定时溢出 */
    void sample_timeout();

};

/* setitimer定时事件代码 */
void timer_func(int signal_type);

#endif // THREAD_DATA_PROC_H
