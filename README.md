# 🌈 WiFi LED 控制项目

基于ESP32的智能LED控制系统，支持WS2812B灯带控制，提供Web界面和蓝牙双重控制方式。

## ✨ 项目特色

- 🎨 **直观的Web控制界面** - 现代化的渐变按钮设计和HSV调色盘
- 📱 **多平台支持** - 支持ESP32和ESP32-C3开发板
- 🔗 **双重连接方式** - USB串口通信 + 蓝牙BLE连接
- 💾 **状态持久化** - 自动保存LED状态到NVS存储
- 🌊 **呼吸灯效果** - 可调节周期的平滑呼吸动画
- 🎯 **实时控制** - 颜色、亮度、模式实时调节

## 📁 项目结构

```
wifi-led/
├── 1.frimework/           # 固件代码
│   ├── src/
│   │   └── main.cpp       # 主程序源码
│   ├── include/           # 头文件目录
│   ├── lib/               # 库文件目录
│   ├── test/              # 测试文件
│   └── platformio.ini     # PlatformIO配置
├── 2.hardware/            # 硬件设计文件
│   └── Gerber_PCB1_1_2025-08-19.zip  # PCB制造文件
├── index.html             # 主控制界面
├── color-picker.html      # 独立调色盘组件
└── README.md              # 项目说明
```

## 🛠️ 硬件要求

### 支持的开发板
- **ESP32 DevKit V1** (默认配置)
- **ESP32-C3 DevKit M1** (需要特殊编译标志)

### 外围器件
- **WS2812B LED灯带** (默认7颗LED)
- **连接线材**
- **电源供应** (根据LED数量确定)

### 引脚配置
- ESP32: GPIO 13 (LED数据线)
- ESP32-C3: GPIO 4 (LED数据线)

## 🚀 快速开始

### 1. 环境准备
```bash
# 安装PlatformIO Core
pip install platformio

# 或使用VSCode扩展
# 安装 "PlatformIO IDE" 扩展
```

### 2. 编译上传
```bash
# 克隆项目
git clone <repository-url>
cd wifi-led/1.frimework

# ESP32编译上传
pio run -e esp32 --target upload

# ESP32-C3编译上传  
pio run -e c3 --target upload
```

### 3. Web界面使用
```bash
# 启动本地服务器
python -m http.server 8000

# 浏览器访问
http://localhost:8000/index.html
```

## 🎮 功能说明

### Web控制界面
- **连接管理**: USB串口/蓝牙BLE连接切换
- **LED控制**: 开关、亮度调节
- **呼吸灯**: 开关、周期调节
- **调色盘**: HSV颜色空间选择器
- **状态显示**: 实时连接状态指示

### 蓝牙命令协议
```
ON              # 开启LED
OFF             # 关闭LED
BREATHE ON      # 开启呼吸模式
BREATHE OFF     # 关闭呼吸模式
RGB,255,128,64  # 设置RGB颜色
BRIGHTNESS,128  # 设置亮度(0-255)
PERIOD,3000     # 设置呼吸周期(毫秒)
```

### 状态持久化
系统自动将以下状态保存到NVS:
- LED开关状态
- 呼吸模式状态  
- RGB颜色值
- 亮度设置
- 呼吸周期

## 🔧 开发配置

### PlatformIO依赖库
```ini
lib_deps =
    adafruit/Adafruit NeoPixel  # WS2812B控制
    h2zero/NimBLE-Arduino       # 蓝牙BLE通信
    OneButton                   # 按钮处理
    WIFI                        # WiFi功能
```

### 编译标志
```ini
# ESP32-C3专用标志
build_flags = 
    -DUSING_C3_BOARD
    -DARDUINO_USB_MODE=1
    -DARDUINO_USB_CDC_ON_BOOT=1
```

## 🎨 界面预览

Web控制界面采用现代化设计:
- 渐变色按钮和卡片布局
- 响应式设计适配移动端
- HSV调色盘支持拖拽选色
- 实时状态反馈

## 🤝 贡献指南

1. Fork 本项目
2. 创建特性分支 (`git checkout -b feature/AmazingFeature`)
3. 提交更改 (`git commit -m 'Add some AmazingFeature'`)
4. 推送到分支 (`git push origin feature/AmazingFeature`)
5. 开启 Pull Request

## 📄 许可证

本项目采用 MIT 许可证 - 查看 [LICENSE](LICENSE) 文件了解详情

## 🔗 相关链接

- [PlatformIO 文档](https://docs.platformio.org/)
- [ESP32 Arduino 核心](https://github.com/espressif/arduino-esp32)
- [Adafruit NeoPixel 库](https://github.com/adafruit/Adafruit_NeoPixel)

---

⭐ 如果这个项目对你有帮助，请给个星标支持！
