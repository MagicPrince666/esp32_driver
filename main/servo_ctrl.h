#ifndef __SERVO_CTRL_H__
#define __SERVO_CTRL_H__

#include "driver/ledc.h"
#include "driver/gpio.h"

void ServoCtrlInit(void);
void SetAngle(int angle); // alias for channel 1
void SetServoAngle(int channel, int angle);
void SetServo1(int angle);
void SetServo2(int angle);
void SetServo3(int angle);
void SetServo4(int angle);
void SetServo5(int angle);
void SetServo6(int angle);

#endif
