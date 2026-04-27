#include "business.h"
#include "adc_data.h"
#include "servo_ctrl.h"
#include "moto_ctrl.h"
#include "softap_sta.h"
#include "battery.h"
#include "select.h"
#include "serial.h"
#include "esp_log.h"
#include <sys/param.h>
#include <string.h>
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

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
static bool udp_initialized = false;
static int64_t last_udp_time = 0;  // 上次接收UDP消息的时间戳
static const int64_t TIMEOUT_MS = 1000;  // 超时时间：1秒
void ShowAdcData(const uint32_t* adcs, const uint32_t channal);

static int udp_server_init(uint16_t port)
{
    int s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (s < 0) {
        ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
        return -1;
    }

    struct sockaddr_in local_addr;
    local_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    local_addr.sin_family = AF_INET;
    local_addr.sin_port = htons(port);

    int err = bind(s, (struct sockaddr *)&local_addr, sizeof(local_addr));
    if (err < 0) {
        ESP_LOGE(TAG, "Socket unable to bind: errno %d", errno);
        close(s);
        return -1;
    }

    ESP_LOGI(TAG, "UDP server initialized, listening on port %d", port);
    return s;
}

static void udp_receive_callback(void)
{
    struct RemoteState state;
    struct sockaddr_in source_addr;
    socklen_t socklen = sizeof(source_addr);

    // 检查是否超时
    int64_t current_time = esp_timer_get_time() / 1000;  // 转换为毫秒
    if (last_udp_time > 0 && (current_time - last_udp_time) > TIMEOUT_MS) {
        ESP_LOGW(TAG, "UDP timeout detected, stopping motors and centering servo");
        SetSpeed(0);  // 停止电机
        SetServo1(0);  // 舵机居中
    }

    int len = recvfrom(sock, &state, sizeof(struct RemoteState), 0,
                      (struct sockaddr *)&source_addr, &socklen);
    if (len < 0) {
        ESP_LOGE(TAG, "recvfrom failed: errno %d", errno);
        SetSpeed(0);  // 停止电机
        SetServo1(0);  // 舵机居中
        return;
    }

    if (len != sizeof(struct RemoteState)) {
        ESP_LOGW(TAG, "Received data size mismatch: %d vs %d", len, sizeof(struct RemoteState));
        return;
    }

    // 更新最后接收时间
    last_udp_time = esp_timer_get_time() / 1000;  // 转换为毫秒

    // 控制电机：左摇杆Y轴，范围-1到1，0为停止，映射到-100到100
    int motor_speed = (int)((state.adsry) * 100.0f);
    SetSpeed(motor_speed);

    // 控制舵机：右摇杆X轴，范围-1-1，0对应90度，1对应-90度
    int servo_angle = (int)((state.adsrx) * 90.0f);
    SetServo1(servo_angle);

    // 可选：打印调试信息
    // ESP_LOGI(TAG, "Motor: %d, Servo: %d", motor_speed, servo_angle);
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
    if (abs((int)percent - (int)last_percent) > 3) {
        last_percent = percent;
        printf("battery:%s \n", str);
    }
}

/**
 * 显示设备的MAC地址和IP地址信息
 * @param event 指向ip_event_ap_staipassigned_t结构体的指针，包含MAC和IP信息
 * @param connect 连接状态标志位，虽然当前函数中未使用，但可能用于扩展功能
 */
void ShowIp(ip_event_ap_staipassigned_t* event, bool connect)
{
    char str[64];  // 用于存储格式化后的字符串
    // 格式化MAC地址并打印
    int len = snprintf(str, sizeof(str), MACSTR, MAC2STR(event->mac));
    str[len] = 0;  // 确保字符串正确终止
    printf("MAC: %s \n", str);
    // 格式化IP地址并打印
    len = snprintf(str, sizeof(str), IPSTR, IP2STR(&event->ip));
    str[len] = 0;  // 确保字符串正确终止
    printf("IP: %s \n", str);
    if (connect) {
        printf("Connected to Wi-Fi network\n");
    } else {
        printf("Disconnected from Wi-Fi network\n");
        SetSpeed(0);  // 停止电机
        SetServo1(0);  // 舵机居中
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
    SetStaIpCallback(ShowIp);
    SetUpSta("Remote", "12345678");
    // 建立UDP服务，端口号5555
    // SetUpAp("Remote", "12345678");
    ServoCtrlInit();
    MotoCtrlInit();

    // 初始化UDP服务器
    sock = udp_server_init(5555);
    if (sock >= 0) {
        udp_initialized = true;
        if (SelectAddFd(sock, udp_receive_callback, NULL) != 0) {
            ESP_LOGE(TAG, "Failed to add UDP socket to select");
        }
    }
}
