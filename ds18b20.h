#ifndef __DS18B20_H
#define __DS18B20_H 

#ifdef __cplusplus
extern "C"
{
#endif

/*
 * 根据BB-W1-00A0.dts设备树文件,ds18b20接在P8_7处，不需要外接电阻，IO口已经配置为内部上拉
*/

/* 获取温度数据，字符串格式 */
int DS18B20_Get_Temp(char *w1_device, char *tempString);
/* 获取温度数据，字符串格式, 加小数点 */
void DS18B20_Get_TempString(char *w1_device, char *tempString);
/* 获取温度数据，整型数格式 */
int DS18B20_Get_TempInt(char *w1_device);
/* 获取温度数据，单精度浮点数格式 */
float DS18B20_Get_TempFloat(char *w1_device);

#ifdef __cplusplus
}
#endif

#endif
