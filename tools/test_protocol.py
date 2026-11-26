#!/usr/bin/env python3
"""Small dependency-free regression test for the OpenMV UART frame."""

FRAME_HEAD = bytes((0xAA, 0x55))


def checksum(payload):
    result = 0
    for byte in payload:
        result ^= byte
    return result


def encode(command, left, right, confidence, sequence):
    payload = bytes((command, left & 0xFF, right & 0xFF,
                     confidence & 0xFF, sequence & 0xFF))
    return FRAME_HEAD + payload + bytes((checksum(payload),))


def parse(frame):
    assert len(frame) == 8
    assert frame[:2] == FRAME_HEAD
    payload = frame[2:7]
    assert checksum(payload) == frame[7]
    left = frame[3] - 256 if frame[3] & 0x80 else frame[3]
    right = frame[4] - 256 if frame[4] & 0x80 else frame[4]
    return frame[2], left, right, frame[5], frame[6]


def main():
    frame = encode(0x01, 35, -20, 80, 7)
    assert parse(frame) == (0x01, 35, -20, 80, 7)
    broken = bytearray(frame)
    broken[-1] ^= 0x01
    try:
        parse(bytes(broken))
    except AssertionError:
        pass
    else:
        raise AssertionError("bad checksum was accepted")
    assert parse(encode(0x00, 0, 0, 0, 8))[0] == 0x00
    print("protocol tests: PASS")


if __name__ == "__main__":
    main()

