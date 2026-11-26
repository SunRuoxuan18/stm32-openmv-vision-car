/* Dependency-free regression test for the OpenMV UART frame. */
'use strict';

const HEAD = [0xaa, 0x55];

function checksum(payload) {
  return payload.reduce((value, byte) => value ^ byte, 0) & 0xff;
}

function encode(command, left, right, confidence, sequence) {
  const payload = [command, left & 0xff, right & 0xff,
    confidence & 0xff, sequence & 0xff];
  return [...HEAD, ...payload, checksum(payload)];
}

function decode(frame) {
  if (frame.length !== 8 || frame[0] !== HEAD[0] || frame[1] !== HEAD[1]) {
    throw new Error('invalid frame shape');
  }
  const payload = frame.slice(2, 7);
  if (checksum(payload) !== frame[7]) throw new Error('invalid checksum');
  const signed = (value) => (value & 0x80) ? value - 0x100 : value;
  return [payload[0], signed(payload[1]), signed(payload[2]), payload[3], payload[4]];
}

if (JSON.stringify(decode(encode(0x01, 35, -20, 80, 7))) !==
    JSON.stringify([0x01, 35, -20, 80, 7])) {
  throw new Error('round-trip mismatch');
}

const broken = encode(0x01, 35, -20, 80, 7);
broken[7] ^= 0x01;
try {
  decode(broken);
  throw new Error('bad checksum was accepted');
} catch (error) {
  if (error.message === 'bad checksum was accepted') throw error;
}

if (decode(encode(0x00, 0, 0, 0, 8))[0] !== 0x00) {
  throw new Error('stop command mismatch');
}

console.log('protocol tests: PASS');

