# PR 最终说明：`peer_demo` 源码版 `libpeer` 集成与真实音频链路改造

## 标题建议

`feat: integrate source libpeer and enable real audio pipeline in peer_demo`

## 建议目标分支

`release/v1.2`

## 本次 PR 包含的提交

- `5161bcc` `feat: integrate source libpeer and real audio peer demo`
- `02a303d` `docs: add auxiliary libpeer materials and PR draft`
- `e5bedfb` `docs: add environment baseline for peer demo`
- `9e4af4f` `build: expose libpeer public headers`

---

## 背景

当前 `solutions/peer_demo` 原始实现更偏向基础示例，存在以下问题：

- PeerConnection 核心路径依赖预编译实现，不便定位运行期问题
- 应用层对真实麦克风采集 / 板端播放支持不足
- 浏览器侧 WebRTC 音频联调过程中，协议层问题难以直接分析

本 PR 的目标是把 `peer_demo` 整理为一个：

- 可源码定位的 WebRTC demo
- 支持真实麦克风采集与板端音频播放
- 可与浏览器进行实际 PCMA 音频联调
- 具备基础排障能力与文档沉淀

---

## 环境基线

本次改造、编译与烧录验证基于如下环境：

- 主机系统：`macOS 14.6.1 (23G93)`
- `ESP-IDF`：`v5.4-dirty`，提交 `67c1de1eeb`
- `idf.py` 使用 Python：`~/.espressif/python_env/idf5.4_py3.13_env/bin/python` `3.13.2`
- 系统 Python：`3.14.3`（未直接用于构建）
- `ESP-ADF`：`v2.7-99-geca11f20`，提交 `eca11f20`
- `xtensa-esp-elf-gcc`：`14.2.0`
- `esptool.py`：`4.11.0`

补充说明：

- `peer_demo` 当前实际构建方式为 `ESP-IDF + 本仓组件 + dependencies.lock`
- `dependencies.lock` 锁定 `idf` 为 `5.4.0`
- 因此当前建议保持 `ESP-IDF v5.4` 基线，不建议在问题定位阶段先切到 `v5.5.1`

---

## 摘要

本 PR 将 `solutions/peer_demo` 从“偏示例性质的聊天/DataChannel demo”整理为一个可源码定位、可真实采音、可板端播放、可与浏览器做 WebRTC 音频联调的版本，并把排障资料一并沉淀进仓库。

交付结果包括：

- `peer_demo` 切换为源码版 `libpeer`
- 接入真实麦克风采集、G711A 编码发送与板端播放链路
- 新增 `rec2play` 本地回环测试命令
- 修复浏览器无声、远端 candidate 处理缺失、控制台不可输入、烧录不稳定等问题
- 补充环境基线、操作步骤、问题分析与 PR 文档
- 补齐 `libpeer` 公共头导出，方便组件边界更清晰

---

## 主要改动

### 1. 引入源码版 `libpeer`

- 新增 `components/libpeer/` 源码组件
- `peer_demo` 不再依赖不可见的预编译 PeerConnection 路径做核心定位
- 现在 ICE / STUN / DTLS / SRTP / RTP / RTCP / SCTP / SDP 均可直接在源码层排查
- 导出 `components/libpeer/include/` 作为公共头目录，并在 `CMake` 中显式暴露 `include`

### 2. `peer_demo` 改为真实音频链路

- 新增 `solutions/peer_demo/main/board.c`
- 新增 `solutions/peer_demo/main/media_sys.c`
- 更新 `solutions/peer_demo/main/main.c`
- 更新 `solutions/peer_demo/main/webrtc.c`

实现内容：

- 板端 codec 初始化
- 麦克风采集
- G711A 编码发送
- 浏览器下行音频板端播放
- `rec2play` 本地回环测试

### 3. 修复多个协议与兼容性问题

#### RTP / RTCP 头字节序修复

- 修复 `libpeer` 使用 bitfield 构造 RTP/RTCP 头导致的小端平台网络字节序错误
- 这是浏览器无声的核心根因之一

#### SDP 浏览器兼容增强

- 增加 `msid-semantic`
- 补充 `msid` / `ssrc` 相关字段
- 增加 `ptime` / `maxptime`
- 使用更兼容的 `UDP/TLS/RTP/SAVPF`

#### STUN 兼容修复

- 忽略 Chromium 私有 STUN 属性 `0xc057`

#### Keepalive 调整

- 增大 `CONFIG_KEEPALIVE_TIMEOUT`

#### 应用层 candidate 补齐

- 在 `webrtc.c` 中补齐远端 trickle ICE candidate 处理

#### 信令线程栈风险规避

- 避免在 WebSocket 回调线程里直接跑较深的 `create_offer/create_answer` 流程
- 改为独立调度线程处理协议深栈路径

### 4. 调整控制台与分区配置

- ESP32-S3 切换为 `USB-Serial/JTAG` 主控制台
- 自定义 `3M` app 分区
- Flash 设定为 `4MB`

---

## 额外补充文件

本 PR 一并补充了辅助材料：

- `components/libpeer/examples/`
- `components/libpeer/tests/`
- `components/libpeer/.github/`
- `solutions/peer_demo/LIBPEER_FIX_ANALYSIS.md`
- `docs/peer_demo源码集成与问题排查全记录.md`

这些内容用于：

- 保留源码组件完整上下文
- 保留测试与示例参考
- 保留本次问题分析过程
- 方便后续继续维护与排障

另外，本次还补充了：

- `docs/PR_release_v1.2_peer_demo源码版libpeer与真实音频.md`
  - 当前这份 PR 最终说明

---

## 验证情况

### 本地验证

- `idf.py build` 通过
- `peer_demo.bin` 落入当前 `3M` app 分区
- 新固件已成功烧录到 ESP32-S3 设备
- `components/libpeer/include/` 导出后再次构建通过

### 设备侧验证

- `rec2play` 可用于本地回环验证
- WebRTC 建连后设备侧出现：
  - `Current peer state: completed`
  - `Sent audio packets=... active=... silence_like=...`
- 实际联调中可见：
  - `Sent audio packets=51 active=51 silence_like=0 pending=0`

### 浏览器侧验证

- 浏览器成功接收 offer 并返回 answer
- `Remote stream added`
- `ICE connection state changed to: connected`
- 浏览器 `getStats()` 中可见：
  - candidate pair succeeded
  - transport connected
  - codec 为 `audio/PCMA`

### 环境基线验证

- 当前文档已记录可复现环境：
  - `macOS 14.6.1`
  - `ESP-IDF v5.4-dirty` / `idf 5.4.0`
  - `ESP-ADF v2.7-99-geca11f20`
  - `xtensa-esp-elf-gcc 14.2.0`
  - `esptool.py 4.11.0`

---

## 风险与说明

### 1. 当前版本未接入 AEC

当前 `peer_demo` 采用普通音频采集：

- `esp_capture_new_audio_dev_src(...)`

尚未切换到：

- `esp_capture_new_audio_aec_src(...)`

因此当前版本没有声学回声消除，喇叭与麦克风同时启用时可能存在回声。

### 2. 原生 USB 口会重枚举

如果使用 `usbmodem*` 原生 USB 口，刷写和监视过程中端口名可能变化。

### 3. 本 PR 包含较多辅助文件

这是有意为之，用于保留 `libpeer` 源码集成后的完整上下文与排障资料。

### 4. 构建基线建议保持 `ESP-IDF v5.4`

- `dependencies.lock` 当前锁定 `idf` 为 `5.4.0`
- 因此本 PR 不建议在问题定位阶段先切到 `v5.5.1`
- 若后续要升级 IDF，建议作为单独变更处理

---

## 对 Reviewer 的重点说明

建议重点关注以下几类变更：

1. `libpeer` 源码集成是否满足后续可维护性需求
2. RTP/RTCP 头修复是否覆盖浏览器无声的根因
3. `peer_demo` 的真实音频链路是否符合当前 Korvo-2 使用场景
4. 控制台、分区和串口策略是否足够稳健
5. 辅助文档与源码材料的保留范围是否合适

---

## 后续建议

后续可继续推进：

1. 将 `peer_demo` 改造成 AEC 版本
2. 继续优化浏览器 / 设备双向音频体验
3. 视需要收敛 `libpeer` 辅助文件范围
4. 增加更系统的联调验证脚本与说明
