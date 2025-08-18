# WiFi LED 控制项目

这是一个基于 ESP8266 的 WiFi LED 控制项目。通过手机或电脑可以远程控制 LED 的开关和亮度。

## 功能特点

- WiFi 远程控制
- LED 开关控制
- LED 亮度调节
- Web 界面操作
- 支持多设备接入

## 硬件需求

- ESP8266 开发板
- LED 灯
- 电阻
- 面包板
- 杜邦线

## 软件需求

- Arduino IDE
- ESP8266 开发板支持包
- 相关库文件

## 接线说明

1. LED 正极 -> ESP8266 D1 引脚
2. LED 负极 -> 电阻 -> GND

## 使用说明

1. 下载代码并烧录到 ESP8266
2. 连接设备到 WiFi
3. 访问设备 IP 地址
4. 通过 Web 界面控制 LED

## 注意事项

- 请确保供电电压合适
- 注意 LED 极性连接
- 建议使用 5V-12V 电源供电

## 贡献

欢迎提交 Issue 和 Pull Request

## 许可证

MIT License
