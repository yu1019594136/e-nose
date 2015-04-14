/*
 * i2c.h
 *
 *  Created on: 2014年9月23日
 *      Author: zhouyu
 */

#ifndef I2C_H_
#define I2C_H_

#ifdef __cplusplus
extern "C"
{
#endif

void i2cOpen();
void i2cClose();
void i2cSetAddress(int address);
int Readi2c(char REG_Address, char *buf, int len);
void Writei2c(char REG_Address,char REG_data);

/* 使用说明：
 * 1）、打开设备（选择是i2c-0还是i2c-1，通过参数i2cbus_num来选择（0或1）），
 * 如果选则1，SDA和SCL分别接在Pin9_20和Pin9_19，
 * 如果选择0，还需要进行引脚映射操作之后，才能确定i2c-0的引脚，
 * 最后得到设备的文件句柄。
 * 2）、确定从设备地址，接下来的读写操作均针对地址（address）的从设备
 * 3）、进行相应的读写操作
 * 4）、记得操作完后关闭文件句柄，这个操作将会自动关闭设备，取消设备的注册。
 *
 * */

#ifdef __cplusplus
}
#endif

#endif /* I2CDEV_H_ */
