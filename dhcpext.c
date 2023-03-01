#include <qdos.h>
#include <stdio.h>
#include <string.h>
#include "types.h"
#include "heap.h"
#include "debug.h"
#include "dhcpc.h"
#include "pt.h"
#include "timer.h"
#include "w5300-ops.h"

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

int main(int ac, char **av) {
  _super();
  mymem = (struct extmem *)sv_memalloc(sizeof(struct extmem));
  _user();
  sprintf(mymem->strmsg, "Mem at %lu\n", (uint32)mymem);
  qlstr = cstr_to_ql((QLSTR_t *)&(mymem->qlstr_msg), mymem->strmsg);
  ut_mtext((chanid_t)0, qlstr);
  setup_basic_keywords();
  if(!w5300_configure()) {
    sprintf(mymem->strmsg, "Configuration failed\n");
    qlstr = cstr_to_ql((QLSTR_t *)&(mymem->qlstr_msg), mymem->strmsg);
    ut_mtext((chanid_t)0, qlstr);
    return -1;
  }
  return 0;
}

int dhcpinit_impl() {
  ut_mtext((chanid_t)0, (QLSTR_t *)&msg_dhcpinit);
  dhcpc_init();
}

int omanreset_impl() {
  PT_INIT(&pt);
  ut_mtext((chanid_t)0, (QLSTR_t *)&msg_reset_w5300);
  if(!w5300_configure()) {
    sprintf(mymem->strmsg, "Configuration failed\n");
    qlstr = cstr_to_ql((QLSTR_t *)&(mymem->qlstr_msg), mymem->strmsg);
    ut_mtext((chanid_t)0, qlstr);
    return -1;
  }
  ut_mtext((chanid_t)0, (QLSTR_t *)&msg_done);
  return 0;
}
