#include <qdos.h>
#include <string.h>
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
        return -1;
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
