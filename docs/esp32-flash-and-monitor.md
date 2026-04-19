# ESP32 编译后运行和查看结果指南

## 编译完成后的操作流程

### 方法一：使用 idf.py（推荐）

这是最简单和推荐的方法：

```bash
# 1. 烧录固件
idf.py -p /dev/cu.usbserial-XXX flash

# 2. 查看运行结果（监控串口输出）
idf.py -p /dev/cu.usbserial-XXX monitor

# 3. 或者一步完成：烧录并监控
idf.py -p /dev/cu.usbserial-XXX flash monitor
```

**退出监控**：按 `Ctrl + ]`

### 方法二：使用 esptool.py（手动方式）

如果你已经使用 esptool 命令编译，继续使用它烧录：

#### 1. 查找串口设备

**macOS:**
```bash
ls /dev/cu.*
```

常见输出：
- `/dev/cu.usbserial-0001`
- `/dev/cu.SLAB_USBtoUART`
- `/dev/cu.wchusbserial14410`

**Linux:**
```bash
ls /dev/ttyUSB*
# 或
ls /dev/ttyACM*
```

**Windows:**
```bash
# 在设备管理器中查看 COM 端口
# 例如: COM3, COM4
```

#### 2. 烧录固件

```bash
# 基本烧录命令
python -m esptool --chip esp32s3 \
    -p /dev/cu.usbserial-XXX \
    -b 460800 \
    --before default_reset \
    --after hard_reset \
    write_flash \
    --flash_mode dio \
    --flash_size 2MB \
    --flash_freq 80m \
    0x0 build/bootloader/bootloader.bin \
    0x8000 build/partition_table/partition-table.bin \
    0x10000 build/hello_world.bin
```

**参数说明：**
- `-p /dev/cu.usbserial-XXX`: 串口设备路径
- `-b 460800`: 波特率（烧录速度）
- `--chip esp32s3`: 目标芯片型号
- `--flash_mode dio`: Flash 模式
- `--flash_size 2MB`: Flash 大小
- `--flash_freq 80m`: Flash 频率
- `0x0`, `0x8000`, `0x10000`: 烧录地址

#### 3. 查看运行结果

烧录完成后，使用串口监控工具查看输出：

**选项 A：使用 idf.py monitor（推荐）**
```bash
idf.py -p /dev/cu.usbserial-XXX monitor
```

**选项 B：使用 screen（macOS/Linux）**
```bash
# 115200 是默认波特率
screen /dev/cu.usbserial-XXX 115200

# 退出 screen：Ctrl + A，然后按 K，再按 Y 确认
```

**选项 C：使用 minicom（Linux）**
```bash
# 安装 minicom
sudo apt-get install minicom

# 运行
minicom -D /dev/ttyUSB0 -b 115200

# 退出：Ctrl + A，然后按 X
```

**选项 D：使用 PuTTY（Windows）**
1. 下载并安装 PuTTY
2. 选择 Serial 连接类型
3. 输入 COM 端口（如 COM3）
4. 设置波特率为 115200
5. 点击 Open

**选项 E：使用 Arduino IDE 串口监视器**
1. 打开 Arduino IDE
2. 工具 → 串口监视器
3. 选择正确的端口
4. 设置波特率为 115200

## 完整的工作流程示例

### 示例 1：Hello World 项目

```bash
# 1. 进入项目目录
cd ~/esp/hello_world

# 2. 设置环境变量
source ~/esp/esp-idf/export.sh

# 3. 配置项目（首次）
idf.py set-target esp32s3

# 4. 编译
idf.py build

# 5. 查找串口
ls /dev/cu.*

# 6. 烧录并监控（一步完成）
idf.py -p /dev/cu.usbserial-0001 flash monitor
```

**预期输出：**
```
Hello world!
This is esp32s3 chip with 2 CPU core(s), WiFi/BLE, silicon revision v0.1, 2MB external flash
Minimum free heap size: 395636 bytes
Restarting in 10 seconds...
Restarting in 9 seconds...
...
```

### 示例 2：分步操作

```bash
# 1. 编译
idf.py build

# 2. 烧录
idf.py -p /dev/cu.usbserial-0001 flash

# 等待烧录完成...
# Hash of data verified.
# Leaving...
# Hard resetting via RTS pin...

# 3. 监控输出
idf.py -p /dev/cu.usbserial-0001 monitor
```

## 常见问题和解决方案

### 问题 1：找不到串口设备

**症状：**
```
serial.serialutil.SerialException: [Errno 2] could not open port /dev/cu.usbserial-XXX
```

**解决方法：**
```bash
# 1. 检查设备是否连接
ls /dev/cu.*

# 2. 检查 USB 线缆（必须是数据线，不是仅充电线）

# 3. 安装驱动（如需要）
# CP210x: https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers
# CH340: https://github.com/adrianmihalko/ch340g-ch34g-ch34x-mac-os-x-driver

# 4. 检查权限（Linux）
sudo usermod -a -G dialout $USER
# 注销后重新登录
```

### 问题 2：烧录失败

**症状：**
```
A fatal error occurred: Failed to connect to ESP32-S3
```

**解决方法：**

**方法 A：手动进入下载模式**
1. 按住开发板上的 **BOOT** 按钮
2. 按一下 **RESET** 按钮
3. 松开 **BOOT** 按钮
4. 重新运行烧录命令

**方法 B：降低波特率**
```bash
# 使用较低的波特率
idf.py -p /dev/cu.usbserial-XXX -b 115200 flash
```

**方法 C：擦除 Flash 后重试**
```bash
idf.py -p /dev/cu.usbserial-XXX erase-flash
idf.py -p /dev/cu.usbserial-XXX flash
```

### 问题 3：串口输出乱码

**症状：**
```
���������������������
```

**解决方法：**

**原因 1：波特率不匹配**
```bash
# 确保使用正确的波特率（默认 115200）
idf.py -p /dev/cu.usbserial-XXX -b 115200 monitor

# 或在 menuconfig 中检查
idf.py menuconfig
# Component config → ESP System Settings → UART console baud rate
```

**原因 2：Flash 频率设置错误**
```bash
idf.py menuconfig
# Serial flasher config → Flash SPI speed
# 尝试降低频率：80MHz → 40MHz
```

### 问题 4：设备不断重启

**症状：**
```
rst:0x10 (RTCWDT_RTC_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
...
Rebooting...
```

**可能原因：**
1. 电源不足 - 使用质量好的 USB 线和电源
2. 看门狗超时 - 检查代码中的死循环
3. 内存溢出 - 检查堆栈大小配置

### 问题 5：监控无输出

**检查清单：**

```bash
# 1. 确认设备已烧录
idf.py -p /dev/cu.usbserial-XXX flash

# 2. 确认设备已重启
# 按一下 RESET 按钮

# 3. 检查串口是否被占用
# 关闭其他串口监控程序

# 4. 尝试不同的监控工具
idf.py monitor
# 或
screen /dev/cu.usbserial-XXX 115200
```

## 监控工具对比

| 工具 | 优点 | 缺点 | 推荐度 |
|------|------|------|--------|
| idf.py monitor | 自动解析崩溃信息、彩色输出、快捷键丰富 | 需要 ESP-IDF 环境 | ⭐⭐⭐⭐⭐ |
| screen | 系统自带、简单快速 | 功能基础、退出不直观 | ⭐⭐⭐⭐ |
| minicom | 功能丰富、配置灵活 | 需要安装、配置复杂 | ⭐⭐⭐ |
| PuTTY | 图形界面、易用 | 仅 Windows | ⭐⭐⭐⭐ |
| Arduino IDE | 图形界面、简单 | 功能有限 | ⭐⭐⭐ |

## idf.py monitor 高级功能

### 常用快捷键

```
Ctrl + ]        退出监控
Ctrl + T        菜单（显示所有快捷键）
Ctrl + R        重启设备
Ctrl + F        查找文本
Ctrl + H        显示帮助
Ctrl + X        退出菜单
Ctrl + P        暂停/恢复输出
```

### 自动解析崩溃信息

当程序崩溃时，`idf.py monitor` 会自动解析堆栈跟踪：

```
Guru Meditation Error: Core  0 panic'ed (LoadProhibited)
...
Backtrace: 0x400d1234:0x3ffb5678 0x400d5678:0x3ffb5690
0x400d1234: app_main at /path/to/main.c:42
0x400d5678: main_task at /path/to/main_task.c:15
```

### 过滤日志输出

```bash
# 只显示错误和警告
idf.py monitor --print-filter="*:E,*:W"

# 只显示特定标签
idf.py monitor --print-filter="MY_TAG:I"
```

## 调试技巧

### 1. 查看详细启动信息

在代码中添加：
```c
#include "esp_log.h"

static const char *TAG = "MAIN";

void app_main(void)
{
    ESP_LOGI(TAG, "Starting application...");
    ESP_LOGI(TAG, "Free heap: %lu", esp_get_free_heap_size());
    ESP_LOGI(TAG, "IDF version: %s", esp_get_idf_version());

    // 你的代码...
}
```

### 2. 使用不同日志级别

```c
ESP_LOGE(TAG, "Error message");      // 红色
ESP_LOGW(TAG, "Warning message");    // 黄色
ESP_LOGI(TAG, "Info message");       // 绿色
ESP_LOGD(TAG, "Debug message");      // 默认颜色
ESP_LOGV(TAG, "Verbose message");    // 灰色
```

### 3. 实时修改日志级别

```bash
idf.py menuconfig
# Component config → Log output → Default log verbosity
# 选择: None/Error/Warning/Info/Debug/Verbose
```

## 保存串口输出到文件

### 方法 1：使用 idf.py

```bash
idf.py monitor | tee output.log
```

### 方法 2：使用重定向

```bash
idf.py monitor > output.log 2>&1
```

### 方法 3：使用 screen

```bash
# 启动 screen 并记录
screen -L -Logfile output.log /dev/cu.usbserial-XXX 115200
```

## 性能监控

### 查看任务状态

在代码中添加：
```c
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void print_task_info(void)
{
    char *task_list = malloc(2048);
    vTaskList(task_list);
    printf("Task Name\tState\tPrio\tStack\tNum\n");
    printf("%s\n", task_list);
    free(task_list);
}
```

### 查看内存使用

```c
#include "esp_heap_caps.h"

void print_memory_info(void)
{
    printf("Free heap: %lu bytes\n", esp_get_free_heap_size());
    printf("Min free heap: %lu bytes\n", esp_get_minimum_free_heap_size());
    printf("Largest free block: %lu bytes\n",
           heap_caps_get_largest_free_block(MALLOC_CAP_8BIT));
}
```

## 快速参考

### 完整命令流程

```bash
# 1. 设置环境
source ~/esp/esp-idf/export.sh

# 2. 编译
idf.py build

# 3. 查找串口
ls /dev/cu.*

# 4. 烧录并监控
idf.py -p /dev/cu.usbserial-XXX flash monitor

# 5. 退出监控
# 按 Ctrl + ]
```

### 常用组合命令

```bash
# 清理、编译、烧录、监控
idf.py fullclean && idf.py build && idf.py -p PORT flash monitor

# 仅编译和烧录应用（快速迭代）
idf.py app-flash monitor

# 擦除 Flash 并重新烧录
idf.py erase-flash flash monitor
```

## 相关文档

- [ESP-IDF 快速参考](esp-idf-quick-reference.md)
- [macOS ESP-IDF 安装指南](macos-esp-idf-installation.md)
- [ESP-IDF 官方监控文档](https://docs.espressif.com/projects/esp-idf/zh_CN/latest/esp32/api-guides/tools/idf-monitor.html)

---

**提示**: 建议使用 `idf.py flash monitor` 一步完成烧录和监控，这是最简单高效的方式！
