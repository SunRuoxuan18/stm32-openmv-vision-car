# STM32 + OpenMV 视觉小车原型

这是一个独立的 OpenMV 视觉控制原型仓库。OpenMV 负责采集图像、识别黑色引导线并通过 UART 发送左右轮速度；STM32 负责接收带校验的数据帧、执行超时保护，并把速度命令交给电机驱动层。

## 仓库结构

```text
openmv/                    OpenMV MicroPython 视觉脚本
protocol/                  UART 二进制协议和接线说明
firmware/Core/Inc/         STM32 端头文件
firmware/Core/Src/         STM32 端协议解析实现
tools/                     不依赖第三方库的协议回归测试
```

## 当前实现

- OpenMV 黑线区域识别、横向误差计算和比例转向。
- 115200-8-N-1 UART 二进制帧，包含帧头、序号、左右轮速度、置信度和 XOR 校验。
- STM32 端逐字节解析，丢帧、坏校验和未知命令会被忽略。
- 250 ms 没有收到有效视觉帧时自动停车，避免 OpenMV 断线后电机保持上一次速度。
- 视觉速度限制在 `-100..100`，输出前进行限幅。
- Python 协议测试覆盖正常帧、坏校验和半帧输入。

## 快速开始

1. 用 OpenMV IDE 将 `openmv/openmv_line_follow.py` 下载到摄像头。
2. 按 `protocol/PROTOCOL.md` 连接 OpenMV UART TX/RX 与 STM32 UART。
3. 在 CubeMX 中启用一个 UART（115200，8 数据位，无校验，1 停止位），把接收字节交给 `VisionLink_ProcessByte()`。
4. 在主循环中周期调用 `VisionLink_Poll()`，并在 `VisionLink_OnCommand()` 中接入你自己的电机控制函数。
5. 先架空车轮进行测试；确认方向和电源安全后再落地运行。

协议回归测试（本机有 Python 时）：

```text
python tools/test_protocol.py
```

Node.js 环境也可以直接运行等价测试：

```text
node tools/test_protocol.js
```

当前执行环境没有安装 Python，因此本次只用等价的 Node.js 断言完成了帧编码、解码和坏校验检查；OpenMV 脚本仍需在 OpenMV IDE 的 MicroPython 环境中运行。

## CubeMX/Keil 接入边界

`firmware/Core/Src/vision_link.c` 只依赖 STM32 HAL 的 UART 和 tick 接口，不包含特定开发板的引脚初始化，也不替换用户已有的电机驱动代码。CubeMX 生成的 UART 初始化、Keil 工程文件和电机驱动应由实际硬件工程单独维护。

## 状态说明

这是独立的 AI 生成原型代码，已完成静态协议测试；尚未在实际 OpenMV、STM32、驱动板和整车上完成现场联调。因此不能把“实车循迹已完成”当作当前仓库的验证结论。

