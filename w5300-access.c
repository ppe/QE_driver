/*
 * SPDX: MIT
 */
#include "w5300-access.h"
#include "types.h"

uint32 w5300_base_addr;

void w5300_write_reg8(uint16 reg, uint8 value) {
    *(vuint8 *)(w5300_base_addr + reg) = value;
}

void w5300_write_reg16(uint16 reg, uint16 value) {
    *(vuint16 *)(w5300_base_addr + reg) = value;
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

int w5300_read_buf(uint16 reg, uint8 *buffer, uint8 size) {

}
