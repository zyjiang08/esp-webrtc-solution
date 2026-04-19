# `peer_demo` 浏览器无声问题分析与修复

## 现象

`peer_demo` 已经切换为：

- 真实麦克风采集
- 真实喇叭播放
- `libpeer` 源码编译

设备侧日志也能看到：

- `Peer state: completed`
- `Real audio stream started`
- `Sent audio packets=...`

但浏览器侧一直听不到来自 ESP32-S3 的音频。

---

## 先排除的几个方向

### 1. 不是“板子没有喇叭”导致浏览器无声

`ESP32-S3-Korvo-2 V3.1` 确实需要外接喇叭，这会影响 **板端本地播放**，但不会影响 **浏览器接收设备上行音频**。

所以“浏览器听不到声音”的关键链路是：

`麦克风采集 -> G711 编码 -> RTP -> SRTP -> 浏览器解码播放`

而不是板载喇叭本身。

### 2. 不是采集线程没启动

设备侧已经明确出现：

- `AUD_SRC: Start to fetch audio src data now`
- `ESP_GMF_AENC: Open, type:G711-Alaw`
- `PEER_DEMO: Sent audio packets=...`

说明采集和编码已经在工作。

### 3. 不是 SDP/ICE 完全没连上

连接过程中可以到达：

- `checking`
- `connected`
- `completed`
- `Data channel opened`

说明 ICE / DTLS / SCTP 主流程已建立，问题更像是 **媒体 RTP 包本身不被浏览器识别**。

---

## 根因

根因在 `libpeer` 的 RTP/RTCP 头定义方式。

### 问题点

原实现把 RTP/RTCP 头的前 16bit 写成了 C bitfield，例如 RTP 头：

- `version`
- `padding`
- `extension`
- `csrccount`
- `markerbit`
- `type`

这类 bitfield 在 **小端平台** 上的内存布局并不等于网络字节序。

ESP32-S3 是小端，因此实际发出的 RTP 头字节序是错的。

### 直接证据

按原来的 `RtpHeader` 写法，在小端主机上构造一个典型 PCMA RTP 包，本来前两个字节应该是：

```text
80 08
```

即：

- `0x80` -> RTP version 2
- `0x08` -> payload type 8 (PCMA)

但实际得到的是：

```text
02 10
```

这说明浏览器收到的并不是合法 RTP 头，因而不会把它当成可播放音频流。

### 为什么之前日志看起来“像在发音频”

因为设备侧日志里的：

```text
Sent audio packets=51
```

只能说明：

- 采集到了音频
- 调用了 `peer_connection_send_audio()`
- 走到了 RTP 编码与发送路径

但**不代表浏览器能把收到的数据识别成合法 RTP 音频包**。

---

## 本次修复

### 1. 修正 RTP 头编码与解析

文件：`esp-webrtc-solution/components/libpeer/src/rtp.h`

将原来的 16bit bitfield 头改为明确的网络字节布局：

- `flags`
- `payload_type`
- `seq_number`
- `timestamp`
- `ssrc`

同时增加显式 helper：

- `rtp_header_set_version()`
- `rtp_header_set_marker()`
- `rtp_header_set_payload_type()`
- `rtp_header_get_version()`
- `rtp_header_get_payload_type()`

文件：`esp-webrtc-solution/components/libpeer/src/rtp.c`

同步把以下路径全部改成显式按位设置：

- `rtp_encoder_encode_generic()`
- `rtp_encoder_encode_h264_single()`
- `rtp_encoder_encode_h264_fu_a()`
- `rtp_packet_validate()`

这样可以保证：

- 发包时浏览器看到的是合法 RTP 头
- 收包时设备也能正确解析浏览器发来的 RTP 头

### 2. 修正 RTCP 头编码与解析

文件：`esp-webrtc-solution/components/libpeer/src/rtcp.h`

同样移除了 16bit bitfield，改成：

- `flags`
- `type`
- `length`

并新增：

- `rtcp_header_set_version()`
- `rtcp_header_set_rc()`
- `rtcp_header_get_version()`
- `rtcp_header_get_rc()`

文件：`esp-webrtc-solution/components/libpeer/src/rtcp.c`

同步修复：

- `rtcp_probe()`
- `rtcp_get_pli()`
- `rtcp_get_fir()`

文件：`esp-webrtc-solution/components/libpeer/src/peer_connection.c`

同步修复 RTCP FMT/RC 的读取逻辑，避免继续按旧 bitfield 解释。

### 3. 顺手修复 `start/stop` 并发导致的崩溃

在联调过程中，连续 `stop` / `start` 会触发：

```text
Guru Meditation Error: Core 0 panic'ed (LoadProhibited)
```

回溯定位到：

- `destroy_wss()`
- `wss_signal_stop()`
- `esp_peer_signaling_stop()`

也就是信令 stop 被并发重入。

因此在：

文件：`esp-webrtc-solution/solutions/peer_demo/main/webrtc.c`

增加了递归互斥锁，对：

- `start_webrtc()`
- `stop_webrtc()`

做串行保护，避免控制台快速连按 `stop/start` 时再次把 WebSocket 信令撞崩。

---

## 编译结果

在下面目录重新编译通过：

```bash
/Users/jiangzhongyang/work/esp32/esp-webrtc-solution/solutions/peer_demo
```

构建结果：

- `peer_demo.bin` 大小约 `0x242a10`
- 仍可放入当前 `3MB` app 分区

---

## 本轮验证结论

### 浏览器日志新增结论

后续拿到的浏览器日志里，已经出现了以下关键信号：

- `Sending answer to peer.`
- `Remote stream added.`
- `Set remote session description success.`
- `ICE connection state changed to: connected`

这几条说明：

1. 浏览器已经成功接收并接受了 ESP32 发出的 offer。
2. 浏览器已经为远端音频轨创建了 `MediaStream`。
3. ICE 连通性已经建立，不再是“房间没进来”或“candidate 没打通”的问题。

同时，浏览器 answer 里明确只协商了：

- `m=audio ... 8`
- `a=rtpmap:8 PCMA/8000`

这说明 **PCMA 编解码协商本身也是成功的**。

因此，问题进一步收敛为：

- 设备虽然已经建立了 WebRTC 音频通道，
- 但发出的音频 RTP 内容仍可能是“静音/近静音”，或者媒体面上仍有残余问题。

### 新增的设备侧诊断

为了继续区分“协议通了但麦克风几乎是静音”这种情况，已经在 `peer_demo` 里新增了一条发送统计日志。

文件：`esp-webrtc-solution/solutions/peer_demo/main/webrtc.c`

现在每秒会输出：

```text
Sent audio packets=51 active=48 silence_like=3 pending=0
```

其中：

- `active`：不像 G711A 静音包的发送包数
- `silence_like`：看起来像 `0xD5/0x55` 静音模式的包数

如果后续现场日志长期表现为：

```text
active=0 silence_like=51
```

那么就可以基本确认：

> WebRTC 通道已经通了，但设备麦克风侧送出来的编码结果几乎全是静音。

这会把排查重点完全转向：

- 麦克风增益
- 输入通道选择
- 采集格式/采样链路
- 板卡实际拾音效果

### 已确认的结论

1. `peer_demo` 浏览器无声的**核心协议根因**已经定位：
   - 不是单纯采集问题
   - 而是 `libpeer` 的 RTP/RTCP 头 bitfield 在小端平台上生成/解析错误
2. 该问题会同时影响：
   - 设备 -> 浏览器的音频 RTP
   - 浏览器 -> 设备的音频 RTP/RTCP 解析
3. 修复已经完成并通过本地源码编译。

### 尚待你现场再确认的一点

在我准备继续自动化回归时，串口设备临时从 macOS 枚举里消失，导致无法继续直接刷写并做最后一轮板端联调。

也就是说：

- **源码修复已完成并编译通过**
- **最终板端听感确认** 还需要你把板子重新连上串口后再跑一轮

---

## 建议的现场复测步骤

### 1. 重新连接开发板串口

确认 macOS 下重新出现串口，例如：

```bash
ls /dev/cu.*
```

### 2. 重新烧录最新 `peer_demo`

```bash
cd /Users/jiangzhongyang/work/esp32/esp-webrtc-solution/solutions/peer_demo
source ~/esp/esp-idf/export.sh
idf.py -p <你的串口> flash monitor
```

### 3. 浏览器与设备联调

串口中执行：

```text
start roomxxxx
```

浏览器打开：

```text
https://webrtc.espressif.com/r/roomxxxx
```

### 4. 重点看两个现象

#### 浏览器侧

- 是否出现远端流
- 是否能听到 ESP32 麦克风声音

#### 设备侧

- 连接是否到 `completed`
- 是否持续出现 `Sent audio packets=...`
- 是否还会出现异常关闭或崩溃

---

## 总结

这次“浏览器无声”不是应用层“没开始采音”，而是更底层的协议实现问题：

> `libpeer` 使用 16bit bitfield 构造 RTP/RTCP 头，在 ESP32-S3 这类小端平台上会产生错误的网络字节布局，导致浏览器无法把设备发出的数据识别成合法 WebRTC 音频 RTP 包。

本次已经把该问题在源码层修正，并顺手补上了 `start/stop` 并发保护。后续只需要你在串口重新枚举后做一次现场复测，即可确认浏览器端音频是否恢复。
