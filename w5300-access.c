/*
 * SPDX: MIT
 */
#include "w5300-access.h"
#include "w5300-regs.h"
#include "types.h"

uint32 w5300_base_addr;

void w5300_write_reg8(uint16 reg, uint8 value) {
    *(vuint8 *)(w5300_base_addr + reg) = value;
}

void w5300_write_reg16(uint16 reg, uint16 value) {
    *(vuint16 *)(w5300_base_addr + reg) = value;
}

void w5300_write_reg32(uint16 reg, uint32 value) {
    *(vuint32 *)(w5300_base_addr + reg) = value;
}

void w5300_write_reg_buf(uint16 reg, uint8 *buffer, uint8 size) {
    int i;

    for (i = 0; i < size; i++) {
        w5300_write_reg8(reg + i, buffer[i]);
    }
}

uint16 w5300_read_reg16(uint16 reg) {
    return (*(vuint16 *)(w5300_base_addr + reg));
}

uint32 w5300_read_reg32(uint32 reg) {
    return (*(vuint32 *)(w5300_base_addr + reg));
}

int w5300_read_buf(uint16 reg, uint8 *buffer, uint8 size) {

}

void w5300_read_fifo(SOCKET s, uint16 *buffer, uint16 size) {
  uint32 i;
  uint32 words_to_read = 0;
  if (size & 1) {
    words_to_read = (size >> 1) + 1;
  } else {
    words_to_read = size >> 1;
  }
  for (i = 0; i < words_to_read; i++) {
    buffer[i] = w5300_read_reg16(W5300_Sn_RX_FIFOR(s));
  }
}

void w5300_write_fifo(SOCKET s, uint16 *buffer, uint16 size) {
  uint32 i;
  uint32 words_to_write = 0;
  if (size & 1) {
    words_to_write = (size >> 1) + 1;
  } else {
    words_to_write = size >> 1;
  }
  for (i = 0; i < words_to_write; i++) {
    w5300_write_reg16(W5300_Sn_TX_FIFOR(s),buffer[i]);
  }
}
