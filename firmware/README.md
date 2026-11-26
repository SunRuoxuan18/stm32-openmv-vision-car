# STM32 接入说明

## CubeMX 配置

在你自己的 STM32F4 CubeMX 工程中启用一个 UART：115200 baud、8 data bits、无校验、1 stop bit。接收可以先采用轮询方式验证，也可以改为中断或 DMA。

## 轮询示例

```c
#include "vision_link.h"

extern UART_HandleTypeDef huart2;
static uint8_t rx_byte;

void app_init(void)
{
    VisionLink_Init(&huart2);
}

void app_loop(void)
{
    if (HAL_UART_Receive(&huart2, &rx_byte, 1U, 1U) == HAL_OK)
    {
        VisionLink_ProcessByte(rx_byte);
    }
    VisionLink_Poll();
}
```

## 电机适配

在应用工程中覆盖弱函数，把收到的左右速度转到实际电机接口。示例：

```c
void VisionLink_OnCommand(const VisionLink_Command *command)
{
    Car_Control_Set(command->left_percent, command->right_percent);
}
```

把 `vision_link.c` 加入 Keil 的 `Application/User/Core` 组，把 `firmware/Core/Inc` 加入 Include Paths。不要在这个模块里重复定义 UART 初始化或电机引脚初始化。

