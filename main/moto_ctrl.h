#ifndef __MOTO_CTRL_H__
#define __MOTO_CTRL_H__

#include "driver/ledc.h"
#include "driver/gpio.h"

void MotoCtrlInit(void);
void SetSpeed(int speed); // x小于0为后退，x大于0为前进

#endif
