#include <stdio.h>
#include <qdos.h>
#include "dhcpc.h"
#include "types.h"
#include "timer.h"
#include "pt.h"
#include "w5300.h"

static struct pt pt;
static struct timer timer;

/* Don't ask user to press enter when exiting */
char *_endmsg = NULL;

static PT_THREAD(reset_w5300(void)) {
  PT_BEGIN(&pt);
  timer_set(&timer, (clock_time_t)(2 * CLOCK_SECOND));
  *hw_reset_addr = 0;
  PT_WAIT_UNTIL(&pt,timer_expired(&timer));
  setSHAR(mac); /* set source hardware address */
  sysinit(tx_mem_conf, rx_mem_conf);
  PT_END(&pt);
}

int main(int ac, char **av) {
  static uint8 addr[4] = {0, 0, 0, 0};
  printf("Reset O'man...");
  while( PT_ENDED != reset_w5300());
  printf("Done\n");
  printf("Acquiring configuration through DHCP.\n");
  dhcpc_init();

  getSIPR((uint8 *)addr);
  printf("IP address set to : %d.%d.%d.%d\n", addr[0], addr[1], addr[2], addr[3]);
  getSUBR((uint8 *)addr);
  printf("Subnet mask set to : %d.%d.%d.%d\n", addr[0], addr[1], addr[2], addr[3]);
  getGAR((uint8 *)addr);
  printf("Gateway set to : %d.%d.%d.%d\n", addr[0], addr[1], addr[2], addr[3]);
}

void set_dns_server(uint8 * addr) {
  printf("DNS server set to : %d.%d.%d.%d\n", addr[0], addr[1], addr[2], addr[3]);
}
