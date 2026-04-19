# ESP32 开发环境路径配置

## 📁 目录结构

所有 ESP32 相关的工具和项目都位于：`/Users/jiangzhongyang/esp`

```
/Users/jiangzhongyang/esp/
├── esp-idf/                    # ESP-IDF 框架
│   ├── components/             # ESP-IDF 组件
│   ├── examples/               # 官方示例
│   ├── tools/                  # 构建工具
│   ├── export.sh               # 环境变量设置脚本
│   └── install.sh              # 安装脚本
│
├── esp-adf/                    # ESP-ADF 音频框架（如果安装）
│   ├── components/
│   ├── examples/
│   └── export.sh
│
└── [你的项目目录]/             # 用户项目
    ├── hello_world/
    ├── my_project/
    └── ...
```

## 🔧 环境变量设置

### 每次新终端都需要运行：

```bash
# 进入 ESP-IDF 目录
cd /Users/jiangzhongyang/esp/esp-idf

# 设置环境变量
source ./export.sh
```

### 或使用绝对路径：

```bash
source /Users/jiangzhongyang/esp/esp-idf/export.sh
```

### 添加别名（推荐）

编辑 `~/.zshrc`：

```bash
# 添加以下行
alias get_idf='source /Users/jiangzhongyang/esp/esp-idf/export.sh'

# 如果安装了 ESP-ADF
alias get_adf='source /Users/jiangzhongyang/esp/esp-adf/export.sh'
```

然后重新加载配置：
```bash
source ~/.zshrc
```

以后只需运行：
```bash
get_idf
```

## 📝 常用路径

### ESP-IDF 相关

```bash
# ESP-IDF 主目录
ESP_IDF_PATH="/Users/jiangzhongyang/esp/esp-idf"

# 示例代码
EXAMPLES_PATH="/Users/jiangzhongyang/esp/esp-idf/examples"

# 工具目录
TOOLS_PATH="/Users/jiangzhongyang/esp/esp-idf/tools"

# 组件目录
COMPONENTS_PATH="/Users/jiangzhongyang/esp/esp-idf/components"
```

### 工具链和 Python 环境

```bash
# 工具链安装位置
ESPRESSIF_PATH="$HOME/.espressif"

# Python 虚拟环境
PYTHON_ENV="$HOME/.espressif/python_env/idf5.4_py3.13_env"

# 工具链
TOOLCHAIN_PATH="$HOME/.espressif/tools"
```

## 🚀 快速命令

### 创建新项目

```bash
# 方法 1：从示例复制
cd /Users/jiangzhongyang/esp
cp -r esp-idf/examples/get-started/hello_world my_project
cd my_project

# 方法 2：使用 idf.py 创建
cd /Users/jiangzhongyang/esp
idf.py create-project my_project
cd my_project
```

### 编译和烧录

```bash
# 1. 设置环境
source /Users/jiangzhongyang/esp/esp-idf/export.sh

# 2. 进入项目目录
cd /Users/jiangzhongyang/esp/my_project

# 3. 设置目标芯片
idf.py set-target esp32s3

# 4. 编译
idf.py build

# 5. 烧录（需要先找到串口）
idf.py -p /dev/cu.usbserial-XXX flash monitor
```

## 📋 环境变量说明

设置环境后，会自动配置以下变量：

```bash
# 查看 ESP-IDF 路径
echo $IDF_PATH
# 输出: /Users/jiangzhongyang/esp/esp-idf

# 查看工具链路径
echo $IDF_TOOLS_PATH
# 输出: /Users/jiangzhongyang/.espressif

# 查看 Python 路径
which python
# 输出: /Users/jiangzhongyang/.espressif/python_env/idf5.4_py3.13_env/bin/python

# 查看 idf.py 路径
which idf.py
# 输出: /Users/jiangzhongyang/esp/esp-idf/tools/idf.py
```

## 🔄 更新 ESP-IDF

```bash
cd /Users/jiangzhongyang/esp/esp-idf
git pull
git submodule update --init --recursive
./install.sh esp32s3
```

## 📦 项目推荐结构

```bash
/Users/jiangzhongyang/esp/
├── esp-idf/                    # ESP-IDF 框架（不要修改）
├── esp-adf/                    # ESP-ADF 框架（不要修改）
│
├── projects/                   # 你的项目目录
│   ├── project1/
│   ├── project2/
│   └── ...
│
└── components/                 # 自定义组件（可选）
    ├── my_component1/
    └── my_component2/
```

## 🛠️ 工具链位置

```bash
# xtensa-esp32s3 工具链
~/.espressif/tools/xtensa-esp-elf-gdb/
~/.espressif/tools/xtensa-esp32s3-elf/

# OpenOCD
~/.espressif/tools/openocd-esp32/

# Python 包
~/.espressif/python_env/idf5.4_py3.13_env/
```

## 💡 使用技巧

### 1. 快速切换项目

```bash
# 创建项目切换函数
function cdp() {
    cd /Users/jiangzhongyang/esp/$1
    source /Users/jiangzhongyang/esp/esp-idf/export.sh
}

# 使用
cdp my_project
```

### 2. 一键设置环境

创建 `~/esp_env.sh`：

```bash
#!/bin/bash
export ESP_BASE="/Users/jiangzhongyang/esp"
source $ESP_BASE/esp-idf/export.sh
cd $ESP_BASE
echo "ESP-IDF environment ready!"
echo "ESP-IDF path: $IDF_PATH"
```

使用：
```bash
source ~/esp_env.sh
```

### 3. 项目模板

在 `/Users/jiangzhongyang/esp/` 创建项目模板：

```bash
mkdir -p /Users/jiangzhongyang/esp/templates
cp -r esp-idf/examples/get-started/hello_world templates/basic_template
```

创建新项目：
```bash
cp -r /Users/jiangzhongyang/esp/templates/basic_template /Users/jiangzhongyang/esp/my_new_project
```

## 📚 相关文档路径

```bash
# 本地文档
/Users/jiangzhongyang/work/esp32/docs/

# ESP-IDF 文档
/Users/jiangzhongyang/esp/esp-idf/docs/

# 示例代码
/Users/jiangzhongyang/esp/esp-idf/examples/
```

## 🔍 查找文件

```bash
# 查找 ESP-IDF 中的头文件
find /Users/jiangzhongyang/esp/esp-idf/components -name "*.h" | grep wifi

# 查找示例
ls /Users/jiangzhongyang/esp/esp-idf/examples/

# 查找组件
ls /Users/jiangzhongyang/esp/esp-idf/components/
```

## ⚙️ 配置文件位置

```bash
# pip 配置
~/.config/pip/pip.conf

# Git 配置
~/.gitconfig

# Shell 配置
~/.zshrc  # 或 ~/.bash_profile

# ESP-IDF 约束文件
~/.espressif/espidf.constraints.v5.4.txt
```

---

**提示**：将此文件保存为书签，方便快速查找路径！
