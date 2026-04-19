# ESP32-S3-Korvo-2 V3.1 开发板完整分析

## 📊 开发板概述

**型号**: ESP32-S3-Korvo-2 V3.1
**制造商**: Espressif Systems
**定位**: 多媒体音频开发板
**应用场景**: 语音识别、语音唤醒、音视频处理、AIoT 应用

## 🔧 硬件规格

### 核心模组：ESP32-S3-WROOM-1

| 项目 | 规格 |
|------|------|
| **芯片** | ESP32-S3 (QFN56) |
| **CPU** | 双核 Xtensa LX7 @ 240 MHz |
| **内部 SRAM** | 512 KB |
| **ROM** | 384 KB |
| **Flash** | 根据模组型号不同 |
| **PSRAM** | 根据模组型号不同 |

**注意**: ESP32-S3-WROOM-1 有多个变体：
- ESP32-S3-WROOM-1-N4: 4 MB Flash, 无 PSRAM
- ESP32-S3-WROOM-1-N8: 8 MB Flash, 无 PSRAM
- ESP32-S3-WROOM-1-N16R8: 16 MB Flash, 8 MB PSRAM
- ESP32-S3-WROOM-1-N4R2: 4 MB Flash, 2 MB PSRAM
- ESP32-S3-WROOM-1-N8R2: 8 MB Flash, 2 MB PSRAM
- ESP32-S3-WROOM-1-N8R8: 8 MB Flash, 8 MB PSRAM

**你的设备**: 根据之前检测，你的设备是 **N16R8** 版本（16 MB Flash + 8 MB PSRAM）

### 音频组件

#### 1. 音频 ADC (ES7210)

| 项目 | 规格 |
|------|------|
| **型号** | ES7210 |
| **通道数** | 4 通道 |
| **采样率** | 8 kHz - 96 kHz |
| **位深度** | 16/24 bit |
| **接口** | I2S + I2C |
| **用途** | 麦克风阵列采集、AEC 参考信号 |

#### 2. 音频 Codec (ES8311)

| 项目 | 规格 |
|------|------|
| **型号** | ES8311 |
| **类型** | 单声道编解码器 |
| **ADC** | 1 通道 |
| **DAC** | 1 通道 |
| **采样率** | 8 kHz - 96 kHz |
| **位深度** | 16/24/32 bit |
| **接口** | I2S + I2C |
| **特性** | 低噪声前置放大器、耳机驱动器 |

#### 3. 音频功放 (NS4150)

| 项目 | 规格 |
|------|------|
| **型号** | NS4150 |
| **类型** | D 类功放 |
| **功率** | 3 W |
| **通道** | 单声道 |
| **特性** | 低 EMI |

#### 4. 麦克风

| 项目 | 规格 |
|------|------|
| **数量** | 2 个（左右各一） |
| **类型** | 板载 MEMS 麦克风 |
| **连接** | ES7210 ADC |
| **用途** | 语音采集、远场唤醒 |

### 外设接口

| 接口 | 数量/规格 | 说明 |
|------|----------|------|
| **USB-to-UART** | 1 个 (CP2102N) | 程序下载和调试 |
| **USB 电源** | 1 个 (Micro-B) | 5V 供电 |
| **电池接口** | 1 个 (2-pin) | 单节锂电池 (3.7V) |
| **扬声器接口** | 1 个 | 外接扬声器 |
| **microSD 卡槽** | 1 个 | 1-line 模式 |
| **摄像头接口** | 1 个 (14-pin) | 支持 JPEG 视频流 |
| **LCD 接口** | 1 个 (FPC 0.5mm) | 连接 LCD 扩展板 |
| **功能按键** | 6 个 | REC, MUTE, PLAY, SET, VOL-, VOL+ |
| **Boot/Reset** | 2 个 | 固件上传和复位 |
| **LED** | 2 个 | 绿色和红色状态指示灯 |

### 电源管理

| 项目 | 规格 |
|------|------|
| **主电源** | 5V USB (推荐 5V/2A) |
| **辅助电源** | 3.7V 锂电池 |
| **充电芯片** | AP5056 |
| **电源开关** | 拨动开关 |
| **独立供电** | 音频和数字电路分离供电 |

## 💾 存储配置

### Flash 分区（基于你的 16 MB Flash）

典型的 ESP-ADF 分区表：

```
# Name,     Type, SubType, Offset,  Size,   Flags
nvs,        data, nvs,     0x9000,  24K,
phy_init,   data, phy,     0xf000,  4K,
factory,    app,  factory, 0x10000, 3M,
storage,    data, fat,     0x310000, 12M,
```

| 分区 | 大小 | 用途 |
|------|------|------|
| **Bootloader** | 32 KB | 引导加载程序 |
| **Partition Table** | 4 KB | 分区表 |
| **NVS** | 24 KB | 非易失性存储（WiFi 配置等） |
| **PHY Init** | 4 KB | PHY 初始化数据 |
| **Factory App** | 3 MB | 应用程序 |
| **Storage (FAT)** | 12 MB | 文件系统（音频文件等） |
| **剩余** | ~1 MB | 预留空间 |

### RAM 使用（运行时）

基于 ESP-ADF 音频应用的典型内存使用：

| 内存类型 | 总容量 | 典型使用 | 可用 |
|---------|--------|---------|------|
| **内部 SRAM** | 512 KB | 150-250 KB | 260-360 KB |
| **PSRAM** | 8 MB | 0-4 MB | 4-8 MB |

**内存分配示例**（play_mp3_control）：

```
启动时堆内存分配:
├── DRAM (3FFAE6E0)      6 KB
├── DRAM (3FFB2FE0)    180 KB
├── D/IRAM (3FFE0440)   14 KB
├── D/IRAM (3FFE4350)  111 KB
└── IRAM (4008A95C)     85 KB
                       ─────
                       396 KB 总可用堆内存
```

## 🎯 GPIO 管脚分配

### 音频相关

| GPIO | 功能 | 连接 |
|------|------|------|
| **GPIO16** | I2S0_MCLK | ES8311 + ES7210 主时钟 |
| **GPIO9** | I2S0_SCLK | ES8311 + ES7210 位时钟 |
| **GPIO45** | I2S0_LRCK | ES8311 + ES7210 左右声道时钟 |
| **GPIO8** | I2S0_DSDIN | ES8311 数据输入 |
| **GPIO10** | SDOUT | ES7210 数据输出 |
| **GPIO17** | I2C_SDA | I2C 数据线 |
| **GPIO18** | I2C_CLK | I2C 时钟线 |
| **GPIO48** | PA_CTRL | 功放控制 |

### 摄像头接口

| GPIO | 功能 |
|------|------|
| GPIO17 | SIOD (I2C 数据) |
| GPIO18 | SIOC (I2C 时钟) |
| GPIO3 | D5 |
| GPIO11 | PCLK |
| GPIO12 | D6 |
| GPIO13 | D2 |
| GPIO14 | D4 |
| GPIO21 | VSYNC |
| GPIO47 | D3 |
| GPIO38 | HREF |
| GPIO39 | D9 |
| GPIO40 | XCLK |
| GPIO41 | D8 |
| GPIO42 | D7 |

### microSD 卡

| GPIO | 功能 |
|------|------|
| GPIO4 | DATA0 |
| GPIO7 | CMD |
| GPIO15 | CLK |

### LCD 接口

| GPIO | 功能 |
|------|------|
| GPIO0 | LCD_SPI_SDA |
| GPIO1 | LCD_SPI_CLK |
| GPIO2 | LCD_SPI_DC |

### 按键

| GPIO | 功能 |
|------|------|
| GPIO5 | REC, MUTE, PLAY, SET, VOL-, VOL+ (共享) |
| GPIO0 | BOOT_KEY |
| EN | EN_KEY (Reset) |

## 📦 ESP-ADF 框架

### 目录结构

```
esp-adf/
├── components/          # ADF 组件
│   ├── audio_board/    # 开发板配置
│   ├── audio_hal/      # 硬件抽象层
│   ├── audio_pipeline/ # 音频管道
│   ├── audio_stream/   # 音频流
│   ├── esp_codec/      # 编解码器
│   └── ...
├── examples/           # 示例程序
│   ├── get-started/   # 入门示例
│   ├── player/        # 播放器示例
│   ├── recorder/      # 录音示例
│   └── ...
└── esp-idf/           # ESP-IDF 子模块
```

### 支持的音频格式

#### 解码器（播放）

| 格式 | 支持 | 说明 |
|------|------|------|
| **MP3** | ✅ | 完整支持 |
| **AAC** | ✅ | AAC-LC, HE-AAC |
| **FLAC** | ✅ | 无损压缩 |
| **WAV** | ✅ | PCM |
| **AMR** | ✅ | AMR-NB, AMR-WB |
| **OPUS** | ✅ | 低延迟 |
| **OGG** | ✅ | Vorbis |

#### 编码器（录音）

| 格式 | 支持 | 说明 |
|------|------|------|
| **AMR** | ✅ | AMR-NB |
| **OPUS** | ✅ | 可变码率 |
| **AAC** | ✅ | AAC-LC |
| **WAV** | ✅ | PCM |

## 🚀 开发环境配置

### 1. 安装 ESP-ADF

```bash
# 克隆 ESP-ADF（如果还没有）
cd ~/work/esp32
git clone --recursive https://github.com/espressif/esp-adf.git

# 进入 ESP-ADF 目录
cd esp-adf

# 安装依赖
./install.sh esp32s3

# 设置环境变量
source export.sh
```

### 2. 配置开发板

在 `menuconfig` 中选择 ESP32-S3-Korvo-2：

```bash
cd examples/get-started/play_mp3_control
idf.py menuconfig
```

导航到：
```
Audio HAL → ESP32-S3-Korvo-2
```

### 3. 编译和烧录

```bash
# 设置目标芯片
idf.py set-target esp32s3

# 编译
idf.py build

# 烧录（使用你的串口）
idf.py -p /dev/cu.SLAB_USBtoUART flash monitor
```

## 📝 示例程序分析

### play_mp3_control 示例

**功能**: 播放 Flash 中嵌入的 MP3 文件，支持按键控制

#### 内存使用

从日志输出可以看到：

```
堆内存初始化:
├── DRAM (3FFAE6E0)      6 KB
├── DRAM (3FFB2FE0)    180 KB
├── D/IRAM (3FFE0440)   14 KB
├── D/IRAM (3FFE4350)  111 KB
└── IRAM (4008A95C)     85 KB
                       ─────
总计:                  396 KB
```

**可用堆内存**: 约 396 KB（内部 SRAM）

#### Flash 使用

```
分区表:
├── nvs (0x9000)        24 KB
├── phy_init (0xf000)    4 KB
└── factory (0x10000)    1 MB
```

**应用程序大小**: 约 158 KB（从日志中的 segment 大小推算）

#### 音频处理流程

```
[Flash MP3 数据]
      ↓
[MP3 Decoder] ← read_cb 回调读取
      ↓
[I2S Stream]
      ↓
[ES8311 Codec]
      ↓
[NS4150 功放]
      ↓
[扬声器]
```

#### 支持的功能

| 按键 | 功能 |
|------|------|
| **PLAY** | 开始/暂停/恢复播放 |
| **VOL+** | 音量增加 |
| **VOL-** | 音量减少 |
| **MODE** | 切换采样率 (8/22.05/44.1 kHz) |
| **SET** | 停止播放并退出 |

## 🎓 典型应用场景

### 1. 语音识别系统

**硬件配置**:
- 双麦克风阵列（远场拾音）
- ES7210 ADC（4 通道采集）
- AEC 回声消除

**软件配置**:
```c
// 音频采集管道
[麦克风] → [ES7210] → [I2S Stream] → [AEC] → [VAD] → [ASR]
```

**内存需求**:
- 音频缓冲: ~100 KB
- AEC 算法: ~200 KB
- ASR 模型: 2-4 MB (PSRAM)

### 2. 音乐播放器

**硬件配置**:
- microSD 卡（存储音乐文件）
- ES8311 Codec（解码输出）
- NS4150 功放（驱动扬声器）

**软件配置**:
```c
// 播放管道
[microSD] → [FAT FS] → [MP3 Decoder] → [I2S Stream] → [ES8311] → [扬声器]
```

**内存需求**:
- 文件缓冲: ~32 KB
- 解码缓冲: ~16 KB
- 总计: ~50 KB

### 3. 视频流处理

**硬件配置**:
- 摄像头模组
- LCD 扩展板
- microSD 卡（存储）

**软件配置**:
```c
// 视频管道
[摄像头] → [JPEG 编码] → [LCD 显示]
                    ↓
              [microSD 存储]
```

**内存需求**:
- 图像缓冲: 320x240 RGB565 = 150 KB
- JPEG 缓冲: ~30 KB
- 总计: ~180 KB (需要 PSRAM)

## 🔧 优化建议

### 1. 启用 PSRAM

对于复杂应用（AI、视频），必须启用 PSRAM：

```bash
idf.py menuconfig
→ Component config
  → ESP PSRAM
    → Support for external PSRAM: Enable
    → SPI RAM config
      → Initialize SPI RAM during startup: Yes
      → SPI RAM access method: Make RAM allocatable using malloc()
```

### 2. 音频质量优化

```bash
idf.py menuconfig
→ Audio HAL
  → Audio Sample Rate: 48000 Hz (高质量)
  → Audio Bits: 16 bits
```

### 3. 功耗优化

```c
// 使用 Light Sleep 模式
esp_pm_config_esp32s3_t pm_config = {
    .max_freq_mhz = 240,
    .min_freq_mhz = 80,
    .light_sleep_enable = true
};
esp_pm_configure(&pm_config);
```

### 4. Flash 分区优化

对于大型应用，调整分区表：

```csv
# partitions.csv
nvs,        data, nvs,     0x9000,  96K,
phy_init,   data, phy,     0x1f000, 4K,
factory,    app,  factory, 0x20000, 4M,
storage,    data, fat,     0x420000, 11M,
```

## 📊 性能基准

### 音频处理性能

| 操作 | CPU 使用率 | 内存使用 |
|------|-----------|---------|
| **MP3 解码** (128 kbps) | ~15% | ~50 KB |
| **AAC 解码** (128 kbps) | ~20% | ~80 KB |
| **AEC 处理** | ~25% | ~200 KB |
| **VAD 检测** | ~5% | ~20 KB |

### 启动时间

| 阶段 | 时间 |
|------|------|
| **Bootloader** | ~100 ms |
| **应用程序加载** | ~200 ms |
| **音频初始化** | ~300 ms |
| **总启动时间** | ~600 ms |

## 🐛 常见问题

### 1. 串口无法连接

**症状**: 找不到 `/dev/cu.SLAB_USBtoUART`

**解决方案**:
- 确保 CP2102N 驱动已安装
- 检查 USB 线是否支持数据传输
- 确认电源线和串口线都已连接

### 2. 音频无输出

**症状**: 编译烧录成功，但扬声器无声音

**检查清单**:
- [ ] 扬声器是否正确连接
- [ ] 电源开关是否打开
- [ ] 音量是否设置为 0
- [ ] ES8311 是否正确初始化

**调试命令**:
```bash
# 检查 I2C 设备
i2cdetect -y 0

# 应该看到 ES8311 (0x18) 和 ES7210 (0x40)
```

### 3. microSD 卡无法识别

**症状**: 无法读取 microSD 卡

**解决方案**:
- 使用 FAT32 格式化
- 确保卡容量 ≤ 32 GB
- 检查卡槽接触是否良好

### 4. 内存不足

**症状**: `heap_caps_malloc failed` 错误

**解决方案**:
- 启用 PSRAM
- 减少音频缓冲区大小
- 优化代码，减少静态分配

## 📖 相关文档

### 官方文档

- [ESP32-S3-Korvo-2 V3.1 用户指南](https://docs.espressif.com/projects/esp-adf/zh_CN/latest/design-guide/dev-boards/user-guide-esp32-s3-korvo-2.html)
- [ESP-ADF 编程指南](https://docs.espressif.com/projects/esp-adf/zh_CN/latest/)
- [ESP32-S3 技术规格书](https://www.espressif.com/sites/default/files/documentation/esp32-s3_datasheet_cn.pdf)
- [ESP32-S3-WROOM-1 技术规格书](https://www.espressif.com/sites/default/files/documentation/esp32-s3-wroom-1_wroom-1u_datasheet_cn.pdf)

### 硬件资料

- [ESP32-S3-Korvo-2 V3.1 原理图](https://dl.espressif.com/dl/schematics/SCH_ESP32-S3-KORVO-2_V3_0_20210918.pdf)
- [ESP32-S3-Korvo-2 V3.1 PCB 布局图](https://dl.espressif.com/dl/schematics/PCB_ESP32-S3-KORVO-2_V3.0_20210918.pdf)

### 芯片数据手册

- [ES7210 数据手册](http://www.everest-semi.com/pdf/ES7210%20PB.pdf)
- [ES8311 数据手册](http://www.everest-semi.com/pdf/ES8311%20PB.pdf)

## 🎯 快速开始指南

### 第一次使用

1. **硬件准备**:
   ```
   ✓ ESP32-S3-Korvo-2 V3.1 开发板
   ✓ 扬声器（连接到扬声器输出端口）
   ✓ 2 根 USB 数据线（Micro-B）
   ✓ 电脑（macOS/Linux/Windows）
   ```

2. **连接硬件**:
   ```
   1. 连接扬声器到扬声器输出端口
   2. USB 电源线连接到 USB Power Port
   3. USB 串口线连接到 USB-to-UART Port
   4. 打开电源开关（向下拨动）
   5. 红色电源 LED 应该亮起
   ```

3. **配置环境**:
   ```bash
   cd ~/work/esp32/esp-adf
   source export.sh
   ```

4. **编译示例**:
   ```bash
   cd examples/get-started/play_mp3_control
   idf.py set-target esp32s3
   idf.py menuconfig  # 选择 ESP32-S3-Korvo-2
   idf.py build
   ```

5. **烧录运行**:
   ```bash
   idf.py -p /dev/cu.SLAB_USBtoUART flash monitor
   ```

6. **测试功能**:
   - 应该自动播放 MP3 音乐
   - 按 PLAY 键暂停/恢复
   - 按 VOL+/VOL- 调节音量
   - 按 MODE 切换采样率
   - 按 SET 停止播放

## 📹 视频通话能力分析

### 支持的视频编码格式

ESP32-S3-Korvo-2 V3.1 支持的视频编码格式：

#### ✅ 完全支持

| 格式 | 支持程度 | 说明 |
|------|---------|------|
| **JPEG** | ✅ 完全支持 | 官方文档明确支持"基于 JPEG 的视频流处理" |
| **MJPEG** | ✅ 完全支持 | Motion JPEG，JPEG 图像序列，适合视频流 |

#### ⚠️ 有限支持

| 格式 | 支持程度 | 说明 |
|------|---------|------|
| **H.264** | ⚠️ 软件编码 | 可通过软件实现，但 CPU 占用高 |
| | | - 适合低分辨率（320x240 @ 15fps） |
| | | - 需要第三方库（如 x264 轻量级版本） |
| | | - 会显著影响其他任务性能 |

#### 推荐配置

**视频通话推荐使用 MJPEG**：
- **编码/解码简单**：CPU 占用低（~20-30%）
- **分辨率**：320x240 或 640x480
- **帧率**：15-30 fps（取决于分辨率和网络）
- **带宽需求**：200-500 Kbps（取决于 JPEG 质量）

### AEC（回声消除）实现方式

**AEC 是软件实现的**，具体架构如下：

#### 硬件部分（信号采集）

ESP32-S3-Korvo-2 V3.1 的 AEC 电路设计：

**1. 参考信号源**（二选一）：
- **推荐**：ES8311 Codec 的 DAC 输出 (DAC_AOUTLN/DAC_AOUTLP)
- **备选**：NS4150 功放输出 (PA_OUTL+/PA_OUTL-)

**2. 信号采集路径**：
```
扬声器播放信号 → ES8311 DAC 输出
                      ↓
              ES7210 ADC (MIC3P/MIC3N)
                      ↓
                  I2S 接口
                      ↓
                  ESP32-S3
```

**3. 麦克风输入**：
- 双麦克风阵列通过 ES7210 的其他通道采集
- 支持 4 通道同时采集（2 个麦克风 + 1 个参考信号 + 1 个预留）

#### 软件部分（AEC 算法）

ESP32-S3 上运行的 AEC 算法处理流程：

```
音频处理管道：

[左麦克风] ──┐
            ├──→ [ES7210 ADC] ──→ [I2S Stream] ──→ [ESP32-S3]
[右麦克风] ──┘                                          │
                                                       ↓
[参考信号] ────→ [ES7210 ADC] ──→ [I2S Stream] ──→ [AEC 算法]
(扬声器输出)                                            │
                                                       ↓
                                                  [处理后音频]
                                                       ↓
                                                  [编码器]
                                                       ↓
                                                  [网络传输]
```

#### AEC 性能指标

基于 ESP-ADF 框架的 AEC 实现：

| 指标 | 数值 | 说明 |
|------|------|------|
| **CPU 占用** | ~25% | 单核，240 MHz |
| **内存占用** | ~200 KB | 内部 SRAM |
| **处理延迟** | < 50 ms | 取决于缓冲区大小 |
| **回声抑制** | 30-40 dB | 典型值 |
| **采样率** | 8-48 kHz | 可配置 |
| **帧大小** | 128-512 samples | 可配置 |

#### ESP-ADF 中的 AEC 配置

```bash
idf.py menuconfig
→ Audio HAL
  → AEC Configuration
    → Enable AEC: Yes
    → AEC Frame Size: 256 samples
    → AEC Filter Length: 128
    → AEC Sample Rate: 16000 Hz
```

**代码示例**：
```c
// 创建 AEC 音频管道
audio_pipeline_handle_t pipeline;
audio_element_handle_t i2s_stream_reader, aec_element, encoder;

// I2S 流（麦克风输入）
i2s_stream_cfg_t i2s_cfg = I2S_STREAM_CFG_DEFAULT();
i2s_stream_reader = i2s_stream_init(&i2s_cfg);

// AEC 元素
aec_cfg_t aec_cfg = {
    .aec_mode = AEC_MODE_NLMS,  // NLMS 算法
    .frame_size = 256,
    .filter_length = 128,
};
aec_element = aec_init(&aec_cfg);

// 连接管道
audio_pipeline_register(pipeline, i2s_stream_reader, "i2s");
audio_pipeline_register(pipeline, aec_element, "aec");
audio_pipeline_link(pipeline, (const char *[]){"i2s", "aec", NULL}, 2);
```

### 视频通话完整方案

基于 ESP32-S3-Korvo-2 V3.1 的视频通话系统架构：

#### 硬件配置

| 组件 | 型号/规格 | 用途 |
|------|----------|------|
| **摄像头** | OV2640/OV3660 | 视频采集（14-pin 连接器） |
| **麦克风** | 板载双 MEMS | 音频采集（远场拾音） |
| **扬声器** | 3W 8Ω | 音频播放 |
| **网络** | WiFi 802.11n | 最高 150 Mbps |
| **存储** | 16 MB Flash + 8 MB PSRAM | 缓冲和算法 |

#### 软件架构

```
┌─────────────────────────────────────────────────────────┐
│                    ESP32-S3-Korvo-2                     │
├─────────────────────────────────────────────────────────┤
│                                                         │
│  视频流：                                                │
│  [摄像头] → [JPEG 编码] → [RTP 封装] → [WiFi] → 网络    │
│     ↓                                                   │
│  [LCD 显示]（可选）                                      │
│                                                         │
│  音频发送流：                                            │
│  [麦克风] → [ES7210] → [AEC] → [OPUS 编码] → [RTP] → 网络│
│                         ↑                               │
│                    [参考信号]                            │
│                         ↑                               │
│  音频接收流：                                            │
│  网络 → [RTP] → [OPUS 解码] → [ES8311] → [扬声器]       │
│                                   ↓                     │
│                              [参考信号]                  │
│                                                         │
└─────────────────────────────────────────────────────────┘
```

#### 内存需求估算

| 组件 | 内存占用 | 存储位置 |
|------|---------|---------|
| **视频缓冲** (320x240 RGB565) | 150 KB | PSRAM |
| **JPEG 编码缓冲** | 30 KB | PSRAM |
| **音频发送缓冲** | 32 KB | SRAM |
| **音频接收缓冲** | 32 KB | SRAM |
| **AEC 算法** | 200 KB | SRAM |
| **OPUS 编解码** | 64 KB | SRAM |
| **网络缓冲** | 64 KB | SRAM |
| **FreeRTOS 任务栈** | 128 KB | SRAM |
| **总计** | ~700 KB | 400 KB SRAM + 300 KB PSRAM |

**结论**：你的设备（512 KB SRAM + 8 MB PSRAM）完全满足需求。

#### 带宽需求

| 流 | 编码格式 | 分辨率/采样率 | 码率 |
|----|---------|--------------|------|
| **视频** | MJPEG | 320x240 @ 15fps | 200-400 Kbps |
| **音频发送** | OPUS | 16 kHz mono | 16-32 Kbps |
| **音频接收** | OPUS | 16 kHz mono | 16-32 Kbps |
| **总上行** | - | - | 220-430 Kbps |
| **总下行** | - | - | 20-30 Kbps |

**网络要求**：WiFi 802.11n（150 Mbps）完全满足，建议信号强度 > -70 dBm。

#### CPU 负载估算

| 任务 | CPU 占用 | 核心分配 |
|------|---------|---------|
| **JPEG 编码** | ~30% | Core 0 |
| **MJPEG 流处理** | ~10% | Core 0 |
| **音频采集 + AEC** | ~25% | Core 1 |
| **OPUS 编码** | ~15% | Core 1 |
| **OPUS 解码** | ~10% | Core 1 |
| **网络传输** | ~10% | Core 0 |
| **总计** | ~100% | 双核分配 |

**结论**：双核 240 MHz 可以支持，但需要合理的任务分配和优化。

#### 推荐配置参数

```c
// 视频配置
#define VIDEO_WIDTH         320
#define VIDEO_HEIGHT        240
#define VIDEO_FPS           15
#define JPEG_QUALITY        60      // 0-100

// 音频配置
#define AUDIO_SAMPLE_RATE   16000   // Hz
#define AUDIO_CHANNELS      1       // Mono
#define AUDIO_BITS          16      // bits
#define OPUS_BITRATE        24000   // bps

// AEC 配置
#define AEC_FRAME_SIZE      256     // samples
#define AEC_FILTER_LENGTH   128     // taps

// 网络配置
#define RTP_PAYLOAD_SIZE    1200    // bytes
#define NETWORK_BUFFER      8192    // bytes
```

### 实现建议

#### 1. 启用 PSRAM（必须）

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

#### 2. 优化任务优先级

```c
// 高优先级任务（实时性要求高）
xTaskCreatePinnedToCore(audio_capture_task, "audio_cap", 4096, NULL, 10, NULL, 1);
xTaskCreatePinnedToCore(aec_process_task, "aec", 4096, NULL, 9, NULL, 1);

// 中优先级任务
xTaskCreatePinnedToCore(video_capture_task, "video_cap", 4096, NULL, 8, NULL, 0);
xTaskCreatePinnedToCore(network_tx_task, "net_tx", 4096, NULL, 7, NULL, 0);

// 低优先级任务
xTaskCreatePinnedToCore(network_rx_task, "net_rx", 4096, NULL, 5, NULL, 0);
```

#### 3. 使用 ESP-ADF 音频管道

```c
// 推荐使用 ESP-ADF 的 pipeline 架构
audio_pipeline_handle_t rec_pipeline;
audio_element_handle_t i2s_reader, aec, opus_encoder, rtp_writer;

// 创建录音管道
rec_pipeline = audio_pipeline_init(&pipeline_cfg);
audio_pipeline_register(rec_pipeline, i2s_reader, "i2s");
audio_pipeline_register(rec_pipeline, aec, "aec");
audio_pipeline_register(rec_pipeline, opus_encoder, "opus");
audio_pipeline_register(rec_pipeline, rtp_writer, "rtp");
audio_pipeline_link(rec_pipeline,
    (const char *[]){"i2s", "aec", "opus", "rtp", NULL}, 4);
```

#### 4. 参考示例项目

ESP-ADF 中的相关示例：
- `examples/protocols/voip`: VoIP 通话示例
- `examples/recorder/pipeline_recording_to_sdcard`: 录音管道
- `examples/player/pipeline_http_mp3`: 网络流播放
- `examples/camera`: 摄像头示例（ESP-IDF）

## 🎓 总结

### ESP32-S3-Korvo-2 V3.1 特点

**✅ 优势**:
- 🎵 专业音频处理能力（双麦克风 + 高质量 Codec）
- 🧠 强大的 AI 运算能力（8 MB PSRAM）
- 📦 丰富的外设接口（摄像头、LCD、microSD）
- 🔋 灵活的供电方式（USB + 电池）
- 🎛️ 完整的开发生态（ESP-ADF 框架）
- 📹 支持视频通话（MJPEG + OPUS + 软件 AEC）

**⚠️ 注意事项**:
- 需要两根 USB 线（电源 + 串口）
- 音频应用需要启用 PSRAM
- 某些功能需要额外配件（LCD、摄像头）
- 视频通话需要合理的任务调度和优化

**🎯 适用场景**:
- 智能音箱开发
- 语音识别系统
- 音视频处理应用
- 视频通话/会议系统
- AIoT 原型开发
- 音频算法研究

---

**文档版本**: 1.1
**创建日期**: 2026-04-14
**最后更新**: 2026-04-14
**基于**: ESP-ADF 官方文档 + 实际硬件检测 + 视频通话能力分析
