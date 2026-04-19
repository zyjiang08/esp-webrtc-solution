# macOS 上安装 ESP-IDF 及工具链

本文档介绍如何在 macOS 系统上安装 ESP-IDF（Espressif IoT Development Framework）及其工具链。

## 系统要求

- macOS 10.15 (Catalina) 或更高版本
- 至少 10 GB 可用磁盘空间
- 命令行工具基础知识

## 安装步骤

### 1. 安装前置依赖

#### 1.1 安装 Homebrew

如果尚未安装 Homebrew，请在终端中运行：

```bash
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
```

#### 1.2 安装必需的工具

```bash
brew install cmake ninja dfu-util
```

工具说明：
- **cmake**: 构建系统生成器
- **ninja**: 快速构建工具
- **dfu-util**: 设备固件升级工具

#### 1.3 安装 Python 3

ESP-IDF 需要 Python 3.8 或更高版本：

```bash
# 检查 Python 版本
python3 --version

# 如果版本过低或未安装，使用 Homebrew 安装
brew install python3
```

### 2. 下载 ESP-IDF

#### 2.1 克隆 ESP-IDF 仓库

选择一个合适的目录（例如 `~/esp`）来存放 ESP-IDF：

```bash
mkdir -p ~/esp
cd ~/esp
git clone --recursive https://github.com/espressif/esp-idf.git
```

#### 2.2 切换到稳定版本（推荐）

```bash
cd ~/esp/esp-idf
git checkout v5.4  # 或其他稳定版本
git submodule update --init --recursive
```

**如果遇到子模块更新错误**（如 `untracked working tree files would be overwritten`），执行以下命令清理：

```bash
# 清理所有子模块中的未跟踪文件
git submodule foreach --recursive git clean -xfd

# 重置 openthread 子模块（如果仍有问题）
cd components/openthread/openthread
git reset --hard
git clean -xfd
cd ~/esp/esp-idf

# 删除有问题的嵌套子模块目录（如果需要）
rm -rf components/openthread/openthread/third_party/mbedtls/repo

# 重新更新子模块
git submodule update --init --recursive
```

查看可用版本：
```bash
git tag | grep v5
```

### 3. 安装 ESP-IDF 工具链

#### 3.1 运行安装脚本

根据你的目标芯片安装相应的工具链：

```bash
cd ~/esp/esp-idf

# 安装所有支持的芯片工具链
./install.sh all

# 或仅安装特定芯片的工具链（推荐，节省空间）
./install.sh esp32      # ESP32
./install.sh esp32s3    # ESP32-S3
./install.sh esp32c3    # ESP32-C3
./install.sh esp32c6    # ESP32-C6

# 安装多个芯片
./install.sh esp32,esp32s3,esp32c3
```

安装成功后会显示：
```
All done! You can now run:
  . ./export.sh
```

#### 3.2 处理常见安装问题

**问题 1：SSL 证书错误**

如果遇到 SSL 证书错误：
```
<urlopen error [SSL: CERTIFICATE_VERIFY_FAILED] certificate verify failed>
```

解决方法：
1. 打开 **访达 (Finder)**
2. 进入 **应用程序 → Python 3.x** 文件夹
3. 双击运行 **Install Certificates.command**

或在终端中运行：
```bash
/Applications/Python\ 3.*/Install\ Certificates.command
```

**问题 2：代理连接错误**

如果遇到代理错误：
```
ProxyError('Cannot connect to proxy.', TimeoutError)
ERROR: Could not install packages due to an OSError
```

解决方法：
```bash
# 1. 清除代理环境变量
unset http_proxy https_proxy HTTP_PROXY HTTPS_PROXY all_proxy ALL_PROXY

# 2. 配置使用国内镜像（推荐）
pip config set global.index-url https://pypi.tuna.tsinghua.edu.cn/simple

# 3. 重新运行安装
cd ~/esp/esp-idf
./install.sh esp32s3
```

其他可用的国内镜像：
- 阿里云：`https://mirrors.aliyun.com/pypi/simple/`
- 中科大：`https://pypi.mirrors.ustc.edu.cn/simple/`
- 华为云：`https://mirrors.huaweicloud.com/repository/pypi/simple`

### 4. 设置环境变量

#### 4.1 临时设置（每次打开新终端都需要运行）

```bash
cd ~/esp/esp-idf
source ./export.sh
```

或使用别名：
```bash
alias get_idf='. ~/esp/esp-idf/export.sh'
```

然后每次只需运行：
```bash
get_idf
```

#### 4.2 永久设置（可选）

**方法一：添加到 shell 配置文件**

对于 **zsh**（macOS 默认）：
```bash
echo 'alias get_idf=". ~/esp/esp-idf/export.sh"' >> ~/.zshrc
```

对于 **bash**：
```bash
echo 'alias get_idf=". ~/esp/esp-idf/export.sh"' >> ~/.bash_profile
```

**方法二：自动加载（不推荐）**

如果希望每次打开终端自动加载 ESP-IDF 环境：
```bash
# zsh
echo 'source ~/esp/esp-idf/export.sh' >> ~/.zshrc

# bash
echo 'source ~/esp/esp-idf/export.sh' >> ~/.bash_profile
```

> **注意**：自动加载会使终端启动变慢，且可能与其他开发环境冲突。

### 5. 验证安装

#### 5.1 检查环境变量

```bash
# 设置环境变量
source ~/esp/esp-idf/export.sh

# 验证 IDF_PATH
echo $IDF_PATH
# 应输出: /Users/你的用户名/esp/esp-idf

# 检查 idf.py 命令
which idf.py
# 应输出: /Users/你的用户名/esp/esp-idf/tools/idf.py
```

#### 5.2 测试编译示例项目

```bash
# 复制示例项目
cd ~/esp
cp -r $IDF_PATH/examples/get-started/hello_world .
cd hello_world

# 设置目标芯片
idf.py set-target esp32s3  # 或 esp32, esp32c3 等

# 编译项目
idf.py build
```

如果编译成功，说明安装完成！

### 6. 连接开发板

#### 6.1 安装 USB 驱动（如需要）

大多数 ESP32 开发板使用以下芯片之一：
- **CP210x**: [下载驱动](https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers)
- **CH340**: [下载驱动](https://github.com/adrianmihalko/ch340g-ch34g-ch34x-mac-os-x-driver)
- **FTDI**: macOS 通常自带驱动

#### 6.2 查找串口设备

```bash
ls /dev/cu.*
```

常见输出：
- `/dev/cu.usbserial-*`
- `/dev/cu.SLAB_USBtoUART`
- `/dev/cu.wchusbserial*`

#### 6.3 烧录固件

```bash
# 烧录并监控
idf.py -p /dev/cu.usbserial-XXX flash monitor

# 退出监控：Ctrl + ]
```

### 7. 常见问题

#### 问题 1：权限被拒绝

```bash
# 添加用户到 dialout 组（Linux）
# macOS 通常不需要此步骤

# 如果遇到权限问题，尝试：
sudo chmod 666 /dev/cu.usbserial-XXX
```

#### 问题 2：找不到串口设备

- 检查 USB 线缆是否支持数据传输（不是仅充电线）
- 尝试不同的 USB 端口
- 重新插拔开发板
- 检查驱动是否正确安装

#### 问题 3：Python 版本冲突

```bash
# 使用 pyenv 管理多个 Python 版本
brew install pyenv

# 安装 Python 3.11
pyenv install 3.11.0
pyenv global 3.11.0
```

#### 问题 4：编译速度慢

```bash
# 使用 ccache 加速编译
brew install ccache

# 在项目中启用
idf.py menuconfig
# 导航到: Compiler options -> Enable ccache
```

## ESP-ADF 安装（音频开发）

如果需要开发音频应用（如本仓库的 RTC AIGC Demo），还需要安装 ESP-ADF：

```bash
cd ~/esp
git clone https://github.com/espressif/esp-adf.git
cd esp-adf

# 切换到特定版本（如需要）
git checkout eca11f20e56f9b5321b714da4305e123672d92a9

# 同步子模块
git submodule update --init --recursive

# 安装工具链（ESP-ADF 包含 ESP-IDF）
./install.sh esp32s3

# 设置环境变量
source ./export.sh
```

## 更新 ESP-IDF

```bash
cd ~/esp/esp-idf
git pull
git submodule update --init --recursive
./install.sh all  # 或指定芯片
```

## 卸载

```bash
# 删除 ESP-IDF 目录
rm -rf ~/esp/esp-idf

# 删除工具链（可选）
rm -rf ~/.espressif

# 从 shell 配置文件中移除相关配置
# 编辑 ~/.zshrc 或 ~/.bash_profile
```

## 参考资源

- [ESP-IDF 官方文档](https://docs.espressif.com/projects/esp-idf/zh_CN/latest/esp32/get-started/index.html)
- [ESP-IDF GitHub](https://github.com/espressif/esp-idf)
- [ESP32 论坛](https://esp32.com/)
- [Espressif 官网](https://www.espressif.com/)

## 下一步

安装完成后，你可以：
1. 浏览 ESP-IDF 示例：`$IDF_PATH/examples/`
2. 阅读 [ESP-IDF 编程指南](https://docs.espressif.com/projects/esp-idf/zh_CN/latest/esp32/api-reference/index.html)
3. 开始开发你的第一个 ESP32 项目
4. 如果要运行 RTC AIGC Demo，请参考 `rtc-aigc-embedded-demo/docs/QUICK_START_ESP.md`
