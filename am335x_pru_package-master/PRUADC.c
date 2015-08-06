/*  This program loads the two PRU programs into the PRU-ICSS transfers the configuration
*   to the PRU memory spaces and starts the execution of both PRU programs.
*   pressed. By Derek Molloy, for the book Exploring BeagleBone. Please see:
*        www.exploringbeaglebone.com/chapter13
*   for a full description of this code example and the associated programs.
*/

#include <stdio.h>
#include <stdlib.h>
#include <prussdrv.h>
#include <pruss_intc_mapping.h>

#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#include <ctype.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/mman.h>

#define ADC_PRU_NUM	   0   // using PRU0 for the ADC capture
#define CLK_PRU_NUM	   1   // using PRU1 for the sample clock
#define MMAP0_LOC   "/sys/class/uio/uio0/maps/map0/"
#define MMAP1_LOC   "/sys/class/uio/uio0/maps/map1/"

#define MAP_SIZE 0x0FFFFFFF
#define MAP_MASK (MAP_SIZE - 1)

#define CHANNEL_WSP2110	0x0c000000
#define CHANNEL_5521			0x1c000000
#define CHANNEL_TGS2611	0x2c000000
#define CHANNEL_TGS2620	0x3c000000
#define CHANNEL_5121			0x4c000000
#define CHANNEL_5526			0x5c000000
#define CHANNEL_5524			0x6c000000
#define CHANNEL_TGS880		0x7c000000
#define CHANNEL_MP502		0x8c000000
#define CHANNEL_TGS2602	0x9c000000
#define CHANNEL_STOP 		0x00000000		//通道截止标记

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

// Short function to load a single unsigned int from a sysfs entry
unsigned int readFileValue(char filename[]){
   FILE* fp;
   unsigned int value = 0;
   fp = fopen(filename, "rt");
   fscanf(fp, "%x", &value);
   fclose(fp);
   return value;
}

int main (void)
{
   if(getuid()!=0){
      printf("You must run this program as root. Exiting.\n");
      exit(EXIT_FAILURE);
   }
   
   /*--------------------------保存数据相关------------------------------------------*/
    int mmap_fd;
	FILE *fp_data_file;
	int i = 0;
    void *map_base, *virt_addr;
    unsigned long read_result, writeval;
    unsigned int addr = readFileValue(MMAP1_LOC "addr");
    unsigned int dataSize = readFileValue(MMAP1_LOC "size");
    unsigned int numberOutputSamples = 2000;//dataSize * 2
	unsigned int num = 0;
    off_t target = addr;
   
   /*------------------------------------------------------------------------------------*/
   
   // Initialize structure used by prussdrv_pruintc_intc
   // PRUSS_INTC_INITDATA is found in pruss_intc_mapping.h
   tpruss_intc_initdata pruss_intc_initdata = PRUSS_INTC_INITDATA;

   // Read in the location and address of the shared memory. This value changes
   // each time a new block of memory is allocated.
   float freq = 100.0;
   
   unsigned int timerData[2];
   timerData[0] = FREQ_50kHz;
   //timerData[0] = (5 * 10^7) / (freq * 10) - 3;
   timerData[1] = RUNNING;
   printf("The PRU clock state is set as period: %d (0x%x) and state: %d\n", timerData[0], timerData[0], timerData[1]);
   unsigned int PRU_data_addr = readFileValue(MMAP0_LOC "addr");
   printf("-> the PRUClock memory is mapped at the base address: %x\n", (PRU_data_addr + 0x2000));
   printf("-> the PRUClock on/off state is mapped at address: %x\n", (PRU_data_addr + 0x10000));

   // data for PRU0 based on the MCPXXXX datasheet
   unsigned int spiData[13];   
   spiData[0] = addr;//readFileValue(MMAP1_LOC "addr")
   spiData[1] = dataSize;//readFileValue(MMAP1_LOC "size")
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
   
   printf("Sending the SPI Control Data: 0x%x\n", spiData[2]);
   printf("The DDR External Memory pool has location: 0x%x and size: 0x%x bytes\n", spiData[0], spiData[1]);
   int numberSamples = spiData[1]/2;
   printf("-> this space has capacity to store %d 16-bit samples (max)\n", numberSamples);

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
   printf("EBBClock PRU1 program now running (%d )\n", timerData[0]);

   // Wait for event completion from PRU, returns the PRU_EVTOUT_0 number
   int n = prussdrv_pru_wait_event (PRU_EVTOUT_0);
   printf("EBBADC PRU0 program completed, event number %d.\n", n);
   
   if(prussdrv_pru_clear_event (PRU_EVTOUT_0, PRU0_ARM_INTERRUPT) == 0)
		printf("prussdrv_pru_clear_event success\n"); 
   
// Disable PRU and close memory mappings 
   prussdrv_pru_disable(ADC_PRU_NUM);
   prussdrv_pru_disable(CLK_PRU_NUM);
   
      /*--------------------------保存数据相关------------------------------------------*/
   if((mmap_fd = open("/dev/mem", O_RDWR | O_SYNC)) == -1)
   {
	printf("Failed to open memory!");
	return -1;
    }
    fflush(stdout);

    map_base = mmap(0, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, mmap_fd, target & ~MAP_MASK);
    if(map_base == (void *) -1) {
       printf("Failed to map base address");
       return -1;
    }
    fflush(stdout);
	
	fp_data_file = fopen("data1.txt", "w");
	numberOutputSamples = 20000;
	target = addr;
    for(i=0; i<numberOutputSamples; i++)
	{
		virt_addr = map_base + (target & MAP_MASK);
        read_result = *((uint16_t *) virt_addr);
        //printf("Value at address 0x%X (%p): 0x%X\n", target, virt_addr, read_result);
        //printf("%d %d\n",i, read_result);
		fprintf(fp_data_file, "%d", read_result);
		num++;
		
			if(num % 10 == 0)
				fprintf(fp_data_file, "\n");
			else
				fprintf(fp_data_file, "\t");
			
        target+=2;                   // 2 bytes per sample
    }
    fflush(stdout);

    if(munmap(map_base, MAP_SIZE) == -1) {
       printf("Failed to unmap memory");
       return -1;
    }
    close(mmap_fd);
	
	fclose(fp_data_file);
	/*--------------------------------------------------------------------------*/
	spiData[1] = dataSize / 10;//readFileValue(MMAP1_LOC "size") 单位字节 / 10
	spiData[2] = CHANNEL_WSP2110;
	spiData[3] = CHANNEL_TGS2620;
	spiData[4] = CHANNEL_STOP;
	timerData[0] = FREQ_50kHz;
	//timerData[0] = (5 * 10^7) / (freq * 10) - 3;
	timerData[1] = RUNNING;
   
   prussdrv_pru_write_memory(PRUSS0_PRU0_DATARAM, 0, spiData, sizeof(spiData));  // spi code
   prussdrv_pru_write_memory(PRUSS0_PRU1_DATARAM, 0, timerData, sizeof(timerData)); // sample clock

   if(prussdrv_pru_enable(ADC_PRU_NUM) == 0) 
		printf("the second ADC enable succeed!  ADC PRU\n");
   if(prussdrv_pru_enable(CLK_PRU_NUM) == 0)
		printf("the second CLK enable succeed!  CLK PRU\n ");
   
      // Wait for event completion from PRU, returns the PRU_EVTOUT_0 number
   n = prussdrv_pru_wait_event (PRU_EVTOUT_0);
   printf("EBBADC PRU0 program completed, event number %d.\n", n);
   
      if(prussdrv_pru_clear_event (PRU_EVTOUT_0, PRU0_ARM_INTERRUPT) == 0)
		printf("the second prussdrv_pru_clear_event success\n"); 
   
// Disable PRU and close memory mappings 
   if(prussdrv_pru_disable(ADC_PRU_NUM) == 0) 
		printf("the second disable succeed!  ADC PRU\n");
   if(prussdrv_pru_disable(CLK_PRU_NUM) == 0)
		printf("the second disable succeed!  CLK PRU\n ");
	
/*--------------------------保存数据相关------------------------------------------*/
   if((mmap_fd = open("/dev/mem", O_RDWR | O_SYNC)) == -1)
   {
	printf("Failed to open memory!");
	return -1;
    }
    fflush(stdout);

    map_base = mmap(0, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, mmap_fd, target & ~MAP_MASK);
    if(map_base == (void *) -1) {
       printf("Failed to map base address");
       return -1;
    }
    fflush(stdout);
	
	fp_data_file = fopen("data2.txt", "w");
	numberOutputSamples = 20000;
	target = addr;
    for(i=0; i<numberOutputSamples; i++)
	{
		virt_addr = map_base + (target & MAP_MASK);
        read_result = *((uint16_t *) virt_addr);
        //printf("Value at address 0x%X (%p): 0x%X\n", target, virt_addr, read_result);
        //printf("%d %d\n",i, read_result);
		fprintf(fp_data_file, "%d", read_result);
		num++;
		
			if(num % 2 == 0)
				fprintf(fp_data_file, "\n");
			else
				fprintf(fp_data_file, "\t");
			
        target+=2;                   // 2 bytes per sample
    }
    fflush(stdout);

    if(munmap(map_base, MAP_SIZE) == -1) {
       printf("Failed to unmap memory");
       return -1;
    }
    close(mmap_fd);
	
	fclose(fp_data_file);
	
	/*--------------------------------------------------------------------------*/
   prussdrv_exit ();
   return EXIT_SUCCESS;
}
