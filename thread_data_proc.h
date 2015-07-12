#ifndef THREAD_DATA_PROC_H
#define THREAD_DATA_PROC_H

#include <QThread>
#include <QString>
#include <QTimer>
#include "common.h"
#include "qcommon.h"
#include <sys/types.h>

/*-----------------plot-----------------*/
#include <QProcess>

/*-----------------pru-----------------*/
#define ADC_PRU_NUM	   0   // using PRU0 for the ADC capture
#define CLK_PRU_NUM	   1   // using PRU1 for the sample clock
#define MMAP0_LOC   "/sys/class/uio/uio0/maps/map0/"
#define MMAP1_LOC   "/sys/class/uio/uio0/maps/map1/"

#define MAP_SIZE 0x0FFFFFFF
#define MAP_MASK (MAP_SIZE - 1)

#define CHANNEL_WSP2110	0x0c000000
#define CHANNEL_5521	0x1c000000
#define CHANNEL_TGS2611	0x2c000000
#define CHANNEL_TGS2620	0x3c000000
#define CHANNEL_5121	0x4c000000
#define CHANNEL_5526	0x5c000000
#define CHANNEL_5524	0x6c000000
#define CHANNEL_TGS880	0x7c000000
#define CHANNEL_MP502	0x8c000000
#define CHANNEL_TGS2602	0x9c000000
#define CHANNEL_STOP 	0x00000000		//通道截止标记

/* FREQ_xHz = delay
如何根据所需要的Hz，计算出n？
delay + 3 = (5 * 10^7) / f

或者
f = (5 * 10^7) / (delay + 3)

f单位是Hz
*/
enum FREQUENCY {    // measured and calibrated, but can be calculated
    FREQ_12_5MHz =  1,
    FREQ_6_25MHz =  5,
    FREQ_5MHz    =  7,
    FREQ_3_85MHz = 10,
    FREQ_1MHz   =  47,
    FREQ_500kHz =  97,
    FREQ_250kHz = 245,
    FREQ_100kHz = 497,
    FREQ_50kHz = 997,
    FREQ_40kHz = 1247,
    FREQ_25kHz = 1997,
    FREQ_10kHz = 4997,
    FREQ_5kHz =  9997,
    FREQ_4kHz = 12497,
    FREQ_2kHz = 24997,
    FREQ_1kHz = 49997
};

enum CONTROL {
    PAUSED = 0,
    RUNNING = 1,
    UPDATE = 3
};

/*-----------------pru-----------------*/


/*********************数据处理线程*****************************/
class DataProcessThread : public QThread
{
    Q_OBJECT
public:
    explicit DataProcessThread(QObject *parent = 0);

    /* 绘图函数 */
    void plot_func(QString argument);

    void stop();

protected:
    void run();

private:
    volatile bool stopped;
    SAMPLE sample;//采样控制参数

    PLOT_INFO plot_info;//绘图尺寸
    int p_data_state;//用于指针的管理，样本数据指针和清洗数据指针

    char *filename;
    QByteArray ba;
    bool sample_flag;

    QProcess *plot_process;

signals:
    void send_to_PlotWidget_plotdata(PLOT_INFO plot_info);

    /* 发送采样完成信号给逻辑线程 */
    void send_to_logic_sample_done();

    /* 此种情况不需要返回信号，系统操作面板中的plot按钮在采集完后需要被使能 */
//    void send_to_GUI_enable_plot_pushbutton();

    /* 发送信号驱使槽函数绘图pdf文件 */
    void send_to_plot2pdf();

public slots:
    /* 处理来自逻辑线程的采样控制信号 */
    void recei_fro_logic_sample(SAMPLE sample_para);

    /* 释放两块内存块，将两个指针清零，发送信号给plot_widget对象，执行一次空指针绘图 */
    void recei_from_logic_reset_memory();

private slots:

    void process_started();

    void process_finished(int,QProcess::ExitStatus);

    void process_error(QProcess::ProcessError);

    void process_readyreadoutput();

    void plot2pdf();

};

// Short function to load a single unsigned int from a sysfs entry
unsigned int readFileValue(char filename[]);

/* PRU初始化，下载PRU代码到Instruction data ram中 */
void PRU_init_loadcode();

/* 保存数据到文件 */
int save_data_to_file(char * filename, unsigned int numberOutputSamples);

/* 将数据文件的内容降频读取到内存块中，准备“在线绘图”
 * char *data_filename      数据文件来源
 * float data_sample_freq	原数据采样频率，如果单通道大于1KHz，则需要降频至1KHz，小于1KHz，则不需要降频
 * int data_sample_count	原数据采样总数，函数将会根据分频因子对数据进行截取
 * int &plot_info_width     记录数据行数
 * 函数返回一个二维内存块指针
 */

unsigned int** data_underclocking_write_to_mem(char *data_filename, float data_sample_freq, int data_sample_count, long *plot_info_width);

#endif // THREAD_DATA_PROC_H
