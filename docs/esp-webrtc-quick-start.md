# ESP WebRTC 解决方案快速开始指南

## 📊 概述

**ESP WebRTC 解决方案 v1.0** 是乐鑫科技专为轻量级嵌入式设备设计的 WebRTC 实现。本指南将帮助你在 ESP32-S3-Korvo-2 V3.1 开发板上快速运行 WebRTC 应用。

**发布日期**: 2025年5月15日
**官方仓库**: https://github.com/espressif/esp-webrtc-solution

### 🔑 关键信息

**无需搭建服务器！** Espressif 提供免费的公共 WebRTC 信令服务器：
- **信令服务器**: `wss://webrtc.espressif.com:8089/ws`
- **Web 测试界面**: `https://webrtc.espressif.com`
- **支持通信方式**: ESP32 ↔ ESP32 / ESP32 ↔ 浏览器 / ESP32 ↔ 移动 App

## 🎯 功能特性

### 核心功能

- ✅ **高级 esp_webrtc API**: 简化 WebRTC 应用开发
- ✅ **点对点通信**: 支持 RTP 和 SCTP 协议
- ✅ **媒体和数据通道**: 音频/视频流 + 数据通道通信
- ✅ **TURN 支持**: 完整的 NAT 穿透支持
- ✅ **灵活信令**: 支持 AppRTC、WHIP、OpenAI、本地 HTTP SSE
- ✅ **公共信令服务器**: Espressif 提供免费的 AppRTC 服务器（无需自建）

### 音视频编解码器

| 类型 | 支持格式 |
|------|---------|
| **视频编码** | H.264 (基线配置), MJPEG |
| **音频编码** | OPUS, G.711 (PCMA/PCMU), AAC |

### 支持的芯片

| 芯片 | 支持状态 | 说明 |
|------|---------|------|
| **ESP32-S3** | ✅ 完全支持 | 需要 PSRAM（你的 Korvo-2 有 8 MB PSRAM ✅） |
| **ESP32-P4** | ✅ 完全支持 | 官方推荐，性能最强 |
| **ESP32-S2** | ✅ 支持 | 仅 WiFi，无蓝牙 |
| **ESP32** | ✅ 支持 | 经典版本 |

## 🔧 硬件要求

### ESP32-S3-Korvo-2 V3.1 兼容性分析

你的开发板配置：
- ✅ **芯片**: ESP32-S3 (双核 240 MHz)
- ✅ **Flash**: 16 MB（远超 4 MB 最低要求）
- ✅ **PSRAM**: 8 MB（满足视频/音频处理需求）
- ✅ **WiFi**: 802.11 b/g/n（支持 WebRTC 传输）
- ✅ **音频**: ES7210 ADC + ES8311 Codec（完整音频支持）
- ⚠️ **摄像头**: 需要外接（通过 14-pin 连接器）

**结论**: ESP32-S3-Korvo-2 V3.1 完全支持 ESP WebRTC 解决方案！

### 推荐摄像头模组

如果需要视频功能，推荐以下摄像头：

| 型号 | 分辨率 | 接口 | 说明 |
|------|--------|------|------|
| **OV2640** | 最高 1600x1200 | DVP | 常用，价格便宜 |
| **OV3660** | 最高 2048x1536 | DVP | 更高分辨率 |
| **SC2336** | 最高 1920x1080 | DVP | ESP32-P4 官方推荐 |

**注意**: ESP32-S3-Korvo-2 的摄像头接口是 14-pin DVP 接口，需要确保摄像头模组兼容。

## 📦 示例项目选择

ESP WebRTC 解决方案提供了多个示例项目，根据你的需求选择：

### 1. peer_demo（推荐入门）⭐

**适合场景**: 学习 WebRTC 基础，无需摄像头

| 项目 | 说明 |
|------|------|
| **功能** | 简单的点对点聊天应用 |
| **硬件需求** | 仅需 WiFi，无需摄像头/音频 |
| **难度** | ⭐ 最简单 |
| **ESP32-S3 支持** | ✅ 完全支持 |

**特点**:
- 发送模拟音频数据
- 通过数据通道发送聊天字符串
- 最小化硬件要求
- 适合理解 WebRTC 工作原理

### 2. doorbell_demo（推荐音视频）⭐⭐

**适合场景**: 完整的音视频应用，类似智能门铃

| 项目 | 说明 |
|------|------|
| **功能** | 门铃应用，支持视频流和双向音频 |
| **硬件需求** | 需要摄像头 + 音频（Korvo-2 自带音频 ✅） |
| **难度** | ⭐⭐ 中等 |
| **ESP32-S3 支持** | ✅ 支持（需要外接摄像头） |

**特点**:
- 浏览器查看实时视频
- 双向音频通话
- 远程开门控制
- 使用 AppRTC 信令

### 3. videocall_demo（设备对设备）⭐⭐⭐

**适合场景**: 两个 ESP32 设备之间的视频通话

| 项目 | 说明 |
|------|------|
| **功能** | 设备到设备的视频通话 |
| **硬件需求** | 需要 2 个开发板 + 2 个摄像头 |
| **难度** | ⭐⭐⭐ 较难 |
| **ESP32-S3 支持** | ✅ 支持 |

**特点**:
- 视频通过数据通道传输
- 支持呼叫/接听/挂断
- 需要两个设备

### 4. whip_demo（流媒体推送）⭐⭐

**适合场景**: 推流到 WebRTC 服务器

| 项目 | 说明 |
|------|------|
| **功能** | 使用 WHIP 协议推流 |
| **硬件需求** | 需要摄像头 + 音频 |
| **难度** | ⭐⭐ 中等 |
| **ESP32-S3 支持** | ✅ 支持 |

**特点**:
- 推流到 Janus/Kurento 等服务器
- 单向视频流
- 适合监控应用

### 5. openai_demo（AI 聊天机器人）⭐⭐⭐

**适合场景**: 与 OpenAI 实时 API 集成

| 项目 | 说明 |
|------|------|
| **功能** | 实时语音聊天机器人 |
| **硬件需求** | 需要音频（Korvo-2 自带 ✅） |
| **难度** | ⭐⭐⭐ 较难 |
| **ESP32-S3 支持** | ✅ 支持 |

**特点**:
- 与 OpenAI 实时 API 通信
- 语音输入/输出
- 需要 OpenAI API 密钥

## 🚀 快速开始：peer_demo

我们从最简单的 `peer_demo` 开始，它不需要摄像头，只需要 WiFi 连接。

### peer_demo 架构说明

#### 通信架构

peer_demo 使用 **Espressif 官方提供的公共 AppRTC 信令服务器**，无需自己搭建服务器：

```
ESP32 设备  ←→  webrtc.espressif.com  ←→  对端设备
```

**服务器信息**:
- **信令服务器**: `wss://webrtc.espressif.com:8089/ws`
- **ICE 服务器**: `webrtc.espressif.com:3033`
- **Web 测试界面**: `https://webrtc.espressif.com`

#### 支持的通信方式

**方式 1: ESP32 ↔ ESP32** (两块开发板)
```
ESP32 板子 1  ←→  webrtc.espressif.com  ←→  ESP32 板子 2
```
- 两块 ESP32 都运行 peer_demo
- 连接到同一个 room ID
- 可以进行音频和数据通道通信

**方式 2: ESP32 ↔ Web 浏览器** (推荐测试) ⭐
```
ESP32 板子  ←→  webrtc.espressif.com  ←→  浏览器
```
- ESP32 运行 peer_demo
- 浏览器打开 `https://webrtc.espressif.com`
- 输入相同的 room ID
- 可以在浏览器中看到音频流和数据通道消息

**方式 3: ESP32 ↔ 移动 App**
```
ESP32 板子  ←→  webrtc.espressif.com  ←→  手机 App
```
- 需要开发支持 WebRTC 的移动应用
- 连接到相同的信令服务器和 room ID

#### 工作流程

```
1. ESP32 连接 WiFi
2. ESP32 连接到 webrtc.espressif.com 信令服务器
3. 执行命令: start room123
4. 对端（浏览器/另一块 ESP32）加入相同房间
5. 建立 WebRTC P2P 连接
6. 开始音频/数据通信
```

#### 功能特点

- ✅ **无需搭建服务器**: 使用 Espressif 公共服务
- ✅ **无需摄像头**: 纯音频和数据通道通信
- ✅ **简单测试**: 浏览器即可测试
- ✅ **P2P 连接**: 低延迟实时通信
- ✅ **数据通道**: 支持自定义数据传输

### 1. 环境准备

#### 1.1 ESP-IDF 版本要求

```bash
# 检查当前 ESP-IDF 版本
cd ~/esp/esp-idf
git describe --tags

# 推荐使用 v5.4 或更高版本
# 如果版本低于 v5.4，需要更新
git fetch
git checkout v5.4
git submodule update --init --recursive
```

#### 1.2 设置环境变量

```bash
# 每次新终端都需要运行
cd ~/esp/esp-idf
source export.sh

# 或者使用别名
alias get_idf='. ~/esp/esp-idf/export.sh'
get_idf
```

### 2. 克隆 ESP WebRTC 解决方案

```bash
# 进入工作目录
cd ~/work/esp32

# 克隆仓库（如果还没有）
git clone --recursive https://github.com/espressif/esp-webrtc-solution.git

# 进入仓库
cd esp-webrtc-solution

# 检查版本
git describe --tags
# 应该显示 v1.0.0 或更高
```

### 3. 配置 peer_demo

```bash
# 进入 peer_demo 目录
cd solutions/peer_demo

# 设置目标芯片为 ESP32-S3
idf.py set-target esp32s3
```

#### 3.1 修改 WiFi 配置

编辑 `main/settings.h` 文件：

```c
/**
 * @brief  Set for wifi ssid
 */
#define WIFI_SSID     "你的WiFi名称"

/**
 * @brief  Set for wifi password
 */
#define WIFI_PASSWORD "你的WiFi密码"
```

### 4. 编译项目

```bash
# 编译
idf.py build
```

**预期输出**:
```
Project build complete. To flash, run:
 idf.py -p PORT flash
or
 idf.py -p PORT flash monitor
```

### 5. 烧录到开发板

```bash
# 烧录并监控（使用你的串口）
idf.py -p /dev/cu.SLAB_USBtoUART flash monitor
```

**注意**: ESP32-S3-Korvo-2 需要两根 USB 线：
- **USB Power Port**: 供电（5V）
- **USB-to-UART Port**: 串口通信（连接到 `/dev/cu.SLAB_USBtoUART`）

### 6. 测试 peer_demo

#### 6.1 启动后的输出

设备启动后会自动连接 WiFi：

```
I (1234) wifi: connected to SSID:你的WiFi名称
I (1235) wifi: got ip:192.168.1.100
I (1236) peer_demo: WiFi connected successfully
```

#### 6.2 使用控制台命令

在串口监控中输入以下命令：

**1. 加入房间**:
```
start mytestroom
```

**预期输出**:
```
I (5678) peer_demo: Joining room: mytestroom
I (5679) peer_demo: Waiting for peer...
```

**2. 查看系统信息**:
```
i
```

**预期输出**:
```
Free heap: 234567 bytes
PSRAM free: 8123456 bytes
CPU usage: 12%
```

**3. 停止聊天**:
```
stop
```

**4. 切换 WiFi**:
```
wifi 新SSID 新密码
```

#### 6.3 使用浏览器测试 (推荐) ⭐

这是最简单的测试方式，无需第二块开发板：

**步骤 1: ESP32 加入房间**

在串口监控中输入：
```
start myroom123
```

**预期输出**:
```
I (5678) peer_demo: Joining room: myroom123
I (5679) apprtc: Connecting to webrtc.espressif.com:8089
I (5680) apprtc: Connected to signaling server
I (5681) peer_demo: Waiting for peer...
```

**步骤 2: 浏览器加入相同房间**

1. 打开浏览器访问: `https://webrtc.espressif.com`
2. 在页面中输入房间号: `myroom123`
3. 点击 "Join" 按钮

**步骤 3: 建立连接**

浏览器和 ESP32 会自动建立 WebRTC P2P 连接：

**ESP32 输出**:
```
I (10000) peer_demo: ICE candidate gathering...
I (10500) peer_demo: Peer connected!
I (10501) peer_demo: Connection state: CONNECTED
I (10502) peer_demo: Sending fake audio data...
I (10503) peer_demo: Data channel opened
```

**浏览器显示**:
- 连接状态: Connected
- 可以看到音频流指示
- 数据通道消息: "Hello from ESP32"

**步骤 4: 测试数据通道**

在浏览器的数据通道输入框中发送消息，ESP32 会收到：

**ESP32 输出**:
```
I (15000) peer_demo: Received data: Hello from browser
```

**步骤 5: 断开连接**

在 ESP32 串口中输入：
```
stop
```

或者在浏览器中点击 "Leave" 按钮。

#### 6.4 双设备测试

如果你有两个 ESP32 开发板：

**设备 1**:
```
start testroom123
```

**设备 2**:
```
start testroom123
```

两个设备会自动建立 WebRTC 连接，并开始交换模拟音频数据和聊天消息。

**预期输出**:
```
I (10000) peer_demo: Peer connected!
I (10001) peer_demo: Sending fake audio data...
I (10002) peer_demo: Received chat message: Hello from peer
```

## 🎥 进阶：doorbell_demo（需要摄像头）

如果你有摄像头模组，可以尝试 `doorbell_demo`。

### 1. 硬件连接

#### 1.1 连接摄像头

将摄像头模组连接到 ESP32-S3-Korvo-2 的 14-pin 摄像头接口。

**摄像头接口管脚分配**:

| 管脚 | 功能 | 说明 |
|------|------|------|
| GPIO17 | SIOD | I2C 数据 |
| GPIO18 | SIOC | I2C 时钟 |
| GPIO3 | D5 | 数据位 5 |
| GPIO11 | PCLK | 像素时钟 |
| GPIO12 | D6 | 数据位 6 |
| GPIO13 | D2 | 数据位 2 |
| GPIO14 | D4 | 数据位 4 |
| GPIO21 | VSYNC | 垂直同步 |
| GPIO47 | D3 | 数据位 3 |
| GPIO38 | HREF | 水平参考 |
| GPIO39 | D9 | 数据位 9 |
| GPIO40 | XCLK | 主时钟 |
| GPIO41 | D8 | 数据位 8 |
| GPIO42 | D7 | 数据位 7 |

#### 1.2 连接扬声器

将扬声器连接到扬声器输出端口（Korvo-2 自带）。

### 2. 配置 doorbell_demo

```bash
# 进入 doorbell_demo 目录
cd ~/work/esp32/esp-webrtc-solution/solutions/doorbell_demo

# 设置目标芯片
idf.py set-target esp32s3
```

#### 2.1 修改配置文件

编辑 `main/settings.h`:

```c
// WiFi 配置
#define WIFI_SSID     "你的WiFi名称"
#define WIFI_PASSWORD "你的WiFi密码"

// 摄像头配置（根据你的摄像头型号修改）
#define CAMERA_TYPE   CAMERA_OV2640  // 或 CAMERA_OV3660
#define CAMERA_WIDTH  640
#define CAMERA_HEIGHT 480
#define CAMERA_FPS    15

// 门铃按键（默认是 Boot 键）
#define DOOR_BELL_RING_BUTTON GPIO_NUM_0
```

### 3. 编译和烧录

```bash
# 编译
idf.py build

# 烧录
idf.py -p /dev/cu.SLAB_USBtoUART flash monitor
```

### 4. 测试 doorbell_demo

#### 4.1 设备启动

设备启动后会自动连接 WiFi 并加入房间：

```
W (9801) Webrtc_Test: Please use browser to join in esp123456 on https://webrtc.espressif.com/doorbell
```

#### 4.2 浏览器访问

1. 打开 Chrome 或 Edge 浏览器
2. 访问: https://webrtc.espressif.com/doorbell
3. 输入房间号（如 `esp123456`）
4. 点击 "Join"

#### 4.3 功能测试

**1. 查看视频流**:
- 浏览器会显示来自 ESP32 摄像头的实时视频

**2. 双向音频**:
- 浏览器可以听到 ESP32 的麦克风声音
- ESP32 可以听到浏览器的声音（通过扬声器播放）

**3. 开门控制**:
- 浏览器点击 "Door" 图标
- ESP32 播放"门已打开"提示音
- 浏览器显示 "Receiving Door opened event"

**4. 呼叫功能**:
- 按下 ESP32 的 Boot 键（或配置的按键）
- ESP32 播放门铃音乐
- 浏览器弹出 "Accept Call" 或 "Deny Call"
- 接受后建立双向音视频通话

## 🔧 常见问题

### 1. 编译错误

#### 问题: `esp_webrtc` 组件未找到

**症状**:
```
CMake Error: Could not find component 'esp_webrtc'
```

**解决方案**:
```bash
# 确保使用 --recursive 克隆
cd ~/work/esp32/esp-webrtc-solution
git submodule update --init --recursive

# 或者重新克隆
cd ~/work/esp32
rm -rf esp-webrtc-solution
git clone --recursive https://github.com/espressif/esp-webrtc-solution.git
```

#### 问题: ESP-IDF 版本过低

**症状**:
```
ESP-IDF version v5.3 is not supported. Please use v5.4 or higher.
```

**解决方案**:
```bash
cd ~/esp/esp-idf
git fetch
git checkout v5.4
git submodule update --init --recursive
source export.sh
```

### 2. 烧录错误

#### 问题: 串口无法连接

**症状**:
```
serial.serialutil.SerialException: [Errno 2] could not open port /dev/cu.SLAB_USBtoUART
```

**解决方案**:
1. 检查 USB 线是否连接到 **USB-to-UART Port**（不是 USB Power Port）
2. 检查 CP2102N 驱动是否安装：
   ```bash
   ls /dev/cu.*
   # 应该看到 /dev/cu.SLAB_USBtoUART
   ```
3. 如果没有，安装驱动：参考 `docs/esp32-serial-driver-guide-macos.md`

#### 问题: 烧录失败

**症状**:
```
A fatal error occurred: Failed to connect to ESP32-S3
```

**解决方案**:
1. 按住 **Boot** 键
2. 按一下 **Reset** 键
3. 松开 **Boot** 键
4. 重新运行 `idf.py flash`

### 3. 运行时错误

#### 问题: WiFi 连接失败

**症状**:
```
E (5000) wifi: Failed to connect to SSID:你的WiFi名称
```

**解决方案**:
1. 检查 WiFi SSID 和密码是否正确
2. 确保 WiFi 是 2.4 GHz（ESP32-S3 不支持 5 GHz）
3. 使用控制台命令重新连接：
   ```
   wifi 正确的SSID 正确的密码
   ```

#### 问题: 内存不足

**症状**:
```
E (10000) heap: heap_caps_malloc failed
```

**解决方案**:
1. 确保 PSRAM 已启用：
   ```bash
   idf.py menuconfig
   → Component config
     → ESP PSRAM
       → Support for external PSRAM: Enable
   ```
2. 检查 PSRAM 是否正常工作：
   ```
   i  # 在控制台输入，查看 PSRAM 可用空间
   ```

#### 问题: 摄像头无法初始化

**症状**:
```
E (3000) camera: Camera init failed with error 0x105
```

**解决方案**:
1. 检查摄像头是否正确连接到 14-pin 接口
2. 检查摄像头型号配置是否正确（`settings.h` 中的 `CAMERA_TYPE`）
3. 尝试降低分辨率：
   ```c
   #define CAMERA_WIDTH  320
   #define CAMERA_HEIGHT 240
   ```

### 4. WebRTC 连接问题

#### 问题: 无法建立 P2P 连接

**症状**:
```
W (30000) peer: ICE connection failed
```

**解决方案**:
1. 检查网络是否支持 UDP（某些公司网络可能阻止）
2. 使用 TURN 服务器（需要配置）
3. 确保两个设备在同一局域网内

#### 问题: 视频卡顿

**症状**: 浏览器视频播放不流畅

**解决方案**:
1. 降低视频分辨率和帧率：
   ```c
   #define CAMERA_WIDTH  320
   #define CAMERA_HEIGHT 240
   #define CAMERA_FPS    10
   ```
2. 检查 WiFi 信号强度：
   ```bash
   # 在控制台查看
   wifi
   # 信号强度应该 > -70 dBm
   ```
3. 减少其他网络流量

## 📊 性能优化

### 1. 内存优化

#### 启用 PSRAM

```bash
idf.py menuconfig
→ Component config
  → ESP PSRAM
    → Support for external PSRAM: Enable
    → SPI RAM config
      → Initialize SPI RAM during startup: Yes
      → SPI RAM access method: Make RAM allocatable using malloc()
      → Try to allocate memories of WiFi and LWIP in SPIRAM firstly: Yes
```

#### 调整堆内存分配

```c
// 在代码中使用 PSRAM
void *buffer = heap_caps_malloc(1024 * 1024, MALLOC_CAP_SPIRAM);
```

### 2. CPU 优化

#### 任务优先级分配

```c
// 高优先级：音频处理
xTaskCreatePinnedToCore(audio_task, "audio", 4096, NULL, 10, NULL, 1);

// 中优先级：视频处理
xTaskCreatePinnedToCore(video_task, "video", 4096, NULL, 8, NULL, 0);

// 低优先级：网络传输
xTaskCreatePinnedToCore(network_task, "net", 4096, NULL, 5, NULL, 0);
```

#### 双核分配

- **Core 0**: 视频编码、网络传输
- **Core 1**: 音频处理、AEC 算法

### 3. 网络优化

#### WiFi 配置

```bash
idf.py menuconfig
→ Component config
  → Wi-Fi
    → WiFi IRAM speed optimization: Enable
    → WiFi RX IRAM speed optimization: Enable
```

#### 降低延迟

```c
// 减少缓冲区大小
#define AUDIO_BUFFER_SIZE  256  // 降低音频延迟
#define VIDEO_BUFFER_SIZE  1    // 减少视频缓冲
```

## 📖 参考资料

### 官方文档

- [ESP WebRTC 解决方案 GitHub](https://github.com/espressif/esp-webrtc-solution)
- [ESP-IDF 编程指南](https://docs.espressif.com/projects/esp-idf/zh_CN/latest/esp32s3/)
- [ESP32-S3 技术规格书](https://www.espressif.com/sites/default/files/documentation/esp32-s3_datasheet_cn.pdf)

### 本地文档

- [ESP32-S3-Korvo-2 V3.1 完整分析](esp32-s3-korvo-2-analysis.md)
- [ESP32-S3 硬件规格](esp32-s3-hardware-specs.md)
- [ESP-IDF 快速参考](esp-idf-quick-reference.md)

### WebRTC 资源

- [WebRTC 官方网站](https://webrtc.org/)
- [AppRTC 示例](https://github.com/webrtc/apprtc)
- [WHIP 协议规范](https://datatracker.ietf.org/doc/html/draft-ietf-wish-whip)

## 🎯 下一步

### 学习路径

1. **入门**: 从 `peer_demo` 开始，理解 WebRTC 基础
2. **音频**: 尝试 `openai_demo`，体验语音交互
3. **视频**: 运行 `doorbell_demo`，实现完整音视频应用
4. **进阶**: 自定义信令，集成到你的项目

### 项目示例

基于 ESP WebRTC 解决方案，你可以构建：

- 🚪 **智能门铃**: 远程视频监控 + 双向对讲
- 📹 **视频监控**: 实时视频流推送到服务器
- 🤖 **AI 助手**: 语音交互的智能设备
- 📞 **视频通话**: 设备间的视频通话系统
- 🎥 **直播推流**: RTSP/WHIP 流媒体推送

### 社区支持

- [ESP32 论坛](https://esp32.com/)
- [GitHub Issues](https://github.com/espressif/esp-webrtc-solution/issues)
- [乐鑫官方技术支持](https://www.espressif.com/zh-hans/contact-us/technical-inquiries)

---

**文档版本**: 1.0
**创建日期**: 2026-04-14
**适用硬件**: ESP32-S3-Korvo-2 V3.1
**ESP-IDF 版本**: v5.4+
**ESP WebRTC 版本**: v1.0.0
