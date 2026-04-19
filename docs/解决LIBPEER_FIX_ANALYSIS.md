# `LIBPEER_FIX_ANALYSIS.md` 复核结论与修复方案

## 结论

`esp-webrtc-solution/solutions/peer_demo/LIBPEER_FIX_ANALYSIS.md` 中“`components/libpeer/src/agent.c` 的 `agent_gather_candidate()` 没有收集 host candidate”这一结论，**不适用于当前 `esp_peer` 实际链接的预编译库**。

复核依据：

1. `peer_demo` 实际运行时使用的是 `esp-webrtc-solution/components/esp_peer/libs/esp32s3/libpeer_default.a`。
2. 对该归档内的 `agent.c.obj` 反汇编后可见，`agent_gather_candidate()` 在检查 `urls == NULL` 之前，已经调用了：
   - `agent_create_bind_addr`
   - `udp_get_local_address`
   - `agent_create_host_addr`
   - `agent_add_local_candidate`
3. 因此，直接克隆一个开源 `libpeer` 仓库再尝试“源码覆盖预编译库”会遇到两个问题：
   - 版本/ABI 不一致
   - `esp_peer_get_default_impl()` 仍然只在预编译库里提供

## 为什么会编译失败

之前的尝试把 `esp-webrtc-solution/components/libpeer/` 作为源码组件加入了构建，并修改了 `esp-webrtc-solution/components/esp_peer/CMakeLists.txt` 去同时链接：

- `esp_peer` 自己的预编译 `libpeer_default.a`
- 新增的 `libpeer` 源码组件

这会带来两类冲突：

1. **链接顺序被破坏**
   - `dtls_srtp_*` 等符号不再按原组件关系被正确解析。
2. **同名实现重复/版本不一致**
   - 新增 `libpeer` 源码与预编译归档中的内部实现不是同一套代码。

## 本次修复

本次采用的方案是：

1. 保留 `esp_peer` 组件原有结构，不再尝试把外部 `libpeer` 源码与 `libpeer_default.a` 混链。
2. 直接将开源 `libpeer` 源码接入 `peer_demo`，使 `PeerConnection` 路径可源码编译、可加日志、可单步定位。
3. 在 `libpeer` 源码里补上 remote trickle candidate 到达后的 `candidate pair` 更新。
4. 在 `peer_demo` 自身修复两个运行期问题：
   - 增加信令层 `ESP_PEER_SIGNALING_MSG_CANDIDATE` 处理
   - 将 `create_offer/create_answer` 从 `websocket_task` 挪到独立调度线程，避免栈溢出

这样做的好处：

- 不再受 `esp_peer_get_default_impl()` 预编译实现限制。
- `peer_demo` 的 ICE / SDP / DTLS / SRTP / SCTP 路径现在都可以直接看源码和打日志。
- 运行时已验证源码版可以正常启动、联网、建房并发送 offer，且不再触发 `websocket_task` 栈溢出。

## 额外发现

在复核 `peer_demo` 应用层逻辑时，还发现一个明确的业务 bug：

1. 原 `esp-webrtc-solution/solutions/peer_demo/main/webrtc.c` 只处理 `SDP`，没有处理 `candidate`。
2. `apprtc_signal` 实现实际会把远端 trickle ICE candidate 以 `ESP_PEER_SIGNALING_MSG_CANDIDATE` 回调出来。

这意味着即使底层库本身没问题，应用层也可能因为没有把远端 candidate 喂给 PeerConnection 而导致迟迟无法连通。

## 验证方式

在 `esp-webrtc-solution/solutions/peer_demo` 目录执行：

```bash
source ~/esp/esp-idf/export.sh
idf.py build
```

如果设备已连接，可继续执行：

```bash
idf.py -p /dev/cu.SLAB_USBtoUART flash monitor
```

## 额外说明

- 如果后续仍需继续分析运行时 ICE 行为，建议以 **当前实际链接的 `libpeer_default.a`** 为准，不要再以外部克隆的 `libpeer` 源码直接代入结论。
- 若要继续做二进制级定位，可优先检查 `agent.c.obj`、`peer_default.c.obj` 的反汇编与运行日志是否一致。
