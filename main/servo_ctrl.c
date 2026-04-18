#include "servo_ctrl.h"
#include "esp_log.h"
#include "driver/ledc.h"

static const char *TAG = "servo";

#define SERVO_MIN_PULSEWIDTH_US 500   // Minimum pulse width in microsecond
#define SERVO_MAX_PULSEWIDTH_US 2500  // Maximum pulse width in microsecond
#define SERVO_MIN_DEGREE        -90   // Minimum angle
#define SERVO_MAX_DEGREE        90    // Maximum angle
#define SERVO_PERIOD_US         20000 // 20ms period for standard servos
#define SERVO_LEDC_FREQ_HZ      50
#define SERVO_LEDC_RESOLUTION   LEDC_TIMER_16_BIT

static const gpio_num_t servo_gpio[6] = {
    GPIO_NUM_21,
    GPIO_NUM_19,
    GPIO_NUM_18,
    GPIO_NUM_5,
    GPIO_NUM_17,
    GPIO_NUM_16,
};

static const ledc_channel_t servo_channel[6] = {
    LEDC_CHANNEL_0,
    LEDC_CHANNEL_1,
    LEDC_CHANNEL_2,
    LEDC_CHANNEL_3,
    LEDC_CHANNEL_4,
    LEDC_CHANNEL_5,
};

static inline uint32_t angle_to_duty(int angle)
{
    if (angle < SERVO_MIN_DEGREE) {
        angle = SERVO_MIN_DEGREE;
    } else if (angle > SERVO_MAX_DEGREE) {
        angle = SERVO_MAX_DEGREE;
    }

    uint32_t pulse_us = SERVO_MIN_PULSEWIDTH_US + (uint32_t)(angle - SERVO_MIN_DEGREE) * (SERVO_MAX_PULSEWIDTH_US - SERVO_MIN_PULSEWIDTH_US) / (SERVO_MAX_DEGREE - SERVO_MIN_DEGREE);
    uint32_t max_duty = (1U << LEDC_TIMER_16_BIT) - 1;
    return (uint32_t)((uint64_t)pulse_us * max_duty / SERVO_PERIOD_US);
}

static void set_servo_duty(int channel_index, int angle)
{
    if (channel_index < 0 || channel_index >= 6) {
        ESP_LOGW(TAG, "Invalid servo channel: %d", channel_index + 1);
        return;
    }

    uint32_t duty = angle_to_duty(angle);
    ESP_ERROR_CHECK(ledc_set_duty(LEDC_HIGH_SPEED_MODE, servo_channel[channel_index], duty));
    ESP_ERROR_CHECK(ledc_update_duty(LEDC_HIGH_SPEED_MODE, servo_channel[channel_index]));
}

void ServoCtrlInit(void)
{
    ESP_LOGI(TAG, "Initializing 6-channel servo PWM");

    ledc_timer_config_t timer_conf = {
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .timer_num = LEDC_TIMER_0,
        .duty_resolution = SERVO_LEDC_RESOLUTION,
        .freq_hz = SERVO_LEDC_FREQ_HZ,
        .clk_cfg = LEDC_AUTO_CLK,
    };
    ESP_ERROR_CHECK(ledc_timer_config(&timer_conf));

    for (int i = 0; i < 6; i++) {
        ledc_channel_config_t ch_conf = {
            .gpio_num = servo_gpio[i],
            .speed_mode = LEDC_HIGH_SPEED_MODE,
            .channel = servo_channel[i],
            .intr_type = LEDC_INTR_DISABLE,
            .timer_sel = LEDC_TIMER_0,
            .duty = 0,
            .hpoint = 0,
        };
        ESP_ERROR_CHECK(ledc_channel_config(&ch_conf));
        set_servo_duty(i, 0);
    }
}

void SetAngle(int angle)
{
    SetServo1(angle);
}

void SetServoAngle(int channel, int angle)
{
    if (channel < 1 || channel > 6) {
        ESP_LOGW(TAG, "Invalid servo channel: %d", channel);
        return;
    }
    set_servo_duty(channel - 1, angle);
}

void SetServo1(int angle) { set_servo_duty(0, angle); }
void SetServo2(int angle) { set_servo_duty(1, angle); }
void SetServo3(int angle) { set_servo_duty(2, angle); }
void SetServo4(int angle) { set_servo_duty(3, angle); }
void SetServo5(int angle) { set_servo_duty(4, angle); }
void SetServo6(int angle) { set_servo_duty(5, angle); }

