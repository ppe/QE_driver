#include <qdos.h>
#include <string.h>
#include "pt.h"
#include "timer.h"
#include "types.h"
#include "w5300-ops.h"
#include "w5300-access.h"
#include "w5300-regs.h"

#define NUM_PROBE_ADDRESSES 7
#define PRINT0( msg ) (void)io_sstrg( (chanid_t)0, (timeout_t)0, msg, strlen(msg))

static const uint8 hexbuf[] = "0123456789ABCDEF";
static const uint32 possible_addr[] = {
    0xC000,
    0x12000,
    0x1A000,
    0xC8000,
    0xFC000,
    0x4c8000,
    0x4fc000
};
/* Make sure emory config follows the rules from datasheet, no checks done! */
static uint8 tx_mem_conf[8] = {8,8,8,8,8,8,8,8};
static uint8 rx_mem_conf[8] = {8,8,8,8,8,8,8,8};
/* locally administered mac address */
/* See: https://en.wikipedia.org/wiki/MAC_address#Ranges_of_group_and_locally_administered_addresses */
static uint8 mac[6] = {0x02,0x00,0x00,0x42,0x42,0x42};

static struct pt pt;
static struct timer timer;

static PT_THREAD(w5300_reset_impl(void)) {
  PT_BEGIN(&pt);
  timer_set(&timer, (clock_time_t)(2 * CLOCK_SECOND));
  *(vuint8 *)(w5300_base_addr + 0x400) = 0;
  PT_WAIT_UNTIL(&pt,timer_expired(&timer));
  PT_END(&pt);
}

void w5300_reset() {
  PT_INIT(&pt);
  PRINT0("Resetting...");
  while(PT_ENDED != w5300_reset_impl());
  PRINT0("done\n");
}

void w5300_memory_config() {
  w5300_write_reg_buf(W5300_TMSR0, tx_mem_conf, 8);
  w5300_write_reg_buf(W5300_RMSR0, rx_mem_conf, 8);
   /* 8x8kB blocks of TX mem and 8x8kB blocks of RX mem, see datasheet */
   w5300_write_reg16(W5300_MTYPER, 0x00FF);
}

int w5300_set_base_address() {
    int i;
    char outbuf[8];
    uint32 addr;
    PRINT0("Probing for W5300 chip...");
    for (i = 0; i < NUM_PROBE_ADDRESSES; i++) {
        w5300_base_addr = possible_addr[i];
        if (w5300_read_reg16(W5300_IDR) == 0x5300) {
            break;
        }
    }

    if (i == NUM_PROBE_ADDRESSES) {
        PRINT0("NOT FOUND");
        return 0;
    }

    addr = w5300_base_addr;
    i = 6;
    outbuf[7] = 0;

    do{
        outbuf[i] = hexbuf[addr % 16];
        i--;
        addr /= 16;
    } while( addr > 0);
    PRINT0(&outbuf[i+1]);
    PRINT0("\n");
    return 1;
}

int w5300_configure() {
  if(!w5300_set_base_address()) {
    return 0;
  }
  w5300_reset();
  w5300_memory_config();
  /* set mac address */
  PRINT0("Set SHAR\n");
  w5300_write_reg_buf(W5300_SHAR, mac, 6);
  return 1;
}
