# ESP32 WebRTC Host Candidate 收集问题分析与修复记录

## 问题描述

ESP32-S3-Korvo-2 V3.1 开发板在使用 WebRTC 进行 P2P 连接时，无法成功建立连接。

### 错误现象
```
E (75039) PEER_CONNECTION: Not found for bind remote 192.168.124.103:54686
E (75039) PEER_CONNECTION: Not found for bind remote 192.168.124.103:54686
```

### 根本原因
通过分析发现，ESP32 端没有收集 **host candidate**（本地网络地址），只收集了 srflx candidate（通过 STUN 服务器获取的公网地址）。

**libpeer 源码 bug 位置**: `components/libpeer/src/agent.c` 的 `agent_gather_candidate` 函数

```c
void agent_gather_candidate(Agent* agent, const char* urls, ...) {
  // ...

  // BUG: 只有当 urls == NULL 时才收集 host candidates
  if (urls == NULL) {
    agent_create_host_addr(agent);
    return;
  }

  // 如果提供了 STUN/TURN URL，直接跳过 host candidate 收集
  // 这违反了 RFC 5245 规范
}
```

**问题**: ESP32 应用总是提供 STUN 服务器 URL，导致永远不会收集 host candidates。

---

## 修复方案对比

### 方案 A: 修改 libpeer 源码（已尝试，遇到技术障碍）

**思路**:
1. 克隆 libpeer 源码到 components 目录
2. 修复 agent.c 的 bug
3. 替换预编译库，使用修改后的源码编译

**实施步骤**:

#### 1. 备份原始组件
```bash
cp -r components/esp_peer components/esp_peer.backup
```

#### 2. 克隆并修复 libpeer 源码
```bash
cd /private/tmp
git clone https://github.com/sepfy/libpeer.git
cp -r /private/tmp/libpeer components/
```

**修复代码** (`components/libpeer/src/agent.c:260-293`):
```c
void agent_gather_candidate(Agent* agent, const char* urls, const char* username, const char* credential) {
  // ... 变量声明 ...

#ifdef ESP_PLATFORM
  ESP_LOGI(TAG, "=== AGENT_GATHER_CANDIDATE CALLED - LIBPEER SOURCE CODE FIX ACTIVE ===");
  ESP_LOGI(TAG, "urls=%s, username=%s", urls ? urls : "NULL", username ? username : "NULL");
#endif

  // FIX: 总是先收集 host candidates（符合 RFC 5245）
  LOGI("=== LIBPEER SOURCE CODE FIX ACTIVE === Gathering host candidates first");
#ifdef ESP_PLATFORM
  ESP_LOGI(TAG, "Gathering host candidates first (before checking urls)");
#endif
  agent_create_host_addr(agent);
  LOGI("=== Host candidates gathered ===");
#ifdef ESP_PLATFORM
  ESP_LOGI(TAG, "Host candidates gathered, count=%d", agent->local_candidates_count);
#endif

  // 如果没有提供 STUN/TURN 服务器，在收集完 host candidates 后返回
  if (urls == NULL) {
    LOGI("No STUN/TURN server, returning after host candidates");
#ifdef ESP_PLATFORM
    ESP_LOGI(TAG, "No STUN/TURN server, returning after host candidates");
#endif
    return;
  }

  // 继续收集 STUN/TURN candidates...
}
```

#### 3. 配置 libpeer 组件

**创建 CMakeLists.txt** (`components/libpeer/CMakeLists.txt`):
```cmake
idf_component_register(
    SRCS
        "src/address.c"
        "src/agent.c"
        "src/base64.c"
        "src/dtls_srtp.c"
        "src/ice.c"
        "src/mdns.c"
        "src/peer.c"
        "src/peer_connection.c"
        "src/ports.c"
        "src/rtcp.c"
        "src/rtp.c"
        "src/sctp.c"
        "src/sdp.c"
        "src/socket.c"
        "src/stun.c"
        "src/utils.c"
    INCLUDE_DIRS
        "src"
    PRIV_REQUIRES
        mbedtls
        espressif__esp_libsrtp
)

target_compile_definitions(${COMPONENT_LIB} PUBLIC
    ESP_PLATFORM
    CONFIG_USE_LWIP=1
    CONFIG_IPV6=0
    CONFIG_IFACE_PREFIX=""
)
```

**创建 Kconfig** (`components/libpeer/Kconfig`):
```kconfig
menu "libpeer Configuration"
    config LIBPEER_USE_LWIP
        bool "Use LwIP"
        default y

    config LIBPEER_IPV6
        bool "Enable IPv6"
        default n
endmenu
```

#### 4. 修改 esp_peer 组件配置

**修改 CMakeLists.txt** (`components/esp_peer/CMakeLists.txt`):

**尝试 1**: 完全替换预编译库
```cmake
# 使用 libpeer 源码而非预编译库
idf_component_register(INCLUDE_DIRS ./include
                       SRC_DIRS "src"
                       PRIV_REQUIRES mbedtls libpeer esp_timer)

target_include_directories(${COMPONENT_LIB} PRIVATE "${CMAKE_CURRENT_LIST_DIR}/../libpeer/src")
```

**结果**: 链接失败 - `undefined reference to 'esp_peer_get_default_impl'`

**尝试 2**: 混合使用预编译库和源码
```cmake
# 同时使用预编译库（glue code）和 libpeer 源码（覆盖 buggy agent.c）
idf_component_register(INCLUDE_DIRS ./include
                       SRC_DIRS "src"
                       PRIV_REQUIRES mbedtls libpeer esp_timer)

target_include_directories(${COMPONENT_LIB} PRIVATE "${CMAKE_CURRENT_LIST_DIR}/../libpeer/src")

# 链接预编译库
target_link_libraries(${COMPONENT_LIB} PRIVATE "-L${CMAKE_CURRENT_SOURCE_DIR}/libs/${IDF_TARGET}")
target_link_libraries(${COMPONENT_LIB} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/libs/${IDF_TARGET}/libpeer_default.a")
```

**结果**: 链接失败 - 多重定义错误（multiple definition）

---

## 遇到的技术障碍

### 问题 1: 缺少 `esp_peer_get_default_impl` 函数

**错误信息**:
```
undefined reference to `esp_peer_get_default_impl'
/Users/jiangzhongyang/work/esp32/esp-webrtc-solution/solutions/peer_demo/main/webrtc.c:171
```

**调用位置** (`solutions/peer_demo/main/webrtc.c:191`):
```c
int ret = esp_peer_open(&cfg, esp_peer_get_default_impl(), &peer);
```

**函数声明** (`components/esp_peer/include/esp_peer_default.h:74`):
```c
const esp_peer_ops_t *esp_peer_get_default_impl(void);
```

**问题分析**:
- 该函数在头文件中声明，但在任何源文件中都没有实现
- 该函数的实现在预编译库 `libpeer_default.a` 中
- 预编译库包含两部分：
  1. Glue code: `esp_peer_get_default_impl` 及相关实现
  2. 完整的 libpeer 代码（包括有 bug 的 agent.c）

### 问题 2: 无法分离预编译库的两部分

**尝试的解决方案**:
1. **只使用源码**: 缺少 glue code 实现
2. **混合链接**: 符号冲突（libpeer 的所有函数都有两份定义）
3. **使用 --whole-archive**: 强制包含所有符号，导致更严重的冲突

**根本原因**:
- Espressif 的 glue code 源码不开源
- 预编译库是一个整体，无法只提取 glue code 部分
- 创建新的 glue code 需要实现整个 `esp_peer_ops_t` 结构（14个函数）

### 问题 3: 调试日志未出现

**添加的调试日志**:
```c
#ifdef ESP_PLATFORM
  ESP_LOGI(TAG, "=== AGENT_CREATE CALLED - LIBPEER SOURCE CODE ACTIVE ===");
#endif
```

**验证步骤**:
1. ✅ 确认 agent.c 被编译（在 compile_commands.json 中找到）
2. ✅ 确认调试字符串在 agent.c.obj 中（使用 strings 命令）
3. ❌ 调试字符串不在 peer_demo.elf 中
4. ✅ 在预编译库备份中找到 "Start agent as" 字符串

**结论**: ESP32 仍在使用预编译库，而非我们修改的源码

---

## 文件修改记录

### 新增文件

1. **components/libpeer/** (整个目录)
   - 从 GitHub 克隆的 libpeer 源码
   - 修改了 `src/agent.c`
   - 添加了 `CMakeLists.txt` 和 `Kconfig`

### 修改文件

1. **components/libpeer/src/agent.c**
   - 行 15-19: 添加 ESP-IDF 日志支持
   - 行 32-44: 在 `agent_create` 中添加调试日志
   - 行 147-150: 在 `agent_create_host_addr` 中添加调试日志
   - 行 260-293: 修复 `agent_gather_candidate` 函数逻辑

2. **components/esp_peer/CMakeLists.txt**
   - 多次修改，尝试不同的链接策略
   - 最终状态: 尝试混合链接（失败）

3. **components/esp_peer/src/dtls_srtp.c** 和 **dtls_srtp.h**
   - 从 git 恢复（之前被误删）

### 重命名/删除文件

1. **components/esp_peer.backup/**
   - 原始 esp_peer 组件的备份
   - 后来被删除（导致链接问题）

2. **components/esp_peer/libs/esp32s3/libpeer_default.a**
   - 重命名为 `.disabled` (尝试 1)
   - 恢复原名 (尝试 2)

---

## 编译和烧录记录

### 编译命令
```bash
cd /Users/jiangzhongyang/work/esp32/esp-webrtc-solution/solutions/peer_demo
rm -rf build
source ~/esp/esp-idf/export.sh
idf.py build
```

### 编译结果

#### 尝试 1: 只使用源码
```
[1537/1539] Linking CXX executable peer_demo.elf
FAILED: peer_demo.elf
undefined reference to `esp_peer_get_default_impl'
```

#### 尝试 2: 混合链接（--whole-archive）
```
[1537/1539] Linking CXX executable peer_demo.elf
FAILED: peer_demo.elf
collect2: error: ld returned 1 exit status
```
多重定义错误（所有 libpeer 函数都有两份）

#### 尝试 3: 混合链接（普通）
```
[1537/1539] Linking CXX executable peer_demo.elf
FAILED: peer_demo.elf
collect2: error: ld returned 1 exit status
```
同样的多重定义错误

### 烧录测试

由于编译失败，无法进行烧录测试。

---

## 当前状态

### 工作目录状态
```
/Users/jiangzhongyang/work/esp32/esp-webrtc-solution/
├── components/
│   ├── esp_peer/
│   │   ├── CMakeLists.txt (已修改，尝试混合链接)
│   │   ├── libs/esp32s3/libpeer_default.a (已恢复)
│   │   └── src/
│   │       ├── esp_peer.c
│   │       ├── dtls_srtp.c (已恢复)
│   │       └── dtls_srtp.h (已恢复)
│   └── libpeer/ (新增)
│       ├── CMakeLists.txt (新增)
│       ├── Kconfig (新增)
│       └── src/
│           └── agent.c (已修复 bug)
└── solutions/peer_demo/
    ├── build/ (编译失败)
    └── main/webrtc.c (调用 esp_peer_get_default_impl)
```

### Git 状态
```bash
$ git status
On branch release/v1.2
Changes not staged for commit:
  modified:   components/esp_peer/CMakeLists.txt
  modified:   components/esp_peer/src/esp_peer.c

Untracked files:
  components/libpeer/
```

---

## 技术分析总结

### 预编译库结构分析

**libpeer_default.a 包含**:
1. **Glue Code** (不开源):
   - `esp_peer_get_default_impl()` 实现
   - 返回 `esp_peer_ops_t` 结构指针
   - 实现了 14 个操作函数，桥接 esp_peer API 到 libpeer API

2. **libpeer 完整代码**:
   - 包括有 bug 的 `agent.c`
   - 所有其他 libpeer 源文件的编译版本

### 为什么混合链接失败

**链接器行为**:
```
链接顺序: esp_peer.a -> libpeer.a -> libpeer_default.a

当链接器遇到符号时:
1. 在 libpeer.a 中找到 agent_gather_candidate (修复版)
2. 在 libpeer_default.a 中又找到 agent_gather_candidate (bug 版)
3. 报错: multiple definition
```

**--whole-archive 的作用**:
- 强制包含库中的所有符号（即使未被引用）
- 导致更多的符号冲突

### esp_peer_ops_t 结构分析

需要实现的函数（如果要自己写 glue code）:
```c
typedef struct {
    int (*open)(esp_peer_cfg_t* cfg, esp_peer_handle_t* peer);
    int (*new_connection)(esp_peer_handle_t peer);
    int (*update_ice_info)(esp_peer_handle_t peer, esp_peer_role_t role,
                          esp_peer_ice_server_cfg_t* server, int server_num);
    int (*send_msg)(esp_peer_handle_t peer, esp_peer_msg_t* msg);
    int (*send_video)(esp_peer_handle_t peer, esp_peer_video_frame_t* frame);
    int (*send_audio)(esp_peer_handle_t peer, esp_peer_audio_frame_t* frame);
    int (*send_data)(esp_peer_handle_t peer, esp_peer_data_frame_t* frame);
    int (*create_data_channel)(esp_peer_handle_t peer, esp_peer_data_channel_cfg_t *ch_cfg);
    int (*close_data_channel)(esp_peer_handle_t peer, const char *label);
    int (*main_loop)(esp_peer_handle_t peer);
    int (*disconnect)(esp_peer_handle_t peer);
    void (*query)(esp_peer_handle_t peer);
    int (*close)(esp_peer_handle_t peer);
} esp_peer_ops_t;
```

每个函数都需要:
1. 参数转换（esp_peer 类型 -> libpeer 类型）
2. 调用对应的 libpeer 函数
3. 结果转换（libpeer 类型 -> esp_peer 类型）
4. 错误处理

**工作量估计**: 至少需要 500-1000 行代码

---

## 下一步建议

### 方案 B: 应用层修复（推荐）

**优点**:
- 简单直接，不涉及底层库修改
- 不需要重新编译 libpeer
- 可以快速验证效果

**实施步骤**:
1. 在应用代码中，在调用 WebRTC 连接前手动添加 host candidate
2. 或者修改 SDP，确保包含 host candidate

**代码位置**: `solutions/peer_demo/main/webrtc.c`

### 方案 C: 联系 Espressif

向 Espressif 提交 issue 或 PR:
1. 报告 libpeer 的 bug
2. 请求提供 glue code 源码
3. 或请求更新预编译库（包含修复）

**GitHub 仓库**: https://github.com/espressif/esp-webrtc-solution

### 方案 D: 完整实现 Glue Code（不推荐）

如果必须使用方案 A，需要:
1. 创建 `components/esp_peer/src/esp_peer_default_impl.c`
2. 实现所有 14 个 `esp_peer_ops_t` 函数
3. 实现 `esp_peer_get_default_impl()` 返回该结构

**预计工作量**: 2-3 天

---

## 参考资料

### RFC 文档
- RFC 5245: Interactive Connectivity Establishment (ICE)
  - Section 4.1.1: 必须收集 host candidates

### 源码仓库
- libpeer: https://github.com/sepfy/libpeer
- esp-webrtc-solution: https://github.com/espressif/esp-webrtc-solution

### 相关文件路径
- Bug 位置: `components/libpeer/src/agent.c:260-323`
- 调用位置: `solutions/peer_demo/main/webrtc.c:191`
- 头文件: `components/esp_peer/include/esp_peer_default.h:74`

---

## 附录: 完整错误日志

### 链接错误（尝试 1）
```
/Users/jiangzhongyang/.espressif/tools/xtensa-esp-elf/esp-14.2.0_20241119/xtensa-esp-elf/bin/../lib/gcc/xtensa-esp-elf/14.2.0/../../../../xtensa-esp-elf/bin/ld:
esp-idf/main/libmain.a(webrtc.c.obj):(.literal.signaling_ice_info_handler+0x28):
undefined reference to `esp_peer_get_default_impl'

/Users/jiangzhongyang/work/esp32/esp-webrtc-solution/solutions/peer_demo/main/webrtc.c:171:
(.text.signaling_ice_info_handler+0x7d):
undefined reference to `esp_peer_get_default_impl'

collect2: error: ld returned 1 exit status
```

### 链接错误（尝试 2 & 3）
```
[1537/1539] Linking CXX executable peer_demo.elf
FAILED: [code=1] peer_demo.elf
collect2: error: ld returned 1 exit status
```

（具体的多重定义错误信息被截断，但本质是所有 libpeer 函数都有两份定义）

---

**文档创建时间**: 2026-04-16
**ESP-IDF 版本**: 5.4
**开发板**: ESP32-S3-Korvo-2 V3.1
**问题状态**: ❌ 未解决（方案 A 遇到技术障碍）
