#include <stdio.h>
#include <qdos.h>
#include "dhcpc.h"
#include "types.h"
#include "w5300-access.h"
#include "w5300-ops.h"
#include "w5300-regs.h"

static struct pt pt;
static struct timer timer;

/* Don't ask user to press enter when exiting */
char *_endmsg = NULL;

int main(int ac, char **av) {
  static uint8 addr[4] = {0, 0, 0, 0};
  printf("Configure O'man...");
  if(!w5300_configure()) {
    printf("FAILED\n");
    return -1;
  }
  printf("Done!\n");
  printf("Acquiring configuration through DHCP...\n");
  dhcpc_init();

  w5300_read_reg_buf(W5300_SIPR, addr, 4);
  printf("IP address set to : %d.%d.%d.%d\n", addr[0], addr[1], addr[2], addr[3]);
  w5300_read_reg_buf(W5300_SUBR, addr, 4);
  printf("Subnet mask set to : %d.%d.%d.%d\n", addr[0], addr[1], addr[2], addr[3]);
  w5300_read_reg_buf(W5300_GAR, addr, 4);
  printf("Gateway set to : %d.%d.%d.%d\n", addr[0], addr[1], addr[2], addr[3]);
}

void set_dns_server(uint8 * addr) {
  printf("DNS server set to : %d.%d.%d.%d\n", addr[0], addr[1], addr[2], addr[3]);
}
