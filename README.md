# 一款esp32自制驾驶机器人

6通道舵机，DRV8833驱动板，ESP32主控制器

## 编译

```zsh
source /Volumes/unix/esp-idf/export.sh
idf.py set-target esp32
idf.py build
idf.py -p PORT flash monitor
idf.py -p /dev/tty.usbserial-0001 flash
idf.py -p /dev/tty.usbserial-0001 monitor
```