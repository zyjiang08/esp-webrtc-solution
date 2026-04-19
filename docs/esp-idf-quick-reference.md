# ESP-IDF 快速参考指南

## 环境设置

每次打开新终端都需要运行：

```bash
cd ~/esp/esp-idf
source ./export.sh
```

或使用别名（添加到 `~/.zshrc` 或 `~/.bash_profile`）：

```bash
alias get_idf='. ~/esp/esp-idf/export.sh'
```

## 常用命令

### 项目配置

```bash
# 设置目标芯片
idf.py set-target esp32s3

# 打开配置菜单
idf.py menuconfig

# 清理配置
idf.py fullclean
```

### 编译和烧录

```bash
# 编译项目
idf.py build

# 烧录固件
idf.py -p /dev/cu.usbserial-XXX flash

# 监控串口输出
idf.py monitor

# 组合命令：编译、烧录、监控
idf.py build flash monitor

# 仅编译和烧录应用（不包括 bootloader）
idf.py app
idf.py app-flash
```

### 清理和擦除

```bash
# 清理构建文件
idf.py clean

# 完全清理（包括配置）
idf.py fullclean

# 擦除整个 Flash
idf.py erase-flash
```

### 调试和分析

```bash
# 查看二进制文件大小
idf.py size
idf.py size-components
idf.py size-files

# 生成编译数据库（用于 IDE）
idf.py reconfigure

# 显示分区表
idf.py partition-table
```

## 串口设备查找

### macOS

```bash
ls /dev/cu.*
```

常见设备名：
- `/dev/cu.usbserial-*`
- `/dev/cu.SLAB_USBtoUART`
- `/dev/cu.wchusbserial*`

### Linux

```bash
ls /dev/ttyUSB*
ls /dev/ttyACM*
```

### Windows

```bash
# 在设备管理器中查看 COM 端口
# 或使用命令
mode
```

## 项目结构

```
my_project/
├── CMakeLists.txt          # 项目 CMake 配置
├── main/
│   ├── CMakeLists.txt      # 主组件配置
│   ├── main.c              # 主程序入口
│   └── Kconfig.projbuild   # 项目配置选项
├── components/             # 自定义组件
├── sdkconfig               # 项目配置（自动生成）
├── sdkconfig.defaults      # 默认配置
└── partitions.csv          # 分区表（可选）
```

## 创建新项目

### 方法一：从示例复制

```bash
cd ~/esp
cp -r $IDF_PATH/examples/get-started/hello_world my_project
cd my_project
idf.py set-target esp32s3
idf.py build
```

### 方法二：使用模板

```bash
cd ~/esp
idf.py create-project my_project
cd my_project
idf.py set-target esp32s3
```

## 常见问题快速修复

### 1. 找不到 idf.py

```bash
source ~/esp/esp-idf/export.sh
```

### 2. 串口权限错误（Linux）

```bash
sudo usermod -a -G dialout $USER
# 注销后重新登录
```

### 3. 编译错误：找不到头文件

```bash
idf.py fullclean
idf.py reconfigure
idf.py build
```

### 4. 烧录失败

```bash
# 检查串口设备
ls /dev/cu.*

# 尝试不同的波特率
idf.py -p /dev/cu.usbserial-XXX -b 115200 flash

# 手动进入下载模式
# 按住 BOOT 按钮，按一下 RESET 按钮，松开 BOOT
```

### 5. 监控乱码

```bash
# 设置正确的波特率
idf.py -p /dev/cu.usbserial-XXX -b 115200 monitor

# 退出监控：Ctrl + ]
```

## 环境变量

安装后自动设置的重要环境变量：

```bash
echo $IDF_PATH          # ESP-IDF 路径
echo $IDF_TOOLS_PATH    # 工具链路径
echo $PATH              # 包含工具链的 PATH
```

## 支持的芯片

| 芯片 | 架构 | 主频 | Flash | RAM |
|------|------|------|-------|-----|
| ESP32 | Xtensa 双核 | 240 MHz | 4 MB | 520 KB |
| ESP32-S2 | Xtensa 单核 | 240 MHz | 4 MB | 320 KB |
| ESP32-S3 | Xtensa 双核 | 240 MHz | 8 MB | 512 KB |
| ESP32-C3 | RISC-V 单核 | 160 MHz | 4 MB | 400 KB |
| ESP32-C6 | RISC-V 单核 | 160 MHz | 4 MB | 512 KB |
| ESP32-H2 | RISC-V 单核 | 96 MHz | 4 MB | 320 KB |

## 配置选项位置

常用配置菜单路径：

```
idf.py menuconfig

Component config →
  ├── FreeRTOS              # RTOS 配置
  ├── ESP System Settings   # 系统设置
  ├── Wi-Fi                 # Wi-Fi 配置
  ├── Bluetooth             # 蓝牙配置
  └── Log output            # 日志级别

Serial flasher config →     # 烧录配置
  ├── Flash size
  ├── Flash frequency
  └── Flash mode

Partition Table →           # 分区表配置
```

## 日志级别

在代码中使用：

```c
#include "esp_log.h"

static const char *TAG = "MY_APP";

ESP_LOGE(TAG, "Error");      // 错误
ESP_LOGW(TAG, "Warning");    // 警告
ESP_LOGI(TAG, "Info");       // 信息
ESP_LOGD(TAG, "Debug");      // 调试
ESP_LOGV(TAG, "Verbose");    // 详细
```

在 menuconfig 中设置全局日志级别：
```
Component config → Log output → Default log verbosity
```

## 有用的工具

```bash
# 查看 ESP-IDF 版本
idf.py --version

# 查看工具链版本
xtensa-esp32s3-elf-gcc --version

# 查看 Python 环境
which python
python --version

# 查看已安装的 Python 包
pip list | grep esp

# 更新组件管理器
pip install --upgrade idf-component-manager
```

## 在线资源

- **官方文档**: https://docs.espressif.com/projects/esp-idf/zh_CN/latest/
- **API 参考**: https://docs.espressif.com/projects/esp-idf/zh_CN/latest/api-reference/
- **示例代码**: `$IDF_PATH/examples/`
- **论坛**: https://esp32.com/
- **GitHub**: https://github.com/espressif/esp-idf

## 更新 ESP-IDF

```bash
cd ~/esp/esp-idf
git pull
git submodule update --init --recursive
./install.sh all  # 或指定芯片
```

## 性能优化

### 编译速度优化

```bash
# 使用 ccache
brew install ccache
idf.py menuconfig
# Compiler options → Enable ccache

# 使用多核编译（默认已启用）
idf.py build -j8  # 使用 8 个核心
```

### 运行时优化

在 `idf.py menuconfig` 中：
- `Compiler options → Optimization Level` → Release (-O2)
- `Component config → FreeRTOS → Tick rate (Hz)` → 调整为合适值
- `Component config → ESP System Settings → CPU frequency` → 240 MHz

---

**提示**: 将此文件保存为书签，方便快速查阅！
