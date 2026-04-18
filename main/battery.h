#ifndef __BATTERY_H__
#define __BATTERY_H__

#include <stdint.h>
#include "adc_data.h"

// 电池电压检测参数
#define ADC_VREF         1.1     // ADC 参考电压
#define R1               10000    // 分压电阻 R1 = 10K
#define R2               100000    // 分压电阻 R2 = 100K
#define VOLTAGE_RATIO    ((R1 + R2) / (float)R1)  // 分压比 = 6.1
#define BATTERY_MIN      7.2    // 最低电压
#define BATTERY_MAX      8.4    // 满电电压

float AdcToVoltage(uint32_t adc_value);
uint8_t GetBatteryPercent(float voltage);

#endif // __BATTERY_H__

