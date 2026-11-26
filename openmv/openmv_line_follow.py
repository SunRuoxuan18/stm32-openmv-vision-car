"""OpenMV black-line follower for the STM32 + OpenMV car prototype.

The script emits the fixed 8-byte frame described in protocol/PROTOCOL.md.
Tune BLACK_THRESHOLD and BASE_SPEED for the camera, lighting, and chassis.
"""

import sensor
import time
from pyb import UART


UART_PORT = 3
UART_BAUD = 115200
# Grayscale images use a two-value (min, max) threshold.
BLACK_THRESHOLD = (0, 55)
BASE_SPEED = 35
MAX_SPEED = 70
KP = 0.65
LOST_LINE_STOP_MS = 250


uart = UART(UART_PORT, UART_BAUD, timeout_char=10)
sensor.reset()
sensor.set_pixformat(sensor.GRAYSCALE)
sensor.set_framesize(sensor.QQVGA)
sensor.skip_frames(time=1500)
sensor.set_auto_gain(False)
sensor.set_auto_whitebal(False)
clock = time.clock()
sequence = 0
last_line_ms = time.ticks_ms()


def clamp(value, low, high):
    return max(low, min(high, int(value)))


def checksum(payload):
    value = 0
    for byte in payload:
        value ^= byte
    return value & 0xFF


def send_frame(command, left, right, confidence):
    global sequence
    left = clamp(left, -100, 100)
    right = clamp(right, -100, 100)
    payload = bytearray((command, left & 0xFF, right & 0xFF,
                         clamp(confidence, 0, 100), sequence & 0xFF))
    frame = bytearray((0xAA, 0x55))
    frame.extend(payload)
    frame.append(checksum(payload))
    uart.write(frame)
    sequence = (sequence + 1) & 0xFF


while True:
    clock.tick()
    image = sensor.snapshot()
    blobs = image.find_blobs([BLACK_THRESHOLD], pixels_threshold=30,
                             area_threshold=30, merge=True)
    now_ms = time.ticks_ms()

    if blobs:
        target = max(blobs, key=lambda blob: blob.pixels())
        center_x = image.width() // 2
        error = target.cx() - center_x
        confidence = clamp((target.pixels() * 100) /
                           (image.width() * image.height() * 0.25), 0, 100)
        turn = KP * error
        left = clamp(BASE_SPEED + turn, -MAX_SPEED, MAX_SPEED)
        right = clamp(BASE_SPEED - turn, -MAX_SPEED, MAX_SPEED)
        send_frame(0x01, left, right, confidence)
        last_line_ms = now_ms
    elif time.ticks_diff(now_ms, last_line_ms) > LOST_LINE_STOP_MS:
        send_frame(0x00, 0, 0, 0)

    time.sleep_ms(20)

