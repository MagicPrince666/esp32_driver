#include "business.h"
#include "adc_data.h"
#include "servo_ctrl.h"
#include "softap_sta.h"
#include "battery.h"
#include "select.h"
#include "serial.h"
#include "esp_log.h"
#include <sys/param.h>
#include <string.h>

// const uint16_t adc_range[5] = {4095, 4095, 3821, 3740, 4095}; // 遥控1参数
const uint16_t adc_range[5] = {4095, 3780, 3970, 3640, 4095}; // 遥控2参数
struct RemoteState {
    bool lose_signal;   // 失控标识

    bool front; // 前进按钮
    bool back; // 后退按钮
    bool left; // 左转按钮
    bool right; // 右转按钮
    bool select; // 选择按钮
    bool start; // 开始按钮
    bool l1; // 左顶部按钮1
    bool l2; // 左顶部按钮2
    bool r1; // 右顶部按钮1
    bool r2; // 右顶部按钮2
    bool adl; // 左摇杆按钮
    bool adr; // 右摇杆按钮
    bool triangle; // 三角按钮
    bool quadrilateral; // 四边形按钮
    bool rotundity; // 园形按钮
    bool fork; // 叉按钮
    float adslx;   // 左摇杆x轴
    float adsly;   // 左摇杆y轴
    float adsrx;   // 右摇杆x轴
    float adsry;   // 右摇杆y轴
    float ads[12];   // 扩展通道 sbus 16路通道都是模拟量
};

static const char *TAG = "peripherals";
static int sock = -1;  // UDP socket
static struct sockaddr_in dest_addr;
static bool udp_initialized = false;
void ShowAdcData(const uint32_t* adcs, const uint32_t channal);

static int udp_init(const char *ip, uint16_t port)
{
    int s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (s < 0) {
        ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
        return -1;
    }

    dest_addr.sin_addr.s_addr = inet_addr(ip);
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(port);

    ESP_LOGI(TAG, "UDP initialized, target: %s:%d", ip, port);
    return s;
}

// 发送RemoteState数据
static void send_remote_state(const struct RemoteState *state)
{
    if (!udp_initialized || sock < 0) {
        return;
    }

    int err = sendto(sock, state, sizeof(struct RemoteState), 0, 
                    (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    if (err < 0) {
        // ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
    }
}

void ShowAdcData(const uint32_t* adcs, const uint32_t channal)
{
    char str[32];
    static uint8_t last_percent = 0;
    float voltage = AdcToVoltage(adcs[0]);
    uint8_t percent = GetBatteryPercent(voltage);

    // 更新百分比显示
    int len = snprintf(str, 32, "%3d%%", percent);
    str[len] = 0;
    if (percent != last_percent) {
        last_percent = percent;
        printf("battery:%s \n", str);
    }
}

void ShowIp(ip_event_ap_staipassigned_t* event, bool connect)
{
    char str[64];
    int len = snprintf(str, sizeof(str), MACSTR, MAC2STR(event->mac));
    str[len] = 0;
    printf("MAC: %s \n", str);
    len = snprintf(str, sizeof(str), IPSTR, IP2STR(&event->ip));
    str[len] = 0;
    printf("IP: %s \n", str);

    if (connect) {
        if (sock > 0) {
            return;
        }
        sock = udp_init("192.168.34.168", 5555);
        if (sock >= 0) {
            udp_initialized = true;
        }
    } else {
        if (sock >= 0) {
            close(sock);
            udp_initialized = false;
        }
    }
}

void ShowIpAndConnect(ip_event_ap_staipassigned_t* event, bool connect)
{
    char str[64];
    int len = snprintf(str, sizeof(str), MACSTR, MAC2STR(event->mac));
    str[len] = 0;
    printf("MAC: %s \n", str);
    len = snprintf(str, sizeof(str), IPSTR, IP2STR(&event->ip));
    str[len] = 0;
    printf("IP: %s \n", str);
    if (connect) {
        // 初始化UDP连接到对端的5555端口
        if (sock > 0) {
            return;
        }
        char ip_str[16];
        snprintf(ip_str, sizeof(ip_str), IPSTR, IP2STR(&event->ip));
        sock = udp_init(ip_str, 5555);
        if (sock >= 0) {
            udp_initialized = true;
        }
    } else {
        if (sock >= 0) {
            close(sock);
            udp_initialized = false;
        }
    }
}

void InitAll(void)
{
    SelectInit();
    // Serial(UART_NUM_1, TXD1_PIN, RXD1_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    AdcInit();
    SetRockerCallback(ShowAdcData);
    
    // 等待一段时间让ADC稳定后进行中位值校准
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    
    SoftApStaInit();
    SetIpCallback(ShowIpAndConnect);
    SetStaIpCallback(ShowIp);
    SetUpSta("Remote", "12345678");
    // SetUpAp("Remote", "12345678");
    PwmCtrlInit();
}
