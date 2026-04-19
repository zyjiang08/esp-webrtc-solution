# ESP-IDF 安装时的代理错误修复

## 问题描述

在执行 `./install.sh esp32s3` 时遇到代理连接错误：

```
ProxyError('Cannot connect to proxy.', TimeoutError('_ssl.c:1011: The handshake operation timed out'))
ERROR: Could not install packages due to an OSError: HTTPSConnectionPool...
```

## 问题原因

pip 正在尝试使用代理服务器连接 PyPI，但代理服务器无法访问或配置不正确。这通常是由于：
1. 系统代理设置残留
2. pip 配置了代理但代理已失效
3. 网络环境变化导致代理不可用

## 解决方案

### 方案一：临时禁用代理（推荐）

在当前终端会话中禁用代理：

```bash
# 取消所有代理环境变量
unset http_proxy
unset https_proxy
unset HTTP_PROXY
unset HTTPS_PROXY
unset all_proxy
unset ALL_PROXY

# 验证代理已清除
env | grep -i proxy

# 重新运行安装
cd ~/esp/esp-idf
./install.sh esp32s3
```

### 方案二：使用国内镜像源

如果网络访问 PyPI 较慢，可以使用国内镜像：

```bash
# 方法 1: 临时使用清华镜像
cd ~/esp/esp-idf
pip config set global.index-url https://pypi.tuna.tsinghua.edu.cn/simple

# 重新运行安装
./install.sh esp32s3

# 安装完成后恢复默认（可选）
pip config unset global.index-url
```

```bash
# 方法 2: 使用阿里云镜像
pip config set global.index-url https://mirrors.aliyun.com/pypi/simple/
```

```bash
# 方法 3: 使用中科大镜像
pip config set global.index-url https://pypi.mirrors.ustc.edu.cn/simple/
```

### 方案三：手动安装 Python 依赖

如果上述方法仍有问题，可以手动安装：

```bash
# 1. 取消代理
unset http_proxy https_proxy HTTP_PROXY HTTPS_PROXY

# 2. 激活 ESP-IDF Python 环境
cd ~/esp/esp-idf
source export.sh

# 3. 手动安装依赖（使用国内镜像）
python -m pip install --upgrade pip
pip install -r tools/requirements/requirements.core.txt \
    -i https://pypi.tuna.tsinghua.edu.cn/simple \
    --trusted-host pypi.tuna.tsinghua.edu.cn

# 4. 验证安装
idf.py --version
```

### 方案四：配置 pip 忽略代理

创建或编辑 pip 配置文件：

```bash
# 创建 pip 配置目录
mkdir -p ~/.pip

# 创建配置文件
cat > ~/.pip/pip.conf << 'EOF'
[global]
trusted-host = pypi.python.org
               pypi.org
               files.pythonhosted.org
no-proxy = *
EOF

# 重新运行安装
cd ~/esp/esp-idf
./install.sh esp32s3
```

### 方案五：检查并修改系统代理设置

如果是 macOS 系统代理导致的问题：

```bash
# 1. 打开系统偏好设置
# 系统偏好设置 -> 网络 -> 高级 -> 代理

# 2. 或使用命令行检查
networksetup -getwebproxy Wi-Fi
networksetup -getsecurewebproxy Wi-Fi

# 3. 临时禁用系统代理（需要管理员权限）
sudo networksetup -setwebproxystate Wi-Fi off
sudo networksetup -setsecurewebproxystate Wi-Fi off

# 4. 安装完成后重新启用（如需要）
sudo networksetup -setwebproxystate Wi-Fi on
sudo networksetup -setsecurewebproxystate Wi-Fi on
```

## 推荐的完整安装流程

```bash
# 1. 清除代理环境变量
unset http_proxy https_proxy HTTP_PROXY HTTPS_PROXY all_proxy ALL_PROXY

# 2. 配置使用国内镜像（可选但推荐）
pip config set global.index-url https://pypi.tuna.tsinghua.edu.cn/simple

# 3. 进入 ESP-IDF 目录
cd ~/esp/esp-idf

# 4. 运行安装脚本
./install.sh esp32s3

# 5. 设置环境变量
source ./export.sh

# 6. 验证安装
idf.py --version
```

## 验证修复

安装成功后应该看到：

```
All done! You can now run:
  . ./export.sh
```

然后验证工具链：

```bash
source ./export.sh
idf.py --version
xtensa-esp32s3-elf-gcc --version
```

## 常见国内 PyPI 镜像源

| 镜像源 | URL | 说明 |
|--------|-----|------|
| 清华大学 | https://pypi.tuna.tsinghua.edu.cn/simple | 推荐，速度快 |
| 阿里云 | https://mirrors.aliyun.com/pypi/simple/ | 稳定性好 |
| 中科大 | https://pypi.mirrors.ustc.edu.cn/simple/ | 教育网友好 |
| 豆瓣 | https://pypi.douban.com/simple/ | 老牌镜像 |
| 华为云 | https://mirrors.huaweicloud.com/repository/pypi/simple | 企业级 |

## 永久配置 pip 镜像（可选）

如果经常遇到网络问题，可以永久配置：

```bash
# 配置清华镜像
pip config set global.index-url https://pypi.tuna.tsinghua.edu.cn/simple
pip config set install.trusted-host pypi.tuna.tsinghua.edu.cn

# 查看配置
pip config list

# 配置文件位置
# macOS/Linux: ~/.pip/pip.conf
# Windows: %APPDATA%\pip\pip.ini
```

## 故障排查

如果问题仍然存在：

```bash
# 1. 检查网络连接
ping pypi.org
ping pypi.tuna.tsinghua.edu.cn

# 2. 检查 DNS
nslookup pypi.org

# 3. 测试 pip 连接
pip install --dry-run requests

# 4. 查看详细错误信息
pip install -v requests

# 5. 清除 pip 缓存
pip cache purge
```

## 参考资源

- [pip 官方文档 - 代理配置](https://pip.pypa.io/en/stable/user_guide/#using-a-proxy-server)
- [清华大学 PyPI 镜像使用帮助](https://mirrors.tuna.tsinghua.edu.cn/help/pypi/)
- [ESP-IDF 安装故障排查](https://docs.espressif.com/projects/esp-idf/zh_CN/latest/esp32/get-started/index.html#get-started-get-prerequisites)

---

**更新日期**: 2026-04-12
**状态**: ✅ 已提供多种解决方案
