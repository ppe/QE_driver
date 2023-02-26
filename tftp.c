#include "tftp.h"
#include "pt.h"
#include "socket.h"
#include "timer.h"
#include "types.h"
#include <fcntl.h>
#include <qdos.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* TODO uses a fixed socket and static global state -> there can only be one
 * tftp get in progress */

#define ERROR_NONE 0
#define ERROR_MAX_RESENDS 1
#define ERROR_SERVER_ERROR 2
#define ERROR_SEND 3
#define ERROR_FILE_IO 4

#define GET_OK 1
#define GET_ERROR 0

#define MAX_RESENDS 3

#define OP_RRQ 1
#define OP_DATA 3
#define OP_ACK 4
#define OP_ERROR 5

/* DATA packet with 4 bytes of header and 512 bytes of payload */
#define FULL_PACKET_SIZE 516
#define ACK_PACKET_SIZE 4
#define MIN_DATA_PACK_SIZE 4
#define RCV_BUF_SIZE 1600

#define BROADCAST_ADDRESS 0xFFFFFFFF
#define TFTP_SERVER_PORT 69

#define STATE_SEND_RRQ 0
#define STATE_DATA_RECEIVED 1
#define STATE_WAIT_FOR_DATA 2
#define STATE_RECEIVE_COMPLETE 3
#define STATE_SEND_ACK 4

struct pt pt;
struct timer timer;

const SOCKET socket = (SOCKET)0;
char *mode_octet = "octet";

static uint16 src_port; /* Source port of this client */
static uint8 sendbuf[600];
static uint16 send_packet_len = 0;
static uint8 rcvbuf[RCV_BUF_SIZE];
static uint32 recv_len = 0;
static uint32 total_recv_len = 0;
static uint8 state = STATE_SEND_RRQ;
static uint8 num_resends = 0;
static uint8 error = ERROR_NONE;
static uint16 expected_block = 0;
// Remote address and port of reveived packet
static uint8 remote_addr[4] = {255, 255, 255, 255};
static uint16 remote_port;
// Expected address and port of reveiced packet
static uint8 expected_addr[4] = {0, 0, 0, 0};
static uint16 expected_port = 0;

static uint16 pack_type = OP_RRQ;
static uint16 block_no = 0;
static uint32 sent_len = 0;

static int file_bytes_written;

uint16 create_rrq(QLSTR_t *file_name) {
  uint8 *buf = &sendbuf[2];
  char *c = file_name->qs_str;
  int i;
  uint16 total = 2;
  *((uint16 *)sendbuf) = OP_RRQ;
  for (i = 0; i < file_name->qs_strlen; i++) {
    *buf++ = *c++;
    total++;
  }
  *buf++ = 0;
  total++;
  c = mode_octet;
  while ((*buf++ = *c++)) {
    total++;
  }
  return ++total;
}

static PT_THREAD(handle_get(QLSTR_t *file_name, chanid_t output_channel)) {
  PT_BEGIN(&pt);
  state = STATE_SEND_RRQ;
  /* Initial request packet goes to broadcast address, port 69 */
  *((uint32 *)expected_addr) = BROADCAST_ADDRESS;
  expected_port = TFTP_SERVER_PORT;
  do {
    if (STATE_SEND_RRQ == state) {
      send_packet_len = create_rrq(file_name);
      sent_len = sendto(socket, sendbuf, send_packet_len, expected_addr,
                        expected_port);
      if (sent_len != send_packet_len) {
        error = ERROR_SEND;
        PT_EXIT(&pt);
      }
      timer_set(&timer, (clock_time_t)2 * CLOCK_SECOND);
      expected_block = 1;
      state = STATE_WAIT_FOR_DATA;
    };
    if (state == STATE_WAIT_FOR_DATA) {
      /* printf("/WFD "); */
      timer_restart(&timer);
      PT_WAIT_UNTIL(&pt,
                    bytes_available((SOCKET)socket) || timer_expired(&timer));
      if (bytes_available(socket)) {

        /* printf("#R "); */
        recv_len =
            recvfrom(socket, rcvbuf, RCV_BUF_SIZE, remote_addr, &remote_port);
        /* printf("R %ulb ", recv_len); */

        if (recv_len < MIN_DATA_PACK_SIZE) {
          /* printf(" XUMX "); */
          continue;
        }
        /* Ignore packets from wrong host/port combination after initial DATA */
        /* packet */
        if (expected_block > 1 &&
            !(*((uint32 *)expected_addr) == *((uint32 *)remote_addr) &&
              expected_port == remote_port)) {
          /* printf(" XWHX "); */
          continue;
        }
        // Check packet type
        pack_type = ((uint16 *)rcvbuf)[0];
        if (OP_ERROR == pack_type) {
          error = ERROR_SERVER_ERROR;
          /* printf("\nErrormsg: %s\n", &(rcvbuf[4])); */
          PT_EXIT(&pt);
        }
        if (OP_DATA != pack_type) {
          // Ignore non-data/error packets
          /* printf(" XWPX "); */
          continue;
        }
        // Verify block #
        block_no = ((uint16 *)rcvbuf)[1];
        /* printf("P %db ", block_no); */
        if (block_no != expected_block) {
          // Ignore out of sequence & duplicate packets
          if (block_no != (expected_block - 1)) {
            /* printf(" _WB_ "); */
            continue;
          }
        } else {
          if (expected_block == 1) {
            /* printf("\nSwitch to %d.%d.%d.%d %d\n", remote_addr[0],
             * remote_addr[1], remote_addr[2], remote_addr[3],remote_port); */
            expected_port = remote_port;
            *((uint32 *)expected_addr) = *((uint32 *)remote_addr);
          }
          // TODO: save data
          // Write to output channel
          /* printf("> %ul. NB %d ", recv_len-4, expected_block ); */
          /* Start from byte 5 to skip over data block header */
          file_bytes_written = io_sstrg(output_channel, (timeout_t)-1,
                                        (void *)(&rcvbuf[4]), recv_len - 4);
          expected_block++;
          total_recv_len += file_bytes_written;
          /* printf("IOW %d ", file_bytes_written); */
          if (file_bytes_written < 0) {
            error = ERROR_FILE_IO;
            ut_err0(file_bytes_written);
            PT_EXIT(&pt);
          }
        }
        num_resends = 0;
        state = STATE_SEND_ACK;
      } else {
        /* printf("TO %d ", num_resends); */
        // No data available on socket so we timed out waiting for packet
        num_resends++;
        if (num_resends++ > MAX_RESENDS) {
          error = ERROR_MAX_RESENDS;
          PT_EXIT(&pt);
        } else {
          state = expected_block == 1 ? STATE_SEND_RRQ : STATE_SEND_ACK;
        }
      }
      if (STATE_SEND_ACK == state) {
        // Verify amount of data
        state = FULL_PACKET_SIZE > recv_len ? STATE_RECEIVE_COMPLETE
                                            : STATE_WAIT_FOR_DATA;
        // Send ACK packet
        /* printf("A %d. ",block_no); */
        ((uint16 *)sendbuf)[0] = OP_ACK;
        ((uint16 *)sendbuf)[1] = block_no;
        sent_len = sendto(socket, sendbuf, ACK_PACKET_SIZE, expected_addr,
                          expected_port);
        if (ACK_PACKET_SIZE != sent_len) {
          error = ERROR_SEND;
          PT_EXIT(&pt);
        }
        // quit if end of transmission
      }
    };
  } while (STATE_RECEIVE_COMPLETE != state);
  PT_END(&pt);
}

int tftp_get(QLSTR_t *file_name, char *destination) {
  int ret = GET_OK, fd;
  uint8 pt_status;
  chanid_t output_channel;
  char c_file_name[65];
  char device_file_name[100];
  // TODO: verify file name len > 0 and < ??
  qlstr_to_c(c_file_name, file_name);
  device_file_name[0] = 0;
  strcat(device_file_name, destination);
  strcat(device_file_name, c_file_name);
  printf("Opening file %s\n", device_file_name);
  output_channel = io_open(device_file_name, NEW_OVER);
  /* fd = opene(device_file_name, NEW_OVER, 0); */
  /* if( fd < 0 ) { */
  /*   printf("Error opening file : %d\n", fd); */
  /*   return fd; */
  /* } */
  /* output_channel = getchid(fd); */
  /* printf("Opened channel : %d\n", output_channel); */
  if (output_channel < 0)
    return output_channel; /* QDOS error in channel # */
  PT_INIT(&pt);
  do {
    pt_status = handle_get(file_name, output_channel);
    if (PT_EXITED == pt_status) {
      printf("\nGet returned %d\n", error);
      // TODO: print the error code and possible error message from server
      ret = GET_ERROR;
      break;
    }
  } while (PT_ENDED != pt_status);
  io_close(output_channel);
  return ret;
}

int main(int ac, char **av) {
  QLSTR_DEF(fname, 256);
  int i, status;
  char *data_use;
  char *destination;
  if ((ac < 2 && ac > 3) || strlen(av[1]) > 255) {
    printf("Usage: tftp file_name [destination]\n");
    exit(-1);
  }
  /* When TK2 in use C68 puts PROG_USE,DATA_USE & SPL_USE into environment */
  data_use = getenv("DATA_USE");
  if (ac == 2) {
    if (NULL == data_use) {
      printf("DATA_USE directory not specified, must supply destination.\n");
      exit(-1);
    }
    destination = data_use;
  } else {
    destination = av[2];
  }
  srand((unsigned int)mt_rclck());
  src_port = (uint16)(rand() & 0xFFFF);
  printf("Using source port %ud\n", src_port);
  open_socket(socket, Sn_MR_UDP, src_port, 0);
  cstr_to_ql((QLSTR_t *)&(fname), av[1]);
  status = tftp_get((QLSTR_t *)&fname, destination);
  if (status < 0) {
    ut_err0(status);
  }
  printf("Received a total of %d bytes.", total_recv_len);
  socket_close(socket);
}
