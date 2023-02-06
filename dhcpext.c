#include <qdos.h>
#include <stdio.h>
#include <string.h>
#include "types.h"
#include "heap.h"
#include "debug.h"
#include "dhcpc.h"
#include "pt.h"
#include "w5300.h"
#include "timer.h"

void setup_basic_keywords(void);
int dhcpinit_impl(void);

static struct pt pt;
static struct timer timer;

const QLSTR_INIT(msg_dhcpinit, "DHCPINIT...\n");
const QLSTR_INIT(msg_reset_w5300, "Resetting W5300...");
const QLSTR_INIT(msg_done, "done.\n");

struct extmem {
  uint8 qlstr_msg[258];
  char strmsg[256];
} ;

struct extmem *mymem;
QLSTR_t *qlstr;

void set_dns_server(uint8 *addr) {
  sprintf(mymem->strmsg, "DNS server set to : %d.%d.%d.%d\n", addr[0], addr[1], addr[2], addr[3]);
  qlstr = cstr_to_ql((QLSTR_t *)&(mymem->qlstr_msg), mymem->strmsg);
  ut_mtext((chanid_t)0, qlstr);
}

static PT_THREAD(reset_w5300(void)) {
  PT_BEGIN(&pt);
  timer_set(&timer, (clock_time_t)(2 * CLOCK_SECOND));
  *hw_reset_addr = 0;
  PT_WAIT_UNTIL(&pt, timer_expired(&timer));
  setSHAR(mac); /* set source hardware address */
  sysinit(tx_mem_conf, rx_mem_conf);
  PT_END(&pt);
}

int main(int ac, char **av) {
  _super();
  mymem = (struct extmem *)sv_memalloc(sizeof(struct extmem));
  _user();
  sprintf(mymem->strmsg, "Mem at %lu\n", (uint32)mymem);
  qlstr = cstr_to_ql((QLSTR_t *)&(mymem->qlstr_msg), mymem->strmsg);
  ut_mtext((chanid_t)0, qlstr);
  setup_basic_keywords();
}

int dhcpinit_impl() {
  ut_mtext((chanid_t)0, (QLSTR_t *)&msg_dhcpinit);
  dhcpc_init();
}

int omanreset_impl() {
  PT_INIT(&pt);
  ut_mtext((chanid_t)0, (QLSTR_t *)&msg_reset_w5300);
  while (PT_ENDED != reset_w5300()) ;
  ut_mtext((chanid_t)0, (QLSTR_t *)&msg_done);
}
