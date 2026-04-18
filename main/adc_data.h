/**
 * @file rocker.h
 * @author 黄李全 (846863428@qq.com)
 * @brief 摇杆检测
 * @version 0.1
 * @date 2024-04-04
 * @copyright 个人版权所有 Copyright (c) 2023
 */
#ifndef __ADC_DATA_H__
#define __ADC_DATA_H__

#include "esp_adc/adc_continuous.h"

#define ADC_MAX_VALUE    4095    // 12bit ADC
#define ADC_CHANNEL_NUM   2       // ADC通道数量

typedef void (*rocker_callback_t)(const uint32_t*, const uint32_t);

void AdcInit();

uint32_t *GetAdcData();

void SetRockerCallback(rocker_callback_t handler);

#endif
