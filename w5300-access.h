/*
 * SPDX: MIT
 */

#ifndef	W5300_ACCESS_H
#define	W5300_ACCESS_H

#include "types.h"

void w5300_write_reg8(uint16 reg, uint8 value);
void w5300_write_reg16(uint16 reg, uint16 value);
void w5300_write_reg_buf(uint16 reg, uint8 *buffer, uint8 size);
uint16 w5300_read_reg16(uint16 reg);
int w5300_read_reg_buf(uint16 reg, uint8 *buffer, uint8 size);
void w5300_read_fifo(SOCKET s, uint16 *buffer, uint16 size);

  extern uint32 w5300_base_addr;

#endif /* W5300_ACCESS_H */
