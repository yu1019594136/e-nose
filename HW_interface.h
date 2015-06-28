#ifndef HW_INTERFACE_H
#define HW_INTERFACE_H

#include "common.h"
#include "thread_hardware.h"

/* 上电后系统初始化各个硬件电路，配置操作系统状态，等待用户操作 */
void init_hardware();

//Switch = open,表示打开;Switch = close,表示关闭
void Beep_Switch(int Switch);
void Heat_S_Switch(int Switch);
void Pump_S_Switch(int Switch);
void M1_Switch(int Switch);
void M2_Switch(int Switch);
void M3_Switch(int Switch);
void M4_Switch(int Switch);

void get_realtime_info(GUI_REALTIME_INFO *realtime_info);

/* 退出应用程序，并启动关机进程 */
void Application_quit(int seconds);

/* 操作结束前，
 * 关闭各个功能硬件电路，恢复系统配置；
 */
void close_hardware(void);

#endif // HW_INTERFACE_H
