#ifndef HW_INTERFACE_H
#define HW_INTERFACE_H
#ifdef __cplusplus
extern "C"
{
#endif

#include "common.h"

/* 上电后系统初始化各个硬件电路，配置操作系统状态，等待用户操作 */
void system_init(void);

//Switch = open,表示打开;Switch = close,表示关闭
void Beep_Switch(int Switch);
void Heat_S_Switch(int Switch);
void Pump_S_Switch(int Switch);
void M1_Switch(int Switch);
void M2_Switch(int Switch);
void M3_Switch(int Switch);
void M4_Switch(int Switch);

void collect_data(void);


/* 退出应用程序，并启动关机进程 */
void Application_quit(int seconds);

/* 操作结束前，
 * 1、程序将保存所有数据文件；
 * 2、关闭各个功能硬件电路，恢复系统配置；
 * 3、退出应用程序，关闭操作系统以及系统电源 */
void system_shutdown(void);

#ifdef __cplusplus
}
#endif

#endif // HW_INTERFACE_H
