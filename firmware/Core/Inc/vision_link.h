#ifndef VISION_LINK_H
#define VISION_LINK_H

#include "stm32f4xx_hal.h"
#include <stdint.h>

#define VISION_LINK_FRAME_SIZE       8U
#define VISION_LINK_TIMEOUT_MS       250U
#define VISION_LINK_CMD_STOP         0x00U
#define VISION_LINK_CMD_DRIVE        0x01U
#define VISION_LINK_CMD_HEARTBEAT    0x02U

typedef struct
{
  uint8_t command;
  int8_t left_percent;
  int8_t right_percent;
  uint8_t confidence;
  uint8_t sequence;
  uint32_t received_at_ms;
  uint8_t valid;
} VisionLink_Command;

void VisionLink_Init(UART_HandleTypeDef *huart);
void VisionLink_ProcessByte(uint8_t byte);
void VisionLink_Poll(void);
const VisionLink_Command *VisionLink_GetCommand(void);
uint8_t VisionLink_IsFresh(void);

/* Override this weak hook to call the board's motor driver. */
void VisionLink_OnCommand(const VisionLink_Command *command);

#endif

