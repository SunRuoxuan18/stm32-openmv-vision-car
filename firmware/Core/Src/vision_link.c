#include "vision_link.h"

static UART_HandleTypeDef *vision_uart;
static VisionLink_Command current_command;
/* Five payload bytes followed by one checksum byte. */
static uint8_t frame[6];
static uint8_t frame_index;
static uint8_t parser_state;

static uint8_t xor_checksum(const uint8_t *data, uint8_t length)
{
  uint8_t value = 0U;
  uint8_t index;
  for (index = 0U; index < length; ++index)
  {
    value ^= data[index];
  }
  return value;
}

static int8_t clamp_speed(int8_t value)
{
  if (value > 100) return 100;
  if (value < -100) return -100;
  return value;
}

static void accept_frame(void)
{
  uint8_t checksum;
  uint8_t command = frame[0];
  if ((command != VISION_LINK_CMD_STOP) &&
      (command != VISION_LINK_CMD_DRIVE) &&
      (command != VISION_LINK_CMD_HEARTBEAT))
  {
    return;
  }

  checksum = xor_checksum(frame, 5U);
  if (checksum != frame[5])
  {
    return;
  }

  current_command.command = command;
  current_command.left_percent = clamp_speed((int8_t)frame[1]);
  current_command.right_percent = clamp_speed((int8_t)frame[2]);
  current_command.confidence = (frame[3] > 100U) ? 100U : frame[3];
  current_command.sequence = frame[4];
  current_command.received_at_ms = HAL_GetTick();
  current_command.valid = 1U;

  if (command == VISION_LINK_CMD_STOP)
  {
    current_command.left_percent = 0;
    current_command.right_percent = 0;
  }
  VisionLink_OnCommand(&current_command);
}

void VisionLink_Init(UART_HandleTypeDef *huart)
{
  vision_uart = huart;
  frame_index = 0U;
  parser_state = 0U;
  current_command.command = VISION_LINK_CMD_STOP;
  current_command.left_percent = 0;
  current_command.right_percent = 0;
  current_command.confidence = 0U;
  current_command.sequence = 0U;
  current_command.received_at_ms = HAL_GetTick();
  current_command.valid = 0U;
  (void)vision_uart;
}

void VisionLink_ProcessByte(uint8_t byte)
{
  switch (parser_state)
  {
    case 0U:
      if (byte == 0xAAU) parser_state = 1U;
      break;
    case 1U:
      if (byte == 0x55U)
      {
        frame_index = 0U;
        parser_state = 2U;
      }
      else
      {
        parser_state = (byte == 0xAAU) ? 1U : 0U;
      }
      break;
    default:
      frame[frame_index++] = byte;
      if (frame_index >= 6U)
      {
        accept_frame();
        frame_index = 0U;
        parser_state = 0U;
      }
      break;
  }
}

void VisionLink_Poll(void)
{
  uint32_t elapsed;
  if (current_command.valid == 0U)
  {
    return;
  }
  elapsed = HAL_GetTick() - current_command.received_at_ms;
  if (elapsed > VISION_LINK_TIMEOUT_MS)
  {
    current_command.valid = 0U;
    current_command.command = VISION_LINK_CMD_STOP;
    current_command.left_percent = 0;
    current_command.right_percent = 0;
    VisionLink_OnCommand(&current_command);
  }
}

const VisionLink_Command *VisionLink_GetCommand(void)
{
  return &current_command;
}

uint8_t VisionLink_IsFresh(void)
{
  return (current_command.valid != 0U) &&
         ((HAL_GetTick() - current_command.received_at_ms) <= VISION_LINK_TIMEOUT_MS);
}

__weak void VisionLink_OnCommand(const VisionLink_Command *command)
{
  (void)command;
}

