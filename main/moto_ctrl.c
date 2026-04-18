#include "moto_ctrl.h"
#include "esp_log.h"

#define MOTO_PWM_IN1             GPIO_NUM_22
#define MOTO_PWM_IN2             GPIO_NUM_23
#define MOTO_PWM_TIMER           LEDC_TIMER_1
#define MOTO_PWM_CHANNEL_FWD     LEDC_CHANNEL_6
#define MOTO_PWM_CHANNEL_REV     LEDC_CHANNEL_7
#define MOTO_PWM_SPEED_MODE      LEDC_HIGH_SPEED_MODE
#define MOTO_PWM_FREQ_HZ         10000
#define MOTO_PWM_RESOLUTION      LEDC_TIMER_10_BIT

static const char *TAG = "moto";

static uint32_t speed_to_duty(int speed)
{
    if (speed < 0) {
        speed = -speed;
    }
    if (speed > 100) {
        speed = 100;
    }
    return (uint32_t)speed * ((1 << MOTO_PWM_RESOLUTION) - 1) / 100;
}

void MotoCtrlInit(void)
{
    ledc_timer_config_t timer_conf = {
        .speed_mode = MOTO_PWM_SPEED_MODE,
        .timer_num = MOTO_PWM_TIMER,
        .duty_resolution = MOTO_PWM_RESOLUTION,
        .freq_hz = MOTO_PWM_FREQ_HZ,
        .clk_cfg = LEDC_AUTO_CLK,
    };
    ESP_ERROR_CHECK(ledc_timer_config(&timer_conf));

    ledc_channel_config_t ch_fwd = {
        .gpio_num = MOTO_PWM_IN1,
        .speed_mode = MOTO_PWM_SPEED_MODE,
        .channel = MOTO_PWM_CHANNEL_FWD,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = MOTO_PWM_TIMER,
        .duty = 0,
        .hpoint = 0,
    };
    ESP_ERROR_CHECK(ledc_channel_config(&ch_fwd));

    ledc_channel_config_t ch_rev = {
        .gpio_num = MOTO_PWM_IN2,
        .speed_mode = MOTO_PWM_SPEED_MODE,
        .channel = MOTO_PWM_CHANNEL_REV,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = MOTO_PWM_TIMER,
        .duty = 0,
        .hpoint = 0,
    };
    ESP_ERROR_CHECK(ledc_channel_config(&ch_rev));

    ESP_LOGI(TAG, "Motor driver initialized: IN1=%d IN2=%d", MOTO_PWM_IN1, MOTO_PWM_IN2);
}

void SetSpeed(int speed)
{
    if (speed > 100) {
        speed = 100;
    } else if (speed < -100) {
        speed = -100;
    }

    uint32_t duty_fwd = 0;
    uint32_t duty_rev = 0;

    if (speed > 0) {
        duty_fwd = speed_to_duty(speed);
    } else if (speed < 0) {
        duty_rev = speed_to_duty(speed);
    }

    ESP_ERROR_CHECK(ledc_set_duty(MOTO_PWM_SPEED_MODE, MOTO_PWM_CHANNEL_FWD, duty_fwd));
    ESP_ERROR_CHECK(ledc_update_duty(MOTO_PWM_SPEED_MODE, MOTO_PWM_CHANNEL_FWD));
    ESP_ERROR_CHECK(ledc_set_duty(MOTO_PWM_SPEED_MODE, MOTO_PWM_CHANNEL_REV, duty_rev));
    ESP_ERROR_CHECK(ledc_update_duty(MOTO_PWM_SPEED_MODE, MOTO_PWM_CHANNEL_REV));
}
