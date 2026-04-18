#ifndef __BUSINESS_H__
#define __BUSINESS_H__

#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <unistd.h>
#include <sys/lock.h>
#include <sys/param.h>

#define IIC_SDA             GPIO_NUM_26
#define IIC_SCL             GPIO_NUM_27

#ifdef __cplusplus
extern "C" {
#endif

void InitAll(void);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif
