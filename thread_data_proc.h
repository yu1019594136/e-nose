#ifndef THREAD_DATA_PROC_H
#define THREAD_DATA_PROC_H

#include <QThread>
#include <QString>
#include <QTimer>
#include "common.h"
#include "qcommon.h"
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
    char *filename;
    QTimer *sample_timer;

signals:
    void send_to_PlotWidget_plotdata(PLOT_INFO plot_info);

    /* 发送采样完成信号给逻辑线程 */
    void send_to_logic_sample_done();

    /* 此种情况不需要返回信号，系统操作面板中的plot按钮在采集完后需要被使能 */
    void send_to_GUI_enable_plot_pushbutton();

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
