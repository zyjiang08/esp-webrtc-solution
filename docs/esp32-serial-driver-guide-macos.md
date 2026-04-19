# ESP32 串口驱动安装指南（macOS）

本文档基于 ESP-IDF 官方文档整理，专门针对 macOS 系统。

## 📌 串口连接方式

ESP32 系列芯片与 PC 的连接方式有以下几种：

### 1. 使用 USB 至 UART 桥（最常见）

大多数开发板内置 USB 至 UART 桥芯片：
- PC 和桥之间通过 USB 连接
- 桥和 ESP32 之间通过 UART 连接

**常见芯片型号：**
- **CP2102/CP2104** - Silicon Labs（最常见）
- **CH340G/CH9102** - WCH（便宜的开发板）
- **FT232RL** - FTDI（高端开发板）

### 2. 使用原生 USB（ESP32-S2/S3/C3/C6/H2）

部分芯片支持原生 USB，无需外部桥芯片：
- ESP32-S3: GPIO19 (D+), GPIO20 (D-)
- ESP32-S2: GPIO19 (D+), GPIO20 (D-)
- ESP32-C3: GPIO18 (D+), GPIO19 (D-)
- ESP32-C6: GPIO12 (D+), GPIO13 (D-)

## 🔧 驱动安装

### CP210x 驱动安装（推荐优先安装）

ESP32 官方开发板大多使用 CP2102 芯片。

#### 下载驱动

访问 Silicon Labs 官网：
```bash
open https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers
```

或使用本地驱动（如果已下载）：
```bash
# 如果驱动在 /Users/jiangzhongyang/esp/macOS_VCP_Driver/
open /Users/jiangzhongyang/esp/macOS_VCP_Driver/SiLabsUSBDriverDisk.dmg
```

#### 安装步骤

1. 下载 "CP210x VCP Mac OSX Driver"
2. 双击 `.dmg` 文件挂载
3. 运行 "Install CP210x VCP Driver" 应用
4. 按照安装向导操作
5. 输入管理员密码
6. 如果出现安全提示：
   - 打开 **系统偏好设置 → 安全性与隐私**
   - 点击底部的 **"允许"** 按钮（允许来自 Silicon Labs 的软件）
7. **重启 Mac**（必须！）

#### 验证安装

```bash
# 检查驱动是否加载
kextstat | grep -i silabs

# 应该看到：
# com.silabs.driver.CP210xVCPDriver
```

### CH340 驱动安装（备选）

如果你的开发板使用 CH340G 或 CH9102 芯片。

#### 下载驱动

```bash
open https://github.com/adrianmihalko/ch340g-ch34g-ch34x-mac-os-x-driver/releases
```

#### 安装步骤

1. 下载最新的 `.pkg` 文件
2. 双击运行安装程序
3. 按照向导完成安装
4. 可能需要在系统偏好设置中允许
5. **重启 Mac**

#### 验证安装

```bash
# 检查驱动是否加载
kextstat | grep -i wch

# 应该看到：
# com.wch.usbserial
```

## 🔍 查看串口设备

### 连接 ESP32 后查看

```bash
# 查看所有串口设备
ls /dev/cu.*

# 过滤掉蓝牙设备
ls /dev/cu.* | grep -v BLTH
```

### 常见设备名称

**CP210x 驱动：**
```
/dev/cu.SLAB_USBtoUART
/dev/cu.usbserial-0001
```

**CH340 驱动：**
```
/dev/cu.wchusbserial14410
/dev/cu.wchusbserial1420
```

**原生 USB：**
```
/dev/cu.usbmodem14101
```

### 识别新设备的方法

```bash
# 1. 断开 ESP32，运行：
ls /dev/cu.* > /tmp/before.txt

# 2. 连接 ESP32，等待 2-3 秒

# 3. 再次运行：
ls /dev/cu.* > /tmp/after.txt

# 4. 对比差异（新出现的就是 ESP32）
diff /tmp/before.txt /tmp/after.txt
```

## 🚀 使用串口

### 烧录固件

```bash
# 基本命令
idf.py -p /dev/cu.SLAB_USBtoUART flash

# 指定波特率（可选，默认 460800）
idf.py -p /dev/cu.SLAB_USBtoUART -b 115200 flash

# 烧录并监控
idf.py -p /dev/cu.SLAB_USBtoUART flash monitor
```

### 监控串口输出

使用 `idf.py monitor`（推荐）：
```bash
idf.py -p /dev/cu.SLAB_USBtoUART monitor

# 退出：Ctrl + ]
```

使用 `screen` 命令：
```bash
# 默认波特率 115200
screen /dev/cu.SLAB_USBtoUART 115200

# 退出：Ctrl + A，然后按 K，再按 Y
```

## ⚠️ 常见问题

### 问题 1：安装驱动后没有设备

**原因：** 未重启 Mac 或驱动未加载

**解决：**
```bash
# 1. 确认已重启 Mac
sudo reboot

# 2. 检查驱动是否加载
kextstat | grep -i "silabs\|wch"

# 3. 如果没有，手动加载
sudo kextload /Library/Extensions/SiLabsUSBDriver.kext
```

### 问题 2：macOS 阻止驱动加载

**原因：** macOS 安全设置阻止了第三方驱动

**解决：**
1. 打开 **系统偏好设置 → 安全性与隐私 → 通用**
2. 查看底部是否有："来自开发人员的系统软件..."
3. 开发者名称为 Silicon Labs 或 FTDI
4. 点击 **"允许"** 按钮
5. 重启 Mac

### 问题 3：串口被占用

**错误信息：**
```
[Errno 16] Resource busy: '/dev/cu.SLAB_USBtoUART'
```

**解决：**
```bash
# 1. 查找占用串口的进程
lsof | grep cu.SLAB_USBtoUART

# 2. 关闭其他串口监控程序（如 screen, minicom）

# 3. 如果 screen 会话未正常退出
screen -ls
screen -X -S [session_id] quit
```

### 问题 4：串口输出乱码

**原因：** 波特率不匹配

**解决：**
```bash
# ESP32 默认波特率为 115200
# ESP32-C2 (40MHz XTAL) 为 115200
# ESP32-C2 (26MHz XTAL) 为 74880

# 使用正确的波特率
idf.py -p /dev/cu.SLAB_USBtoUART -b 115200 monitor
```

### 问题 5：无法进入下载模式

**解决：** 手动进入下载模式
1. 按住开发板上的 **BOOT** 按钮
2. 按一下 **RESET** 按钮
3. 松开 **BOOT** 按钮
4. 重新运行烧录命令

## 📋 验证串口连接

### 使用 screen 测试

```bash
# 1. 连接串口
screen /dev/cu.SLAB_USBtoUART 115200

# 2. 按 ESP32 的 RESET 按钮

# 3. 应该看到启动日志：
# rst:0x5 (DEEPSLEEP_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
# ...
```

### 使用 esptool 测试

```bash
# 测试芯片连接
python -m esptool --port /dev/cu.SLAB_USBtoUART chip_id

# 应该显示芯片信息：
# Detecting chip type... ESP32-S3
# Chip is ESP32-S3 (revision vX.X)
```

## 🔧 高级配置

### 设置默认串口

在项目目录创建或编辑 `sdkconfig.defaults`：
```
CONFIG_ESPTOOLPY_PORT="/dev/cu.SLAB_USBtoUART"
CONFIG_ESPTOOLPY_BAUD_115200B=y
```

### 使用环境变量

```bash
# 设置默认串口
export ESPPORT=/dev/cu.SLAB_USBtoUART
export ESPBAUD=115200

# 然后可以省略 -p 参数
idf.py flash monitor
```

添加到 `~/.zshrc`（永久生效）：
```bash
echo 'export ESPPORT=/dev/cu.SLAB_USBtoUART' >> ~/.zshrc
echo 'export ESPBAUD=115200' >> ~/.zshrc
source ~/.zshrc
```

## 📊 不同芯片的串口配置

| 芯片 | 默认波特率 | USB 支持 | 常见驱动 |
|------|-----------|---------|---------|
| ESP32 | 115200 | 否 | CP2102 |
| ESP32-S2 | 115200 | 是 | CP2102 或原生 |
| ESP32-S3 | 115200 | 是 | CP2102 或原生 |
| ESP32-C2 (40MHz) | 115200 | 否 | CP2102 |
| ESP32-C2 (26MHz) | 74880 | 否 | CP2102 |
| ESP32-C3 | 115200 | 是 | CP2102 或原生 |
| ESP32-C6 | 115200 | 是 | CP2102 或原生 |
| ESP32-H2 | 115200 | 是 | CP2102 或原生 |

## 🛠️ 故障排查清单

在寻求帮助前，请确认：

- [ ] 已安装对应的驱动程序
- [ ] 安装驱动后已重启 Mac
- [ ] 在系统偏好设置中允许了驱动加载
- [ ] ESP32 已通过 USB 连接到 Mac
- [ ] 开发板电源 LED 亮起
- [ ] USB 线支持数据传输（不是仅充电线）
- [ ] 运行 `ls /dev/cu.*` 能看到新设备
- [ ] 没有其他程序占用串口
- [ ] 使用了正确的波特率

## 📚 参考资源

- [ESP-IDF 官方文档 - 建立串口连接](https://docs.espressif.com/projects/esp-idf/zh_CN/latest/esp32/get-started/establish-serial-connection.html)
- [CP210x 驱动下载](https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers)
- [CH340 驱动下载](https://github.com/adrianmihalko/ch340g-ch34g-ch34x-mac-os-x-driver)
- [esptool 文档](https://docs.espressif.com/projects/esptool/)

---

**提示：** 大多数 ESP32 开发板使用 CP2102 芯片，优先安装 CP210x 驱动。安装后必须重启 Mac！
