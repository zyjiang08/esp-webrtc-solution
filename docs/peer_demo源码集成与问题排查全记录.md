# `peer_demo` 源码集成与问题排查全记录

## 背景

本次工作目标是把 `solutions/peer_demo` 从原始示例状态，整理成一个可源码定位、可真实采音、可板端播放、可与浏览器做 WebRTC 音频联调的版本，并把整个排查与修复过程沉淀下来。

最终结果包括：

- `peer_demo` 改为直接使用源码版 `libpeer`
- 接入真实麦克风采集与板端喇叭播放链路
- 增加 `rec2play` 本地回环测试命令
- 修复浏览器无声、控制台不可输入、烧录不稳定等问题
- 输出完整操作步骤与注意事项

---

## 最终代码结构

### `peer_demo` 侧

- `solutions/peer_demo/main/main.c`
  - CLI 命令注册
  - 线程调度配置
  - 板卡初始化、媒体系统初始化
  - 新增 `rec2play`

- `solutions/peer_demo/main/webrtc.c`
  - APPRTC 信令接入
  - 源码版 `PeerConnection` 生命周期管理
  - 麦克风采集数据送 WebRTC
  - 远端音频送本地播放器
  - 音频发送统计与静音诊断

- `solutions/peer_demo/main/media_sys.c`
  - `esp_capture` 音频采集系统
  - `av_render` I2S 播放系统
  - 本地回环 `rec2play`

- `solutions/peer_demo/main/board.c`
  - 板型设置与 codec 初始化

- `solutions/peer_demo/partitions.csv`
  - 自定义 3MB app 分区

- `solutions/peer_demo/sdkconfig.defaults`
- `solutions/peer_demo/sdkconfig.defaults.esp32s3`
- `solutions/peer_demo/sdkconfig`
  - Flash、分区、ESP32-S3 原生 USB 控制台等配置

### `libpeer` 侧

新增源码组件：

- `components/libpeer/CMakeLists.txt`

关键修复文件：

- `components/libpeer/src/config.h`
- `components/libpeer/src/stun.h`
- `components/libpeer/src/stun.c`
- `components/libpeer/src/sdp.c`
- `components/libpeer/src/rtp.h`
- `components/libpeer/src/rtp.c`
- `components/libpeer/src/rtcp.h`
- `components/libpeer/src/rtcp.c`
- `components/libpeer/src/peer_connection.c`

---

## 为什么要改成源码版 `libpeer`

原始 `peer_demo` 依赖 `esp_peer` 默认实现，而 PeerConnection 核心逻辑位于预编译库中，不方便：

- 查看具体协议实现
- 增加精确日志
- 在 ICE / SDP / DTLS / RTP 层定位问题

一开始尝试过“开源 `libpeer` + 预编译 `esp_peer` 混链”的思路，但会遇到：

- `esp_peer_get_default_impl()` 仍在预编译实现里
- 符号重复 / ABI 不一致 / 链接顺序问题

因此最终采用：

> `peer_demo` 直接使用源码版 `libpeer` 的 `PeerConnection`，绕过预编译 PeerConnection 路径。

这样 ICE / STUN / DTLS / SRTP / RTP / RTCP / SCTP 都进入可见源码路径，后续问题都能直接定位。

---

## 主要改造内容

### 1. 真实音频采集与播放

原始 `peer_demo` 更偏向聊天 / DataChannel 示例，并不适合直接做真实音频链路验证。

本次改造后的上行链路为：

`麦克风 -> esp_capture -> G711A -> PeerConnection RTP -> 浏览器`

下行链路为：

`浏览器音频 -> SRTP/RTP -> PeerConnection 回调 -> av_render -> 板端输出`

### 2. 增加 `rec2play`

新增串口命令：

- `rec2play`

作用：

- 不经过网络
- 直接把本地麦克风采集送本地播放器
- 快速验证板端采集 / codec / I2S / 功放 / 喇叭链路

### 3. 分区与镜像大小调整

由于引入源码版 `libpeer` 与真实媒体链路，镜像体积增大。

为保证稳定烧录，新增自定义分区表：

- `factory` app 分区设置为 `3M`
- Flash 配置为 `4MB`

---

## 遇到的问题、分析与解决

### 问题 1：源码集成冲突

**现象**

- 混用预编译 `esp_peer` 与源码版 `libpeer` 时，出现链接冲突与行为不一致。

**分析**

- `esp_peer_get_default_impl()` 仍在预编译路径里
- 同名实现两套并存，无法可靠覆盖

**解决**

- `peer_demo` 直接改成源码版 `PeerConnection`

### 问题 2：应用层未处理远端 candidate

**现象**

- 浏览器 / 设备侧 SDP 能互换，但连通性不稳定

**分析**

- 原始应用层只处理 SDP，没有把 `ESP_PEER_SIGNALING_MSG_CANDIDATE` 喂给 PeerConnection

**解决**

- 在 `webrtc.c` 中补远端 trickle candidate 处理

### 问题 3：WebSocket 回调线程栈不稳定

**现象**

- `create_offer()` / `create_answer()` 放在信令回调线程里执行时，容易出现异常和栈风险

**分析**

- 这些路径协议栈较深，不适合直接占用 WebSocket 任务栈

**解决**

- 改为独立调度线程执行：
  - `pc_offer`
  - `pc_remote`
  - `pc_task`

### 问题 4：`rec2play` 无声

**现象**

- 日志看起来链路打开成功，但本地回环没有声音

**分析**

- 一度把板端输入切到了 TDM / AEC 风格模式
- 但当前 `peer_demo` 使用的是普通 `esp_capture_new_audio_dev_src()`，两者不匹配

**解决**

- `board.c` 恢复普通模式，仅保留 `.reuse_dev = false`

### 问题 5：浏览器无声

**现象**

- 设备侧显示 `completed`
- 设备侧持续打印 `Sent audio packets=...`
- 浏览器收到了远端流，但听不到声音

**分析过程**

先后排除了：

- 房间没连上
- SDP 没协商成功
- 浏览器不接受 PCMA
- 单纯是板端没有喇叭

最终定位到关键根因：

> `libpeer` 的 RTP/RTCP 头使用 16-bit bitfield 描述，在 ESP32-S3 这类小端平台上会生成错误的网络字节布局。

结果是：

- 设备端认为自己在发 RTP
- 浏览器端却不能把它当成合法音频 RTP 包解码

**解决**

- 将 RTP/RTCP 头改为显式字节布局
- 增加 `set/get` helper，统一按网络字节语义构造与解析

### 问题 6：STUN 日志 `0xc057`

**现象**

- 日志频繁出现 `Unknown Attribute Type: 0xc057`

**分析**

- 这是 Chromium 私有 STUN 属性

**解决**

- 在 `stun.h` / `stun.c` 中加入忽略逻辑

### 问题 7：过早关闭

**现象**

- 一段时间后出现 `binding request timeout`

**解决**

- 将 `CONFIG_KEEPALIVE_TIMEOUT` 调到 `30000`

### 问题 8：浏览器兼容性不足

**现象**

- 协商成功率与浏览器兼容性不足

**解决**

- `sdp.c` 中补充：
  - `a=msid-semantic: WMS *`
  - `a=msid`
  - `a=ssrc ... msid/mslabel/label`
  - `a=ptime:20`
  - `a=maxptime:20`
  - 使用 `UDP/TLS/RTP/SAVPF`

### 问题 9：控制台能看日志但不能输入

**现象**

- 串口能看到日志
- 但 `esp>` 命令输不进去

**分析**

- 主控制台在 `UART0`
- 用户连接的是 `usbmodem*` 原生 USB `USB-Serial/JTAG`

**解决**

- 将 ESP32-S3 主控制台切换为 `USB-Serial/JTAG`

### 问题 10：烧录不稳定、端口反复变化

**现象**

- 经常出现：
  - `Failed to write to target RAM (Checksum error)`
  - `invalid header: 0xffffffff`
- 串口名在 `usbmodem*` 间反复变化

**分析**

- 原生 USB 口在复位/下载模式切换时会重枚举
- macOS 会分配新设备名
- 使用旧设备名继续刷就会失败

**解决与经验**

- 每次下载模式切换后先 `ls /dev/cu.*`
- 使用最新枚举出来的串口
- 最后改用更稳定的 `usbserial` 物理口完成最新版烧录

---

## 是否接入回声消除（AEC）

当前这版 `peer_demo` **没有接入 AEC**。

原因是当前采集链路使用的是：

- `esp_capture_new_audio_dev_src(...)`

而不是带 AEC 的：

- `esp_capture_new_audio_aec_src(...)`

板级初始化也没有开启：

- `CODEC_I2S_MODE_TDM`
- `in_use_tdm = true`

因此当前版本是：

> 普通麦克风采集 + G711A 编码 + WebRTC 发送

并非带参考通道的声学回声消除版本。

如果后续要接入 AEC，可参考 `solutions/openai_demo` 的采集方式。

---

## 当前验证结论

### 浏览器侧

已验证：

- 收到设备 offer
- 返回 answer
- `Remote stream added`
- `ICE connection state changed to: connected`
- 协商 codec 为 `PCMA/8000`
- 候选对与 DTLS / SRTP 已建立

### 设备侧

已验证：

- `Current peer state: completed`
- 持续发送音频
- 音频统计出现：

```text
Sent audio packets=51 active=51 silence_like=0 pending=0
```

这说明：

- 设备侧采到了有效的、非静音型音频数据
- 当前版本的上行音频链路是工作的

---

## 操作步骤

### 1. 修改配置

编辑：

- `solutions/peer_demo/main/settings.h`

确认：

- `TEST_BOARD_NAME`
- `WIFI_SSID`
- `WIFI_PASSWORD`
- `DEFAULT_PLAYBACK_VOL`

### 2. 编译

```bash
cd /Users/jiangzhongyang/work/esp32/esp-webrtc-solution/solutions/peer_demo
source ~/esp/esp-idf/export.sh
idf.py build
```

### 3. 进入下载模式（若自动下载不稳）

1. 按住 `BOOT`
2. 按一下 `RESET`
3. 松开 `RESET`
4. 松开 `BOOT`

### 4. 查串口

```bash
ls /dev/cu.*
```

### 5. 烧录

```bash
idf.py -p <PORT> flash
```

### 6. 监视器

```bash
idf.py -p <PORT> monitor
```

### 7. 常用命令

- `i`
- `rec2play`
- `start roomxxxx`
- `stop`

### 8. 浏览器联调

打开：

```text
https://webrtc.espressif.com/r/roomxxxx
```

---

## 注意事项

- 原生 USB `usbmodem*` 口会重枚举，不要把旧端口号写死
- `Korvo-2` 需要外接喇叭才能验证板端播放
- `Remote stream added` 不等于一定有有效拾音，仍应结合设备侧 `active/silence_like` 日志判断
- 当前版本无 AEC，喇叭与麦克风同时开时可能存在回声

---

## 总结

本次改造最关键的结论有四条：

1. `peer_demo` 要想便于定位，必须切到源码版 `libpeer`
2. 浏览器无声的真正根因是 `libpeer` 的 RTP/RTCP bitfield 头在小端平台上的字节序错误
3. 本地硬件链路必须优先用 `rec2play` 排查
4. 烧录稳定性的核心不在 IDF 版本，而在串口链路与端口重枚举管理

当前仓库中的 `peer_demo` 已经具备继续做 WebRTC 音频联调与后续 AEC 改造的基础。
