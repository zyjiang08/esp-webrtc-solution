# ESP32-S3-Korvo-2 V3.1 摄像头识别和配置指南

## 📊 重要说明

**ESP32-S3-Korvo-2 V3.1 主板不自带摄像头**，只提供一个 14-pin 摄像头接口。

如果你购买的是套装版本，可能会附带摄像头模组。本指南将帮助你识别摄像头型号并正确配置。

## 🔍 识别摄像头型号的方法

### 方法 1：物理检查（最直接）⭐

**步骤**:
1. 查看摄像头模组的 PCB 板
2. 寻找芯片上的丝印标识
3. 常见标识：
   - `OV2640` - 200万像素
   - `OV3660` - 300万像素
   - `OV5640` - 500万像素
   - `SC2336` - 200万像素（ESP32-P4 常用）

**示例图片位置**:
- 芯片通常在摄像头镜头下方
- 可能需要放大镜才能看清小字

### 方法 2：通过 I2C 地址识别

不同摄像头有不同的 I2C 地址：

| 摄像头型号 | I2C 地址（7-bit） | I2C 地址（8-bit） | Product ID |
|-----------|------------------|------------------|------------|
| **OV2640** | 0x30 | 0x60 | 0x26 |
| **OV3660** | 0x3C | 0x78 | 0x36 |
| **OV5640** | 0x3C | 0x78 | 0x56 |
| **SC2336** | 0x30 | 0x60 | - |

**ESP32-S3-Korvo-2 的 I2C 配置**:
- **SDA**: GPIO17
- **SCL**: GPIO18
- **频率**: 100 kHz

### 方法 3：使用检测程序

我已经为你创建了一个摄像头检测程序：`/Users/jiangzhongyang/work/esp32/camera_detect.c`

**使用方法**:

1. **创建测试项目**:
```bash
cd ~/work/esp32
cp -r ~/esp/esp-idf/examples/get-started/hello_world camera_test
cd camera_test
```

2. **添加检测代码**:
```bash
# 复制检测程序到项目
cp ~/work/esp32/camera_detect.c main/
```

3. **修改 main.c**:
```c
#include "camera_detect.c"

void app_main(void)
{
    detect_camera();
}
```

4. **编译和运行**:
```bash
idf.py set-target esp32s3
idf.py build
idf.py -p /dev/cu.SLAB_USBtoUART flash monitor
```

**预期输出**:
```
I (1234) CAMERA_DETECT: === ESP32-S3-Korvo-2 Camera Detection ===
I (1235) CAMERA_DETECT: I2C initialized successfully
I (1236) CAMERA_DETECT: Scanning I2C bus...
I (1237) CAMERA_DETECT: Found device at address: 0x30
I (1238) CAMERA_DETECT: Found 1 I2C device(s)

I (1239) CAMERA_DETECT: Trying to detect OV2640 at 0x30...
I (1240) CAMERA_DETECT: OV2640 detected!
I (1241) CAMERA_DETECT:   Product ID: 0x26
I (1242) CAMERA_DETECT:   Version:    0x42
I (1243) CAMERA_DETECT: ✓ Confirmed: OV2640 camera
```

### 方法 4：查看购买信息

检查你的购买订单或包装盒：
- 套装通常会标明包含的摄像头型号
- 查看乐鑫官方商店的产品描述
- 联系卖家确认

## 📦 常见摄像头模组规格

### OV2640（最常见）

| 规格 | 参数 |
|------|------|
| **分辨率** | 最高 1600x1200 (UXGA) |
| **接口** | DVP (并行) |
| **帧率** | 15 fps @ 1600x1200<br>30 fps @ 800x600 |
| **I2C 地址** | 0x30 (7-bit) |
| **电压** | 3.3V |
| **特点** | 低成本，广泛使用 |

**推荐配置**:
```c
#define CAMERA_MODEL_OV2640
#define CAMERA_WIDTH  640
#define CAMERA_HEIGHT 480
#define CAMERA_FPS    15
```

### OV3660

| 规格 | 参数 |
|------|------|
| **分辨率** | 最高 2048x1536 (QXGA) |
| **接口** | DVP (并行) |
| **帧率** | 15 fps @ 2048x1536<br>30 fps @ 1024x768 |
| **I2C 地址** | 0x3C (7-bit) |
| **电压** | 3.3V |
| **特点** | 更高分辨率 |

**推荐配置**:
```c
#define CAMERA_MODEL_OV3660
#define CAMERA_WIDTH  1024
#define CAMERA_HEIGHT 768
#define CAMERA_FPS    15
```

### OV5640

| 规格 | 参数 |
|------|------|
| **分辨率** | 最高 2592x1944 (5MP) |
| **接口** | DVP (并行) |
| **帧率** | 15 fps @ 2592x1944<br>30 fps @ 1920x1080 |
| **I2C 地址** | 0x3C (7-bit) |
| **电压** | 3.3V |
| **特点** | 高分辨率，自动对焦 |

**推荐配置**:
```c
#define CAMERA_MODEL_OV5640
#define CAMERA_WIDTH  1920
#define CAMERA_HEIGHT 1080
#define CAMERA_FPS    15
```

## 🔧 ESP WebRTC 中的摄像头配置

### 配置 doorbell_demo

编辑 `solutions/doorbell_demo/main/settings.h`:

```c
// WiFi 配置
#define WIFI_SSID     "你的WiFi名称"
#define WIFI_PASSWORD "你的WiFi密码"

// 摄像头配置
// 根据你的摄像头型号选择一个：

// 选项 1: OV2640（最常见）
#define CAMERA_MODEL_OV2640
#define CAMERA_WIDTH  640
#define CAMERA_HEIGHT 480
#define CAMERA_FPS    15

// 选项 2: OV3660
// #define CAMERA_MODEL_OV3660
// #define CAMERA_WIDTH  1024
// #define CAMERA_HEIGHT 768
// #define CAMERA_FPS    15

// 选项 3: OV5640
// #define CAMERA_MODEL_OV5640
// #define CAMERA_WIDTH  1920
// #define CAMERA_HEIGHT 1080
// #define CAMERA_FPS    15

// JPEG 质量（0-100，越高质量越好但带宽需求越大）
#define JPEG_QUALITY  60
```

### ESP32-S3 性能建议

基于 ESP32-S3 的性能（双核 240 MHz + 8 MB PSRAM）：

| 分辨率 | 帧率 | JPEG 质量 | 带宽需求 | CPU 占用 | 推荐 |
|--------|------|----------|---------|---------|------|
| 320x240 | 30 fps | 60 | ~150 Kbps | ~40% | ✅ 流畅 |
| 640x480 | 15 fps | 60 | ~300 Kbps | ~60% | ✅ 推荐 |
| 640x480 | 30 fps | 60 | ~600 Kbps | ~80% | ⚠️ 较高 |
| 1024x768 | 15 fps | 60 | ~500 Kbps | ~70% | ⚠️ 较高 |
| 1920x1080 | 15 fps | 60 | ~800 Kbps | ~90% | ❌ 不推荐 |

**推荐配置**（平衡性能和质量）:
```c
#define CAMERA_WIDTH  640
#define CAMERA_HEIGHT 480
#define CAMERA_FPS    15
#define JPEG_QUALITY  60
```

## 🐛 常见问题

### 问题 1: 摄像头无法初始化

**症状**:
```
E (3000) camera: Camera init failed with error 0x105
```

**可能原因**:
1. 摄像头未连接或连接不良
2. 摄像头型号配置错误
3. 电源不足

**解决方案**:
1. 检查 14-pin 连接器是否插紧
2. 运行 I2C 扫描程序确认摄像头地址
3. 确保使用 5V/2A 电源适配器
4. 尝试不同的摄像头型号配置

### 问题 2: I2C 扫描找不到设备

**症状**:
```
W (1000) CAMERA_DETECT: No I2C devices found!
```

**解决方案**:
1. **检查硬件连接**:
   - 摄像头是否插入 14-pin 接口
   - 接口方向是否正确（注意防呆设计）

2. **检查电源**:
   - 确保电源开关已打开
   - 红色电源 LED 应该亮起

3. **检查 I2C 配置**:
   ```c
   #define I2C_MASTER_SDA_IO  17  // 必须是 GPIO17
   #define I2C_MASTER_SCL_IO  18  // 必须是 GPIO18
   ```

### 问题 3: 图像质量差

**症状**: 图像模糊、有噪点、颜色不正常

**解决方案**:
1. **调整 JPEG 质量**:
   ```c
   #define JPEG_QUALITY  80  // 提高到 80
   ```

2. **检查光线条件**:
   - 确保有足够的光线
   - 避免逆光拍摄

3. **调整摄像头焦距**:
   - 某些摄像头模组有可调焦距
   - 旋转镜头调整清晰度

### 问题 4: 帧率低

**症状**: 视频卡顿，帧率低于预期

**解决方案**:
1. **降低分辨率**:
   ```c
   #define CAMERA_WIDTH  320
   #define CAMERA_HEIGHT 240
   ```

2. **降低 JPEG 质量**:
   ```c
   #define JPEG_QUALITY  50
   ```

3. **检查 WiFi 信号**:
   ```bash
   # 在串口监控中查看信号强度
   wifi
   # RSSI 应该 > -70 dBm
   ```

## 📝 快速测试流程

### 1. 硬件检查

```
✓ 摄像头模组已连接到 14-pin 接口
✓ USB 电源线已连接（5V/2A）
✓ USB 串口线已连接
✓ 电源开关已打开（红色 LED 亮起）
```

### 2. 运行 I2C 扫描

```bash
cd ~/work/esp32/camera_test
idf.py -p /dev/cu.SLAB_USBtoUART flash monitor
```

**预期结果**:
- 找到设备地址 0x30（OV2640 或 SC2336）
- 或找到设备地址 0x3C（OV3660 或 OV5640）

### 3. 配置 WebRTC 示例

```bash
cd ~/work/esp32/esp-webrtc-solution/solutions/doorbell_demo
```

编辑 `main/settings.h`，根据检测到的摄像头型号配置。

### 4. 编译和测试

```bash
idf.py set-target esp32s3
idf.py build
idf.py -p /dev/cu.SLAB_USBtoUART flash monitor
```

### 5. 浏览器验证

1. 等待设备连接 WiFi
2. 记录房间号（如 `esp123456`）
3. 浏览器访问: https://webrtc.espressif.com/doorbell
4. 输入房间号并加入
5. 应该能看到摄像头视频流

## 📖 参考资料

### 官方文档

- [ESP32-S3-Korvo-2 用户指南](https://docs.espressif.com/projects/esp-adf/zh_CN/latest/design-guide/dev-boards/user-guide-esp32-s3-korvo-2.html)
- [ESP WebRTC 解决方案](https://github.com/espressif/esp-webrtc-solution)
- [OV2640 数据手册](https://www.uctronics.com/download/cam_module/OV2640DS.pdf)

### 本地文档

- [ESP32-S3-Korvo-2 V3.1 开发板分析](esp32-s3-korvo-2-analysis.md)
- [ESP WebRTC 快速开始](esp-webrtc-quick-start.md)

## 🆘 获取帮助

如果仍然无法识别摄像头：

1. **拍照发送**:
   - 拍摄摄像头模组的清晰照片
   - 特别是芯片上的标识
   - 发送到 ESP32 论坛求助

2. **联系卖家**:
   - 提供订单号
   - 询问具体的摄像头型号

3. **ESP32 论坛**:
   - https://esp32.com/
   - 搜索 "ESP32-S3-Korvo-2 camera"

---

**文档版本**: 1.0
**创建日期**: 2026-04-14
**适用硬件**: ESP32-S3-Korvo-2 V3.1
