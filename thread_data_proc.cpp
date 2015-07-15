#include "thread_data_proc.h"
#include <QDebug>
#include <QDateTime>

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <signal.h>
#include <error.h>
#include "tlc1543.h"
#include "HW_interface.h"

#include <QStringList>

/*-----------------pru-----------------*/
#include <prussdrv.h>
#include <pruss_intc_mapping.h>

#include <ctype.h>
#include <termios.h>
#include <sys/mman.h>

/* DDR地址 */
unsigned int addr;
unsigned int dataSize;

/* 采样配置相关数据 */
unsigned int spiData[13];
unsigned int timerData[2];

/*-----------------plot-----------------*/
PLOT_INFO p_sample_data;
PLOT_INFO p_clear_data;

/*********************数据处理线程*****************************/
DataProcessThread::DataProcessThread(QObject *parent) :
    QThread(parent)
{
    stopped = false;

    plot_info.width = 0;
    plot_info.height = 65536;
    plot_info.p_data = NULL;

    p_sample_data = plot_info;
    p_clear_data = plot_info;

    p_data_state = 0;//用于指针的管理，样本数据指针和清洗数据指针


    filename = NULL;
    sample_flag = false;

    plot_process = new QProcess();
    plot_process->setReadChannelMode(QProcess::SeparateChannels);
    plot_process->setReadChannel(QProcess::StandardOutput);
    connect(plot_process, SIGNAL(started()), this, SLOT(process_started()));
    connect(plot_process, SIGNAL(error(QProcess::ProcessError)), this, SLOT(process_error(QProcess::ProcessError)));
    connect(plot_process, SIGNAL(readyReadStandardOutput()), this, SLOT(process_readyreadoutput()));
    connect(plot_process, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(process_finished(int,QProcess::ExitStatus)));
    connect(this, SIGNAL(terminate_plot_process()), plot_process, SLOT(terminate()));

    connect(this, SIGNAL(send_to_plot2pdf()), this, SLOT(plot2pdf()));
}

void DataProcessThread::run()
{
    FILE *fp;
    int event_num = 0;

    /* 数据处理线程一开始时就会查询缓存文件plot2pdf_task_file.txt，如果还有绘图任务那么开始新一轮绘图任务 */
    emit send_to_plot2pdf();

    PRU_init_loadcode();

    while (!stopped)
    {
        if(sample_flag)//如果采样已经开始
        {
            event_num = prussdrv_pru_wait_event (PRU_EVTOUT_0);//等待采样结束, 阻塞函数
            qDebug("ADC PRU0 program completed, event number %d.\n", event_num);

            if(prussdrv_pru_clear_event (PRU_EVTOUT_0, PRU0_ARM_INTERRUPT) == 0)
                qDebug("prussdrv_pru_clear_event success\n");
            else
                qDebug("prussdrv_pru_clear_event failed!\n");

            if(prussdrv_pru_disable(ADC_PRU_NUM) == 0)
                 printf("The ADCPRU disable succeed!\n");
            if(prussdrv_pru_disable(CLK_PRU_NUM) == 0)
                 printf("The CLKPRU disable succeed!\n ");

            /* 保存数据到文件 */            
            if(save_data_to_file(filename, spiData[1] / 2) == SUCCESS)
                qDebug("Data is saved in %s.\n", filename);
            else
                qDebug("Data save error!\n");

            /* 将数据文件的内容降频读取到内存块中，准备“在线绘图” */
            plot_info.p_data = data_underclocking_write_to_mem(filename, sample.sample_freq, sample.sample_freq * sample.sample_time, &plot_info.width);

            if(plot_info.p_data)//如果内存分配成功，则发送信号驱动绘图事件
            {
                /* 获取图例名称 */
                plot_info.pic_name = sample.filename_prefix;

                qDebug("Get memory for plot data succeed!\n");
                p_data_state++;
                if(p_data_state == 3)
                {
                    p_data_state = 1;
                }

                if(p_data_state == 1)
                {
                    p_sample_data = plot_info;
                    qDebug("Get the  PLOT_INFO p_sample_data\n");
                }
                else if(p_data_state == 2)
                {
                    p_clear_data = plot_info;
                    qDebug("Get the  PLOT_INFO p_clear_data\n");
                }

                qDebug("p_data_state = %d.\n", p_data_state);

                emit send_to_PlotWidget_plotdata(plot_info);

            }
            else//分配不成功则不会触发绘图事件，点开绘图选项卡将看不到任何内容
            {
                qDebug("Warning: Get memory for plot data failed!\n");
            }

            /* 采集数据完成后是否需要将数据绘图成pdf文件 */
            if(sample.plot_to_pdf)
            {
                qDebug() << "sample.plot_to_pdf = " << sample.plot_to_pdf << endl;

                if((fp = fopen(PLOT_TASK_FILE, "a")) != NULL)
                {
                    fprintf(fp, "%s\n", filename);
                    fflush(fp);
                    fclose(fp);
                    fp = NULL;

                    qDebug("A plot task (%s) is added into plot2pdf_task_file.txt!\n", filename);
                }
                else
                {
                    qDebug("Warning: adding plot task (%s) into plot2pdf_task_file.txt failed!\n", filename);
                }

                emit send_to_plot2pdf();
            }
            else
            {
                qDebug() << "sample.plot_to_pdf = " << sample.plot_to_pdf << endl;
                qDebug("This plot task (%s) is ignored!\n", filename);
            }

            if(sample.sample_inform_flag)
            {
                /* 发送采样完成信号给逻辑线程 */
                emit send_to_logic_sample_done();
            }

            sample_flag = false;
        }

    }
    /* 线程退出前关闭PRU */
    prussdrv_exit ();

    if(p_sample_data.p_data)
        free(p_sample_data.p_data);
    p_sample_data.p_data = NULL;

    if(p_clear_data.p_data)
        free(p_clear_data.p_data);
    p_clear_data.p_data = NULL;

    stopped = false;
    qDebug("DataProcessThread done!\n");
}

void DataProcessThread::stop()
{
    stopped = true;
}

void DataProcessThread::recei_fro_logic_sample(SAMPLE sample_para)
{
    QDateTime datetime = QDateTime::currentDateTime();

    sample.sample_freq = sample_para.sample_freq * 10;//10个通道的频率
    sample.sample_time = sample_para.sample_time;
    sample.filename_prefix = QString("/root/qt_program/") + datetime.toString("yyyy.MM.dd-hh_mm_ss_")+ sample_para.filename_prefix  + ".txt";
    sample.sample_inform_flag = sample_para.sample_inform_flag;
    sample.plot_to_pdf = sample_para.plot_to_pdf;

    ba = sample.filename_prefix.toLatin1();
    filename = ba.data();

    /* 配置采样频率 */
    timerData[0] = (5 * 10000000) / sample.sample_freq - 3;

    /* 配置采样次数(实际上是指定存储空间大小，单位字节，采样在没有剩余空间时自动结束) */
    spiData[1] = sample.sample_freq * sample.sample_time * 2;

    qDebug("The expect sample rate:\t%f\tHz (each channel)\n", sample_para.sample_freq);
    qDebug("The real sample rate:  \t%f\tHz (each channel)\n", (5.0 * 1000000) / (timerData[0] + 3));
    qDebug("memory size in PRU code: spiData[1] = %d\n",spiData[1]);
    qDebug("sample delay in PRU code: timerData[0] = %d\n",timerData[0]);

    /* 写入配置数据到DATA RAM */
    prussdrv_pru_write_memory(PRUSS0_PRU0_DATARAM, 0, spiData, sizeof(spiData));  // spi config
    prussdrv_pru_write_memory(PRUSS0_PRU1_DATARAM, 0, timerData, sizeof(timerData)); // sample config

    /* 使能PRU运行采样程序 */
    if(prussdrv_pru_enable(ADC_PRU_NUM) == 0)
         qDebug("The ADCPRU enable succeed!\n");
    if(prussdrv_pru_enable(CLK_PRU_NUM) == 0)
         qDebug("The CLKPRU enable succeed!\n ");

    sample_flag = true;
    qDebug("sample begins!\n");
}

// Short function to load a single unsigned int from a sysfs entry
unsigned int readFileValue(char filename[])
{
   FILE* fp;
   unsigned int value = 0;
   fp = fopen(filename, "rt");
   fscanf(fp, "%x", &value);
   fclose(fp);
   return value;
}

/* PRU初始化，下载PRU代码到Instruction data ram中 */
void PRU_init_loadcode()
{
    addr = readFileValue(MMAP1_LOC "addr");
    dataSize = readFileValue(MMAP1_LOC "size");

    // Initialize structure used by prussdrv_pruintc_intc
    // PRUSS_INTC_INITDATA is found in pruss_intc_mapping.h
    tpruss_intc_initdata pruss_intc_initdata = PRUSS_INTC_INITDATA;

    // Read in the location and address of the shared memory. This value changes
    // each time a new block of memory is allocated.
    timerData[0] = FREQ_50kHz;
    timerData[1] = RUNNING;
    qDebug("The PRU clock state is set as period: %d (0x%x) and state: %d\n", timerData[0], timerData[0], timerData[1]);
    unsigned int PRU_data_addr = readFileValue(MMAP0_LOC "addr");
    qDebug("-> the PRUClock memory is mapped at the base address: %x\n", (PRU_data_addr + 0x2000));
    qDebug("-> the PRUClock on/off state is mapped at address: %x\n", (PRU_data_addr + 0x10000));

    // data for PRU0 based on the TLC2543 datasheet
    spiData[0] = addr;//readFileValue(MMAP1_LOC "addr")
    spiData[1] = 20;//单位字节，2个字节存储一次采样数据, 采样10次作为测试，主要是为了提前将PRU代码写入Instruction data ram中
    spiData[2] = CHANNEL_WSP2110;
    spiData[3] = CHANNEL_5521;//5521
    spiData[4] = CHANNEL_TGS2611;//TGS2611
    spiData[5] = CHANNEL_TGS2620;
    spiData[6] = CHANNEL_5121;
    spiData[7] = CHANNEL_5526;
    spiData[8] = CHANNEL_5524;
    spiData[9] = CHANNEL_TGS880;
    spiData[10] = CHANNEL_MP502;
    spiData[11] = CHANNEL_TGS2602;
    spiData[12] = CHANNEL_STOP;

    qDebug("The DDR External Memory pool has location: 0x%x and size: 0x%x bytes\n", addr, dataSize);
    qDebug("-> this space has capacity to store %d 16-bit samples (max)\n", dataSize / 2);

    qDebug("Sending the SPI Control Data: 0x%x\n", spiData[2]);
    qDebug("This is the test for sampling and load PRU code into the instruction ram.\n");

    // Allocate and initialize memory
    prussdrv_init ();
    prussdrv_open (PRU_EVTOUT_0);
    // Map the PRU's interrupts
    prussdrv_pruintc_init(&pruss_intc_initdata);

    // Write the address and size into PRU0 Data RAM0. You can edit the value to
    // PRUSS0_PRU1_DATARAM if you wish to write to PRU1
    prussdrv_pru_write_memory(PRUSS0_PRU0_DATARAM, 0, spiData, sizeof(spiData));  // spi code
    prussdrv_pru_write_memory(PRUSS0_PRU1_DATARAM, 0, timerData, sizeof(timerData)); // sample clock

    // Load and execute the PRU program on the PRU
    prussdrv_exec_program (ADC_PRU_NUM, PRUADC_BIN);
    prussdrv_exec_program (CLK_PRU_NUM, PRUClock_BIN);
    qDebug("EBBClock PRU1 program now running (%d )\n", timerData[0]);

    // Wait for event completion from PRU, returns the PRU_EVTOUT_0 number
    int n = prussdrv_pru_wait_event (PRU_EVTOUT_0);
    qDebug("EBBADC PRU0 program test completed, event number %d.\n", n);

    if(prussdrv_pru_clear_event (PRU_EVTOUT_0, PRU0_ARM_INTERRUPT) == 0)
         qDebug("prussdrv_pru_clear_event success\n");
    else
        qDebug("prussdrv_pru_clear_event failed!\n");

    // Disable PRU
    prussdrv_pru_disable(ADC_PRU_NUM);
    prussdrv_pru_disable(CLK_PRU_NUM);

    /* 测试不需要保存数据 */
}

/* 保存数据到文件 */
int save_data_to_file(char * filename, unsigned int numberOutputSamples)
{
    /*--------------------------保存数据相关------------------------------------------*/
     int mmap_fd;
     FILE *fp_data_file;
     unsigned int i = 0;
     void *map_base, *virt_addr;
     unsigned long read_result;
     unsigned int num = 0;
     off_t target = addr;

    /*------------------------------------------------------------------------------------*/

     /*--------------------------保存数据相关------------------------------------------*/
  if((mmap_fd = open("/dev/mem", O_RDWR | O_SYNC)) == -1)
  {
      qDebug("Failed to open memory!");
      return ERROR;
  }

   map_base = mmap(0, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, mmap_fd, target & ~MAP_MASK);
   if(map_base == (void *) -1) {
      qDebug("Failed to map base address");
      return ERROR;
   }
   //fflush(stdout);

   fp_data_file = fopen(filename, "w");

   for(i = 0; i < numberOutputSamples; i++)
   {
       virt_addr = map_base + (target & MAP_MASK);
       read_result = *((uint16_t *) virt_addr);
       num++;
       if(num % 10 == 0)
           fprintf(fp_data_file, "%ld\n", read_result);
       else
           fprintf(fp_data_file, "%ld\t", read_result);

       target+=2;// 2 bytes per sample
   }
   //fflush(stdout);

   if(munmap(map_base, MAP_SIZE) == -1)
   {
      qDebug("Failed to unmap memory");
      return ERROR;
   }
   close(mmap_fd);

   fclose(fp_data_file);

   return SUCCESS;
}

void DataProcessThread::process_started()
{
    qDebug() << "plot_process start" << endl;
}

void DataProcessThread::process_error(QProcess::ProcessError processerror)
{
    qDebug() << "plot_process error = " << processerror << endl;
    emit terminate_plot_process();
    qDebug() << "terminate the failed plot process" << endl;
}

void DataProcessThread::process_readyreadoutput()
{
    qDebug() << plot_process->readAllStandardOutput();
}

/*
 * 当一个绘图进程结束时，将会先检查缓存文件plot2pdf_task_file.txt中的任务数量，
    如果数量大于1，先将任务数读取出来，再报告剩余任务数（不包含第一个已经完成的任务），并将除第一个任务外的其他任务重新写入缓存文件plot2pdf_task_file.txt中。
    如果任务数等于1（即只剩下一个已经完成的任务），则清除文件内容
 */
void DataProcessThread::process_finished(int i, QProcess::ExitStatus exitstate)
{
    //struct stat file_info;
    FILE *fp;
    char *filename_temp;
    char **filename_array;
    unsigned int task_count = 0;
    unsigned int j;

    qDebug() << "plot_process finish" << endl;
    qDebug() << "exit code " << i << endl;
    if(exitstate == QProcess::NormalExit)
        qDebug() << "The process exited normally."<< endl;
    else
        qDebug() << "The process crashed." << endl;

    /* 从plot2pdf_task_file.txt中清除第一个已经完成的绘图任务 */
    filename_temp = (char *)malloc(512 * sizeof(char));

    /* 读取文件中的任务个数 */
    if((fp = fopen(PLOT_TASK_FILE, "r")) != NULL)
    {
        while(!feof(fp))
        {
            fscanf(fp, "%s\n", filename_temp);
            task_count++;
        }
        fclose(fp);
        fp = NULL;

        /* 报告任务剩余数量，不包含第一个已经完成的 */
        qDebug("%d tasks left in plot2pdf_task_file.txt!\n", task_count - 1);
    }
    else
    {
        qDebug("Warning: read task from file failed!\n");
    }

    if(task_count > 0)//
    {
        filename_array = (char **)malloc(task_count * sizeof(char *));

        /* 读取出文件中的任务，并清除第一个任务，在将后续任务重新写入文件 */
        if((fp = fopen(PLOT_TASK_FILE, "r")) != NULL)
        {
            for(j = 0; j < task_count; j++)
            {
                filename_array[j] = (char *)malloc(sizeof(char) * 512);
                memset(filename_array[j], 0, sizeof(char) * 512);
                fscanf(fp, "%s\n", filename_array[j]);
            }
            fclose(fp);
            fp = NULL;
        }
        else
        {
            qDebug("Warning: read task from file failed!\n");
        }

        /* 绘图任务读取出来除第一个任务外，其余任务重新写入文件 */
        if((fp = fopen(PLOT_TASK_FILE, "w")) != NULL)
        {
            for(j = 1; j < task_count; j++)
            {
                fprintf(fp, "%s\n", filename_array[j]);
                qDebug("Task %d: %s\n", j, filename_array[j]);
            }
            fflush(fp);
            fclose(fp);
            fp = NULL;

            qDebug("task list file re-writing succeed!\n");
        }
        else
        {
            qDebug("Warning: task list file re-writing failed!\n");
        }

        free(filename_array);

        if(task_count > 1)
        {
            /* 开始新一轮绘图任务 */
            emit send_to_plot2pdf();
        }
    }
}

/*
plot2pdf()槽函数先检查绘图进程是否空闲，
    如果空闲，那么检查plot2pdf_task_file.txt文件中是否有未完成的绘图任务，
        如果有未完成的绘图任务，那么将启动绘图进程执行任务列表中的第一个绘图任务，退出
        如果没有未完成的绘图任务，那么退出。
    如果不空闲，则报告：任务已经添加至plot2pdf_task_file.txt文件中，稍后执行该绘图任务
*/
void DataProcessThread::plot2pdf()
{
    QStringList arguments;
    struct stat file_info;
    FILE *fp;
    char *filename_temp;

    if(plot_process->state() == QProcess::NotRunning)//进程空闲
    {
        qDebug("plot_process state: Not running. Let's do some plot task!\n");

        /* 读取文件大小信 */
        stat(PLOT_TASK_FILE, &file_info);

        if(file_info.st_size > 0)//有未完成的绘图任务
        {
            filename_temp = (char *)malloc(512 * sizeof(char));

            /* 读取文件中的任务个数 */
            if((fp = fopen(PLOT_TASK_FILE, "r")) != NULL)
            {
                fscanf(fp, "%s\n", filename_temp);
                fclose(fp);
                fp = NULL;
            }
            else
            {
                qDebug("Warning: read task from file failed!\n");
            }

            arguments << QString(filename_temp);

            plot_process->start(PLOT_SCRIPT, arguments);

            qDebug("A plot task (%s) from plot2pdf_task_file.txt is executing ...\n", filename_temp);

            free(filename_temp);
        }
        else//没有未完成的绘图任务
        {
            qDebug("No plot task in plot2pdf_task_file.txt! Waiting new plot task...\n");
        }
    }
    else//进程忙, 绘图任务缓存到文件
    {
        qDebug("This plot task has been added into plot2pdf_task_file.txt, it will be executed later!\n");
    }
}

/* 释放两块内存块，将两个指针清零，发送信号给plot_widget对象，执行一次空指针绘图 */
void DataProcessThread::recei_from_logic_reset_memory()
{
    if(p_sample_data.p_data)
        free(p_sample_data.p_data);
    if(p_clear_data.p_data)
        free(p_clear_data.p_data);

    plot_info.width = 0;
    plot_info.height = 65536;
    plot_info.p_data = NULL;

    p_sample_data = plot_info;
    p_clear_data = plot_info;

    emit send_to_PlotWidget_plotdata(plot_info);
}

/* 将数据文件的内容降频读取到内存块中，准备“在线绘图”
 * char *data_filename      数据文件来源
 * float data_sample_freq	原数据采样频率，如果单通道大于1KHz，则需要降频至1KHz，小于1KHz，则不需要降频
 * int data_sample_count	原数据采样总数，函数将会根据分频因子对数据进行截取
 * int &plot_info_width     记录数据行数
 * 函数返回一个二维内存块指针
 */
unsigned int** data_underclocking_write_to_mem(char *data_filename, float data_sample_freq, int data_sample_count, long *plot_info_width)
{
    float expect_plot_sample_freq = 100.0;//期望的数据绘图频率
    int fre_divi_fac = 1;
    unsigned int i;
    int data_num = 0;
    unsigned int **temp_p_data;
    unsigned int temp_data[10];
    FILE *fp;

    unsigned int lines = (data_sample_count / fre_divi_fac) / 10;

    /* 从文件online_plot_fre.txt读取期望的数据绘图频率 */
    if((fp = fopen(ONLINE_PLOT_FRE, "r")) != NULL)
    {
        fscanf(fp, "online_plot_fre = %fHz\n", &expect_plot_sample_freq);
        fclose(fp);
        fp = NULL;
        qDebug("online_plot_fre = %fHz\n", expect_plot_sample_freq);
    }
    else
    {
        qDebug("cann't open the online_plot_fre.txt, use the plot fre (100Hz) parameter in program.\n");
    }

    /* 计算分频因子 */
    if(data_sample_freq > (10 * expect_plot_sample_freq))//如果数据采集频率单通道超过1KHz，那么“在线绘图”需要对数据降频后再绘图
        fre_divi_fac = data_sample_freq / (10 * expect_plot_sample_freq);
    else//不进行降频
        fre_divi_fac = 1;

    qDebug("fre_divi_fac = %d\n", fre_divi_fac);

    /* 计算存储空间 */
    temp_p_data = (unsigned int **)malloc(sizeof(unsigned int *) * lines);//数据行数
    if(temp_p_data)//指针判断
    {
        for(i = 0; i < lines; i++)
        {
            temp_p_data[i] = (unsigned int *)malloc(sizeof(unsigned int) * 10);
            memset(temp_p_data[i], 0, sizeof(unsigned int) * 10);//将分配的内存空间初始化为0
        }
    }
    else//分配失败则直接返回空指针
    {
        qDebug("Get memory failed!\n");
        return NULL;
    }

    data_num = 0;
    *plot_info_width = 0;
    if((fp = fopen(data_filename, "r")) != NULL)
    {
        while(!feof(fp))
        {
            fscanf(fp, "%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\n", &temp_data[0], &temp_data[1], &temp_data[2], &temp_data[3], &temp_data[4], &temp_data[5], &temp_data[6], &temp_data[7], &temp_data[8], &temp_data[9]);

            if(data_num % fre_divi_fac == 0)
            {
                temp_p_data[*plot_info_width][0] = temp_data[0];
                temp_p_data[*plot_info_width][1] = temp_data[1];
                temp_p_data[*plot_info_width][2] = temp_data[2];
                temp_p_data[*plot_info_width][3] = temp_data[3];
                temp_p_data[*plot_info_width][4] = temp_data[4];
                temp_p_data[*plot_info_width][5] = temp_data[5];
                temp_p_data[*plot_info_width][6] = temp_data[6];
                temp_p_data[*plot_info_width][7] = temp_data[7];
                temp_p_data[*plot_info_width][8] = temp_data[8];
                temp_p_data[*plot_info_width][9] = temp_data[9];

                (*plot_info_width)++;
            }
            data_num++;
        }

        qDebug("plot_info_width = %ld\n", *plot_info_width);

        fclose(fp);
        fp = NULL;

        return temp_p_data;//返回二维内存快指针
    }
    else
    {
        qDebug("read data from %s to **p_data failed!\n", data_filename);
        return NULL;
    }
}

