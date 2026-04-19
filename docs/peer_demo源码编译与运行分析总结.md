# `peer_demo` 源码编译与运行分析总结

## 背景

本次目标是解决 `esp-webrtc-solution/solutions/peer_demo` 在 WebRTC 场景下的两类问题：

1. 预编译 `libpeer_default.a` 不便于定位运行期问题
2. 原有方案在切换到源码编译时出现构建冲突，且运行期连接行为不稳定

最终目标不是单纯“编过”，而是让 `peer_demo` 的 `ICE / SDP / DTLS / SRTP / SCTP / DataChannel` 路径全部进入可读、可改、可加日志的源码状态。

---

## 结论概览

本次排查后，最终结论如下：

1. 直接尝试“开源 `libpeer` 源码 + `esp_peer` 预编译 `libpeer_default.a` 混链”不可行
2. `peer_demo` 原应用层确实存在一个明显逻辑缺陷：**只处理 SDP，不处理远端 trickle ICE candidate**
3. 将 `create_offer()` / `create_answer()` 直接放在信令回调线程里执行，会导致 `websocket_task` 栈溢出
4. 改为：**`peer_demo` 直接使用开源 `libpeer` 源码实现 PeerConnection**，可以同时解决“难定位”和“可改造”两个问题
5. 当前结果是：
   - 源码编译通过
   - 烧录通过
   - 设备联网、建房、发送 offer 正常
   - 运行期不再出现 `websocket_task` 栈溢出
   - 设备端已经收到数据，可继续基于源码做后续协议级定位

---

## 一、为什么最初的源码替换方案会失败

### 1. `esp_peer` 默认实现并不开源

`peer_demo` 原先通过：

```c
esp_peer_open(&cfg, esp_peer_get_default_impl(), &peer);
```

来获取默认 PeerConnection 实现。

问题在于：

- `esp_peer_get_default_impl()` 只在预编译库 `libpeer_default.a` 中提供
- 仓库中没有 `peer_default.c` 这类可直接编译的源码实现
- 因此无法只替换其中某一个底层文件，而保留其余“glue code”不变

### 2. 混合链接会导致符号/版本冲突

之前的思路是：

- 保留 `components/esp_peer/libs/esp32s3/libpeer_default.a`
- 再额外引入一个开源 `components/libpeer/`

这会带来两个根本问题：

1. **同名内部实现重复**
   - `agent.c`、`ice.c`、`stun.c` 等内部符号在源码和预编译归档中各有一份
2. **版本/ABI 不一致**
   - 预编译库中的内部结构、符号集合、调用关系并不一定和后来克隆的开源 `libpeer` 一致

结果就是：

- 要么缺少 `esp_peer_get_default_impl()`
- 要么出现多重定义 / 未定义引用 / 链接顺序问题

也就是说，这条路本质上不是“补一个小 patch”，而是“把一套半开半闭的实现拆开重组”，风险很高。

---

## 二、复核后发现的真实运行期问题

### 1. 应用层没有处理远端 trickle ICE candidate

`apprtc_signal` 实际会把远端 candidate 以 `ESP_PEER_SIGNALING_MSG_CANDIDATE` 回调出来。

但原 `peer_demo` 中的信令处理只处理了：

- `BYE`
- `SDP`

没有处理 `CANDIDATE`。

这意味着：

- 即使底层 ICE agent 正常
- 即使本地 SDP 已经发出
- 远端后续 trickle 下来的 candidate 也没有被送进 PeerConnection

这类问题会直接导致：

- pairing 长时间没有进展
- 状态停留在早期阶段
- 连接行为表现异常，且很容易被误判为底层库 bug

### 2. offer / answer 创建放在信令线程里会导致栈溢出

切到源码版 `libpeer` 后，`create_offer()` / `create_answer()` 的执行路径完整进入本地源码，可观察到：

- SDP 生成
- STUN candidate 收集
- 字符串拼接
- DTLS / ICE 相关初始化

这些操作直接在 `websocket_task` 回调中执行时，会让信令线程承受过重栈压力。

实际复现中已经出现：

```text
***ERROR*** A stack overflow in task websocket_task has been detected.
```

因此，除了“用源码编译”，还必须做线程职责拆分。

---

## 三、最终采用的解决方案

### 方案核心

不再走：

- `esp_peer_get_default_impl()`
- `libpeer_default.a`

而是改为：

**让 `peer_demo` 直接使用开源 `libpeer` 的 `PeerConnection` API。**

这样做的收益非常直接：

1. 所有连接逻辑都在源码里
2. 可以直接在 `agent.c / peer_connection.c / stun.c / dtls_srtp.c` 打日志
3. 后续再定位 candidate、pairing、DTLS、SCTP、DataChannel 时不会被预编译库阻断

---

## 四、具体改动内容

### 1. 新增源码组件 `components/libpeer`

将开源 `libpeer` 直接纳入工程，并改造成 ESP-IDF 可编译组件。

关键文件：

- `esp-webrtc-solution/components/libpeer/CMakeLists.txt`

处理内容包括：

- 显式列出编译源文件
- 使用 `espressif__esp_libsrtp`
- 打开 `CONFIG_USE_LWIP=1`
- 关闭 `CONFIG_USE_USRSCTP`
- 让其适配 ESP-IDF 当前依赖体系

### 2. `peer_demo` 不再走 `esp_peer_get_default_impl()`

改为在应用层直接创建源码版 `PeerConnection`：

- `peer_init()`
- `peer_connection_create()`
- `peer_connection_create_offer()`
- `peer_connection_set_remote_description()`
- `peer_connection_add_ice_candidate()`
- `peer_connection_loop()`

关键文件：

- `esp-webrtc-solution/solutions/peer_demo/main/webrtc.c`

### 3. 补上远端 candidate 处理

在 `signaling_msg_handler()` 中新增：

- `ESP_PEER_SIGNALING_MSG_CANDIDATE` 分支
- 将收到的 candidate 送入 `peer_connection_add_ice_candidate()`

同时在源码 `libpeer` 中补充：

- remote candidate 到达后立即更新 candidate pairs
- 必要时将状态推进回 `checking`

关键文件：

- `esp-webrtc-solution/solutions/peer_demo/main/webrtc.c`
- `esp-webrtc-solution/components/libpeer/src/peer_connection.c`

### 4. 将 offer / answer 生成挪到独立线程

新增异步任务：

- `pc_offer`
- `pc_remote`

并在 `thread_scheduler()` 中给这些线程分配较大的栈：

- `25 * 1024`

这一步直接解决了：

- `websocket_task` 栈溢出

关键文件：

- `esp-webrtc-solution/solutions/peer_demo/main/webrtc.c`
- `esp-webrtc-solution/solutions/peer_demo/main/main.c`

### 5. 修正 `libpeer` 在 ESP-IDF 下的适配问题

为了让源码版 `libpeer` 真正可编译，还处理了几类兼容性问题：

1. `libsrtp` 头文件路径差异
   - `srtp2/srtp.h` 改为 `srtp.h`
2. `esp_libsrtp` 的 `protect/unprotect` 函数签名与上游版本不同
   - 调整为 ESP 版签名
3. STUN 可选属性识别
   - `SOFTWARE (0x8022)`
   - `NETWORK_COST (0x802b)`

这一步的收益是：

- 编译链路稳定
- 日志噪音下降
- 后续更容易看真正的异常点

---

## 五、验证结果

### 1. 编译结果

已完成源码版构建：

```bash
cd /Users/jiangzhongyang/work/esp32/esp-webrtc-solution/solutions/peer_demo
source ~/esp/esp-idf/export.sh
idf.py build
```

结果：

- `peer_demo.elf` 生成成功
- `peer_demo.bin` 生成成功

### 2. 烧录结果

已完成烧录：

```bash
idf.py -p /dev/cu.SLAB_USBtoUART flash
```

结果：

- 烧录成功
- 设备正常重启

### 3. 运行结果

串口日志已验证：

1. 设备正常启动
2. 正常连接 Wi‑Fi
3. 正常获取 APPRTC room / client / ICE 信息
4. 正常解析并使用 STUN 服务器
5. 正常发送 offer
6. 不再触发 `websocket_task` stack overflow
7. 设备端已经收到数据

这说明本次改造已经达到“源码可编译 + 运行可进入有效调试状态”的目标。

---

## 六、本次工作的价值

这次工作的价值不只是“把 demo 修到能跑”，更重要的是完成了以下转变：

### 1. 从“二进制黑盒”转为“源码可定位”

之前定位问题时，关键逻辑藏在：

- `libpeer_default.a`

现在关键路径都在源码中，可以直接跟踪：

- ICE candidate 收集
- STUN 交互
- remote candidate 注入
- candidate pair 更新
- SDP 创建
- DTLS / SRTP 初始化
- SCTP / DataChannel 行为

### 2. 从“猜底层 bug”转为“能验证真实根因”

本次排查证明：

- 不是所有问题都来自底层库
- 应用层遗漏 `candidate` 处理本身就是一个关键问题
- 线程模型和栈大小同样会直接影响 WebRTC 建连过程

### 3. 为下一步协议级调试打下基础

后续如果还要继续分析：

- 为什么远端没有回 SDP
- 为什么房间侧浏览器行为异常
- 为什么某些网络环境下 candidate 不全
- 为什么 DTLS / SCTP 状态推进异常

现在都可以直接在源码上继续推进，而不是被预编译库卡住。

---

## 七、建议的后续动作

建议后续继续按下面顺序推进：

1. **浏览器侧对照抓包 / 控制台日志**
   - 观察远端是否成功收到 offer
   - 观察远端是否回 answer / candidate

2. **在 `libpeer` 源码中继续加状态日志**
   - `agent_update_candidate_pairs()`
   - `agent_select_candidate_pair()`
   - `agent_connectivity_check()`
   - `peer_connection_set_remote_description()`

3. **对房间侧信令做端到端核对**
   - 确认 offer POST 成功后，远端页面是否真正加入房间
   - 确认 answer/candidate 是否正确回流到设备侧

4. **如需长期维护，考虑把源码版 `peer_demo` 抽成单独实现分支**
   - 便于继续调试
   - 避免和官方默认预编译实现混淆

---

## 相关文件

本次分析和改动重点集中在以下文件：

- `esp-webrtc-solution/solutions/peer_demo/main/webrtc.c`
- `esp-webrtc-solution/solutions/peer_demo/main/main.c`
- `esp-webrtc-solution/solutions/peer_demo/main/CMakeLists.txt`
- `esp-webrtc-solution/components/libpeer/CMakeLists.txt`
- `esp-webrtc-solution/components/libpeer/src/peer_connection.c`
- `esp-webrtc-solution/components/libpeer/src/dtls_srtp.c`
- `esp-webrtc-solution/components/libpeer/src/stun.c`
- `esp-webrtc-solution/components/libpeer/src/stun.h`
- `docs/解决LIBPEER_FIX_ANALYSIS.md`

---

## 最终结论

本次问题已经从“预编译库难排查、源码替换又冲突”的死胡同，转成了“源码版 `PeerConnection` 可编译、可运行、可继续定位”的状态。

可以认为本阶段已经完成以下里程碑：

- `peer_demo` 源码编译成功
- 板端烧录运行成功
- 关键运行期栈溢出问题已修复
- 远端 candidate 处理缺失问题已修复
- 设备端已收到数据
- 后续调试已具备源码级可观测性

如果后续继续深入，建议直接以当前源码版 `libpeer` 路径为基础继续演进，不再回退到预编译 `libpeer_default.a` 方案。
