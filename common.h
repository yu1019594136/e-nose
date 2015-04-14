/*
 * common.h
 *
 *  Created on: 2014年10月4日
 *      Author: zhouyu
 */

#ifndef COMMON_H_
#define COMMON_H_

#ifdef __cplusplus
extern "C"
{
#endif

enum RESULT{
	ERROR = 0,
	SUCCESS
};

enum SWITCH{
    CLOSE = 0,
    OPEN
};

enum THERMO{
    STOP = 0,
    START
};

#ifdef __cplusplus
}
#endif

#endif /* COMMON_H_ */
