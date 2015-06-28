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
#include <signal.h>
#include <error.h>
#include "tlc1543.h"
#include "HW_interface.h"
/*-----------------pru-----------------*/
#include <prussdrv.h>
#include <pruss_intc_mapping.h>

#include <ctype.h>
#include <termios.h>
#include <sys/mman.h>

/*-----------------pru-----------------*/
/* DDR地址 */
unsigned int addr;
unsigned int dataSize;

/* 采样配置相关数据 */
unsigned int spiData[13];
unsigned int timerData[2];

/*********************数据处理线程*****************************/
DataProcessThread::DataProcessThread(QObject *parent) :
    QThread(parent)
{
    stopped = false;

    plot_info.width = 1000;
    plot_info.height = 65536;
    plot_info.sample_count_real = 0;

    filename = NULL;
    sample_flag = false;

}

void DataProcessThread::run()
{
    msleep(100);

    int event_num = 0;

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

            if(sample.sample_inform_flag)
            {
                /* 发送采样完成信号给逻辑线程 */
                emit send_to_logic_sample_done();
            }
            else//此种情况不需要返回信号，系统操作面板中的plot按钮在采集完后需要被使能
            {
                emit send_to_GUI_enable_plot_pushbutton();
            }

            sample_flag = false;
        }

    }
    /* 线程退出前关闭PRU */
    prussdrv_exit ();

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
    prussdrv_exec_program (ADC_PRU_NUM, "./PRUADC.bin");
    prussdrv_exec_program (CLK_PRU_NUM, "./PRUClock.bin");
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
