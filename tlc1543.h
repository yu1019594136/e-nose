/*
 * tlc1543.h
 *
 *  Created on: 2014年10月19日
 *      Author: zhouyu
 */

#ifndef TLC1543_H_
#define TLC1543_H_

#ifdef __cplusplus
extern "C"
{
#endif

#define WAIT_CONVERSION 50000
void tlc1543_Init(int gpiocs);
void tlc1543_Transfer(uint16_t *TxBuf, uint16_t *RxBuf, int len);
//void tlc1543_Transfer_half_duplex(uint8_t *TxBuf, uint8_t *RxBuf, int len);
void tlc1543_Close();

#ifdef __cplusplus
}
#endif

#endif /* TLC1543_H_ */
