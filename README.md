# Radial Controller

<img src="https://img.shields.io/badge/-Arduino-00979D?style=flat&logo=Arduino&logoColor=white" />  <img src="https://img.shields.io/badge/Licence-MIT-green.svg?logo=github" />

## 项目概述

Radial Controller 是一款基于 CH5xx 系列芯片开发的 USB 径向控制器（旋转编码器），通过标准 USB HID 协议与计算机通信，可作为系统级径向控制器使用，支持多种操作系统和应用程序。

该设备采用模块化设计，包含硬件驱动、USB 通信和灯效控制等功能，支持通过 Web 界面进行参数配置，具有良好的扩展性和易用性。

### 主要功能特性

- **USB HID 径向控制器**：通过标准 HID 协议与计算机通信，支持 Windows、macOS、Linux 等主流操作系统
- **EC11 编码器支持**：支持旋转检测和按钮功能，可配置旋转灵敏度
- **WS2812 灯效控制**：内置 WS2812 灯带驱动，支持多种灯效模式和亮度调节
- **Web 配置工具**：通过浏览器进行设备参数配置，无需安装额外软件
- **EEPROM 配置存储**：非易失性存储用户配置，断电后不会丢失
- **CDC 串口通信**：支持通过串口进行命令控制和调试

### 使用方法

Radial Controller 作为标准 USB HID 设备，可以在系统级别使用，支持多种操作系统：

- **Windows**：支持 Windows 10/11，可在系统设置中配置径向控制器功能
- **macOS**：支持 macOS 10.15+，可在系统偏好设置中配置
- **Linux**：支持主流 Linux 发行版，需要安装相应驱动

#### 基本操作

- **旋转**：顺时针/逆时针旋转编码器，发送相应的角度值
- **按钮**：按下编码器按钮，发送按钮按下/释放事件
- **灯效**：旋转时灯效会发生变化，可通过配置工具调整灯效参数

## 软件依赖

### 核心库

该项目使用了第三方库 **ch55xduino**，这是一个为 CH55X 系列 USB MCU 提供的 Arduino 兼容编程 API。

- **项目地址**：[https://github.com/DeqingSun/ch55xduino](https://github.com/DeqingSun/ch55xduino)

## 编译上传固件

### 使用 Arduino IDE

1. **安装 ch55xduino 库**：

    - 打开 Arduino IDE
    - 进入`文件 > 首选项`，在 "附加开发板管理器网址" 中添加：
        ```
        https://raw.githubusercontent.com/DeqingSun/ch55xduino/ch55xduino/package_ch55xduino_mcs51_index.json
        ```
    - 进入`工具 > 开发板 > 开发板管理器`
    - 搜索 "ch55xduino" 并安装

2. **配置开发环境**：

    - 选择开发板：`工具 > 开发板 > CH55x Boards > CH552 Board`
    - 设置 USB RAM：`工具 > USB Settings > USER CODE w/266 USB Ram`
    - 选择上传方法：`工具 > 上传方式 > USB`

3. **进入 Bootloader 模式**：

    - 按住开发板上的 BOOT 按钮不放
    - 将开发板通过 USB 连接到电脑
    - 释放 BOOT 按钮，此时开发板进入 Bootloader 模式

    > 注意：除首次上传外，都需要手动进入 Bootloader 模式

4. **编译与上传**：

    - 点击 Arduino IDE 中的 "验证" 按钮编译代码
    - 点击 "上传" 按钮将代码烧录到芯片

### 使用 WCH 官方工具

1. **下载 WCH 官方工具**：

    - 访问 WCH 官方网站：[http://www.wch.cn/downloads/WCHISPTool_Setup_exe.html](http://www.wch.cn/downloads/WCHISPTool_Setup_exe.html)
    - 下载并安装 WCHISPTool (Studio) 工具

2. **准备固件文件**：

    - 下载项目提供的[预编译固件文件](https://github.com/walklinewang/Radial-Controller/releases)

3. **进入 Bootloader 模式**：

    - 按住开发板上的 BOOT 按钮不放
    - 将开发板通过 USB 连接到电脑
    - 释放 BOOT 按钮，此时开发板进入 Bootloader 模式

    > 注意：除首次上传外，都需要手动进入 Bootloader 模式

4. **使用 WCHISPTool (Studio) 烧录固件**：

    - 打开 WCHISPTool (Studio) 工具
    - 在 "设备类型" 中选择 "CH552"
    - 点击 "搜索设备" 按钮，工具将自动识别 Bootloader 模式下的 CH552 芯片
    - 在 "目标程序文件" 中选择编译好的 `.hex` 固件文件
    - 勾选 "下载完成后运行目标程序" 选项
    - 点击 "下载" 按钮开始烧录固件
    - 烧录完成后，设备将自动运行新烧录的固件

## Web Config 工具

Radial Controller 提供了基于 Web 的配置工具，可通过浏览器进行设备参数配置。

### 配置步骤

1. 连接设备：将 Radial Controller 连接到计算机
2. 打开配置工具：在浏览器中访问 [https://webconfig.walkline.wang](https://webconfig.walkline.wang)
3. 连接设备：点击 "连接设备" 按钮，选择 Radial Controller 设备
4. 加载配置：设备连接成功后，将自动加载当前配置
5. 调整参数：
    - **灯珠数量**：调整 WS2812 灯珠数量（1-10）
    - **亮度等级**：调整灯带亮度（1-5 级）
    - **颜色顺序**：选择灯珠颜色顺序（GRB/RGB）
    - **灯效模式**：选择灯效模式（默认模式）
    - **灯效循环周期**：调整灯效变化速度（20-500ms）
    - **旋转灵敏度**：调整编码器旋转灵敏度
    - **顺时针旋转值**：设置顺时针旋转时发送的角度值
    - **逆时针旋转值**：设置逆时针旋转时发送的角度值
6. 保存配置：点击 "保存设置" 按钮，保存配置到设备

### 配置参数说明

| 参数 | 说明 | 范围 | 默认值 |
|:---:|------|:---:|:-----:|
| led_count | WS2812 灯珠数量 | 1-10 | 4 |
| brightness | 亮度等级 | 1-5 | 4 |
| color_order | 颜色顺序 | GRB/RGB | GRB |
| effect_mode | 灯效模式 | 0 | 0 |
| effect_tick | 灯效循环周期（ms） | 20-500 | 50 |
| step_per_teeth | 旋转灵敏度（每齿触发次数） | 1-2 | 2 |
| rotate_cw | 顺时针旋转角度（度） | -360~360 | 10 |
| rotate_ccw | 逆时针旋转角度（度） | -360~360 | -10 |

## 使用 USB VendorID 过滤设备

### 设备识别原理

USB 设备通过 VendorID（供应商 ID）和 ProductID（产品 ID）进行唯一识别。Radial Controller 使用了开源硬件通用的 VendorID `0x1209`。

### 固件中的 VendorID

在 `src/CdcRadial/USBconstant.c#L18` 中定义了设备的 VendorID：

```c
.VendorID = 0x1209,
```

这个定义会被包含在 USB 设备描述符中，当设备连接到计算机时，操作系统会读取这些信息来识别设备类型和制造商。

### Web 配置工具中的 VendorID

在 `tools/web_config/script.js#L29` 中，Web 配置工具使用了相同的 VendorID 来过滤目标设备：

```javascript
this.USB_VENDOR_ID = 0x1209;
```

当用户点击"连接设备"按钮时，Web 配置工具会使用这个 VendorID 过滤出所有匹配的 Radial Controller 设备。

## 参考项目

### Microsoft Radial Controller Design Guidelines

[Microsoft 径向控制器设计指南](https://learn.microsoft.com/en-us/windows-hardware/design/component-guidelines/radial-controller-designs) 提供了关于径向控制器的设计规范和最佳实践，包括：

- 硬件设计原则
- HID 报告描述符规范
- 与 Windows 系统集成的最佳实践

该指南为 Radial Controller 项目提供了标准化的 HID 报告格式和交互设计参考，确保设备能够与 Windows 系统无缝集成。

### CH552 旋钮控制器项目

[[星火计划] 丐 dial](https://oshwhub.com/morempty/CH552gyin-liang-xuan-niu) 是一个基于 CH552 芯片的旋钮控制器开源项目，提供了硬件电路设计参考，加速了项目的开发进程。

## 故障排除

### 1. 设备无法被识别

**问题**：设备连接到计算机后，无法被识别或显示为未知设备

**解决方案**：

1. 检查 USB 连接线是否完好
2. 尝试不同的 USB 端口
3. 重新烧录固件
4. 检查设备驱动是否正确安装（Windows 系统）

### 2. Web Config 工具无法连接设备

**问题**：Web Config 工具无法找到或连接设备

**解决方案**：

1. 确保使用支持 Web Serial API 的浏览器（Chrome、Edge、Opera）
2. 检查设备是否已正确连接
3. 清除浏览器缓存并重新加载页面
4. 检查浏览器是否有 Web Serial API 访问权限

### 3. 旋转编码器无响应

**问题**：旋转编码器时，计算机或灯效无响应

**解决方案**：

1. 检查编码器与芯片的连接是否正确
2. 检查编码器是否损坏
3. 调整旋转灵敏度配置
4. 重新烧录固件

### 4. 灯效异常

**问题**：LED 灯珠不亮或显示异常颜色

**解决方案**：

1. 检查灯珠数量配置是否正确
2. 检查颜色顺序配置是否正确
3. 检查灯珠与芯片的连接是否正确
4. 检查灯珠是否损坏

## 许可证

Radial Controller 项目采用 MIT 许可证开源，详见 [LICENSE](LICENSE) 文件。
