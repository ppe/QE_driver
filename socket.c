/**
 * \file    socket.c
 *   Implemetation of WIZnet SOCKET API fucntions
 *
 * This file implements the WIZnet SOCKET API functions that is used in your internat application program.
 *
 * Revision History :
 * ----------  -------  -----------  ----------------------------
 * Date        Version  Author       Description
 * ----------  -------  -----------  ----------------------------
 * 24/03/2008  1.0.0    MidnightCow  Release with W5300 launching
 * ----------  -------  -----------  ----------------------------
 * 15/05/2008  1.1.0    MidnightCow  Refer to M_15052008.
 *                                   Modify the warning code block in recv().
 * ----------  -------  -----------  ----------------------------
 * 04/07/2008  1.1.1    MidnightCow  Refer to M_04072008.
 *                                   Modify the warning code block in recv().
 * ----------  -------  -----------  ----------------------------
 * 08/08/2008  1.2.0    MidnightCow  Refer to M_08082008.
 *                                   Modify close().
 * ----------  -------  -----------  ----------------------------
 * 11/25/2008  1.2.1    Bongjun      Refer to M_11252008.
 *                                   Modify close().
 * ----------  -------  -----------  ----------------------------
 */


/* #include "lstring.h" */
#include <string.h>
#include <stdio.h>
#include "socket.h"
#include "heap.h"
#include "debug.h"
#include "w5300-access.h"
#include "w5300-regs.h"

/**
 * Variable for temporary source port number
 */
uint16   iinchip_source_port;

/**
 * The flag to check if first send or not.
 */
uint8    check_sendok_flag[MAX_SOCK_NUM];

/* TCP_MAX_MTU */
char *recv_buf[MAX_SOCK_NUM];
int tcp_pack_remain[MAX_SOCK_NUM];
int tcp_pack_size[MAX_SOCK_NUM];
char *recv_buf_ptr[MAX_SOCK_NUM];


uint8    open_socket(SOCKET s, uint8 protocol, uint16 port, uint16 flag)
{
   IINCHIP_WRITE(Sn_MR(s),(uint16)(protocol | flag)); /* set Sn_MR with protocol & flag */
   if (port != 0) IINCHIP_WRITE(Sn_PORTR(s),port);
   else
   {
      iinchip_source_port++;     /* if don't set the source port, set local_port number. */
      IINCHIP_WRITE(Sn_PORTR(s),iinchip_source_port);
   }
   setSn_CR(s, Sn_CR_OPEN);      /* open s-th SOCKET  */

   check_sendok_flag[s] = 1;     /* initialize the sendok flag. */

   #ifdef __DEF_IINCHIP_DBG__
      printf("%d : Sn_MR=0x%04x,Sn_PORTR=0x%04x(%d),Sn_SSR=%04x\r\n",s,IINCHIP_READ(Sn_MR(s)),IINCHIP_READ(Sn_PORTR(s)),IINCHIP_READ(Sn_PORTR(s)),getSn_SSR(s));
   #endif
   return 1;
}

void socket_drain(SOCKET s) {
    /* Drain pending input from socket */
}

uint8 is_closed( SOCKET s ) {
    return SOCK_CLOSED == getSn_SSR( s );
}

uint32 bytes_available( SOCKET s ) {
    return getSn_RX_RSR(s);
}

void socket_close(SOCKET s)
{
   /* M_08082008 : It is fixed the problem that Sn_SSR cannot be changed a undefined value to the defined value. */
   /*              Refer to Errata of W5300 */
   /*Check if the transmit data is remained or not. */
   /* if( ((getSn_MR(s)& 0x0F) == Sn_MR_TCP) && (getSn_TX_FSR(s) != getIINCHIP_TxMAX(s)) ) */
   /* { */
   /*    uint16 loop_cnt =0; */
   /*    while(getSn_TX_FSR(s) != getIINCHIP_TxMAX(s)) */
   /*    { */
   /*       if(loop_cnt++ > 10) */
   /*       { */
   /*          uint8 destip[4]; */
   /*          /\* M_11252008 : modify dest ip address *\/ */
   /*          /\*getSIPR(destip); *\/ */
   /*          destip[0] = 0;destip[1] = 0;destip[2] = 0;destip[3] = 1; */
   /*          open_socket(s,Sn_MR_UDP,0x3000,0); */
   /*          sendto(s,(uint8*)"x",1,destip,0x3000); /\* send the dummy data to an unknown destination(0.0.0.1). *\/ */
   /*          break; /\* M_11252008 : added break statement *\/ */
   /*       } */
   /*       wait_10ms(10); */
   /*    } */
   /* } */
   /*////////////////////////////    */
   setSn_IR(s ,0x00FF);          /* Clear the remained interrupt bits. */
   setSn_CR(s ,Sn_CR_CLOSE);     /* Close s-th SOCKET      */
}

uint8    connect(SOCKET s, uint8 * addr, uint16 port)
{
   uint16 socket_status = 0;
   if
   (
      ((addr[0] == 0xFF) && (addr[1] == 0xFF) && (addr[2] == 0xFF) && (addr[3] == 0xFF)) ||
      ((addr[0] == 0x00) && (addr[1] == 0x00) && (addr[2] == 0x00) && (addr[3] == 0x00)) ||
      (port == 0x00)
   )
   {
      #ifdef __DEF_IINCHIP_DBG__
         printf("%d : Fail[invalid ip,port] %u.%u.%u.%u:%u\r\n",s,addr[0],addr[1],addr[2],addr[3],port);
      #endif
      return 0;
   }

   /* set destination IP  */
   IINCHIP_WRITE(Sn_DIPR(s),((uint16)addr[0]<<8)+(uint16)addr[1]);
   IINCHIP_WRITE(Sn_DIPR2(s),((uint16)addr[2]<<8)+(uint16)addr[3]);
   /* set destination port number */
   IINCHIP_WRITE(Sn_DPORTR(s),port);

   /* Connect */
   setSn_CR(s,Sn_CR_CONNECT);
   while(getSn_CR(s) != 0 ); /* Wait for w5300 to ack cmd by clearing the CR */
   do {
      socket_status = getSn_SSR(s);
   } while (socket_status != SOCK_CLOSED && socket_status != SOCK_ESTABLISHED && socket_status != SOCK_FIN_WAIT);
   #ifdef __DEF_IINCHIP_DBG__
            printf("%d: connect()\n",s);
            printf("%d: Sn_MR=0x%04x,Sn_PORTR=0x%04x(%d),Sn_SSR=%04x\r\n",s,IINCHIP_READ(Sn_MR(s)),IINCHIP_READ(Sn_PORTR(s)),IINCHIP_READ(Sn_PORTR(s)),getSn_SSR(s));
            printf("%d: ip=%8x\n",s,*((uint32*)addr));
            printf("%d: port=%d\n",s,port);
   #endif

   return 1;
}

void     disconnect(SOCKET s)
{
   if((getSn_MR(s)& 0x0F) != Sn_MR_TCP) {return;};
   setSn_CR(s,Sn_CR_DISCON);
   while(getSn_CR(s));
}

uint8    listen(SOCKET s)
{
   if (getSn_SSR(s) != SOCK_INIT)
   {
      #ifdef __DEF_IINCHIP_DBG__
         printf("%d : SOCKET is not created!\r\n",s);
      #endif
      return 0;
   }

   setSn_CR(s,Sn_CR_LISTEN);     /* listen */

   return 1;
}

int32   socket_send(int s, uint8 * buf, uint32 len)
{
    static uint8 status=0;
    static uint32 ret=0;
    static uint32 freesize=0;
    TRACE(("socket_send: s=%d,buf=%08x,len=%08x\n", s, buf, len));
    status = 0;
    freesize = 0;
    ret = len;
   /* if (len > getIINCHIP_TxMAX(s)) ret = getIINCHIP_TxMAX(s); /\* check size not to exceed MAX size. *\/ */
   TRACE(("%d : freesize=%ld,status=%04x\n",s,freesize,status));
   TRACE(("%d:Send Size=%08lx(%d)\n",s,ret,ret));
   TRACE(("MR=%04x\r\n",*((vuint16*)MR)));

   /*
    * \note if you want to use non blocking function, <b>"do{}while(freesize < ret)"</b> code block
    * can be replaced with the below code. \n
    * \code
    *       while((freesize = getSn_TX_FSR(s))==0);
    *       ret = freesize;
    * \endcode
    */
   /* ----------------------- */
   /* NOTE : CODE BLOCK START */
   while((freesize = getSn_TX_FSR(s))==0);
   if( ret > freesize ) {
       ret = freesize;
   }
   /* do */
   /* { */
   /*    freesize = getSn_TX_FSR(s); */
   /*    status = getSn_SSR(s); */
   /*    TRACE(("%d : freesize=%ld\n",s,freesize)); */
   /*       if(loopcnt++ > 0x0010000) */
   /*       /\* { *\/ */
   /*       /\*    printf("%d : freesize=%ld,status=%04x\n",s,freesize,status); *\/ */
   /*       /\*    printf("%d:Send Size=%08lx(%d)\n",s,ret,ret); *\/ */
   /*       /\*    printf("MR=%04x\r\n",*((vuint16*)MR)); *\/ */
   /*       /\*    loopcnt = 0; *\/ */
   /*       /\* } *\/ */
   /*           if ((status != SOCK_ESTABLISHED) && (status != SOCK_CLOSE_WAIT)) { */
   /*               TRACE(("Wrong socket state: %d\n", status)); */
   /*               return 0; */
   /*           }; */
   /* } while (freesize < ret); */
   /* NOTE : CODE BLOCK END */
   /* --------------------- */

   TRACE(("%d : write %d bytes\n", s, ret));
   wiz_write_buf(s,buf,ret);                 /* copy data */

   if(!check_sendok_flag[s])                 /* if first send, skip. */
   {
      while (!(getSn_IR(s) & Sn_IR_SENDOK))  /* wait previous SEND command completion. */
      {
      /* #ifdef __DEF_IINCHIP_DBG__ */

      /*    if(loopcnt++ > 0x010000) */
      /*    { */
      /*       printf("%d:Sn_SSR(%04x)\r\n",s,status); */
      /*       printf("%d:Send Size=%08lx(%d)\r\n",s,ret,ret); */
      /*       printf("MR=%04x\r\n",*((vuint16*)MR)); */
      /*       loopcnt = 0; */
      /*    } */
      /* #endif */
         if (getSn_SSR(s) == SOCK_CLOSED)    /* check timeout or abnormal closed. */
         {
            /* #ifdef __DEF_IINCHIP_DBG__ */
            /*    printf("%d : Send Fail. SOCK_CLOSED.\r\n",s); */
            /* #endif */
            return -1;
         }
      }
      setSn_IR(s, Sn_IR_SENDOK);             /* clear Sn_IR_SENDOK	 */
   }
   else check_sendok_flag[s] = 0;

   /* send */
   setSn_TX_WRSR(s,ret);
   setSn_CR(s,Sn_CR_SEND);
   return ret;
}

void clear_cache_entry(struct peek_cache_entry *cache_entry) {
      if(NULL != cache_entry->buffer) {
         sv_memfree(cache_entry->buffer);
      }
      cache_entry->bytes_available = 0;
      cache_entry->read_position = NULL;
      cache_entry->buffer = NULL;
}

void cache_data(void *buf, uint32 num_bytes, struct peek_cache_entry *cache_entry) {
   if( (cache_entry->buffer = sv_memalloc(num_bytes)) != NULL ) {
      cache_entry->bytes_available = num_bytes;
      cache_entry->read_position = cache_entry->buffer;
   }
}

void initialize_peek_cache( struct peek_cache_entry *cache, uint8 num_elements ) {
   uint8 i = 0;
   for (i = 0; i < num_elements; i++) {
      clear_cache_entry(&cache[i]);
   }
}

int32 socket_recv(SOCKET sock, char* buf ) {
    register uint8 sock_status;
    register uint16 wait_cycles;
    register uint32 fifo_addr;
    register int i;
    uint32 bytes_available;
    static uint16 dev_pack_size;
    static int to_recv = 0;

    wait_cycles = 0;
    dev_pack_size = 0;
    to_recv = 0;
    bytes_available = 0;

    i = (int)sock;
    TRACE(("%d: receive to buf at %08x\n",i,buf));
    if( tcp_pack_remain[i] > 0 ) {
        to_recv = tcp_pack_remain[i] > TCP_MAX_MTU ? TCP_MAX_MTU : tcp_pack_remain[i];
        tcp_pack_remain[i] -= to_recv;
        /* PRINTB0('+'); */
        /* ut_mint((chanid_t)0,tcp_pack_remain[i]); */
    } else {
        sock_status = GetSn_SSR(sock);
        fifo_addr = Sn_RX_FIFOR(sock);
        while ((bytes_available = getSn_RX_RSR(sock)) == 0
               && (sock_status != SOCK_CLOSED)
               && (sock_status != SOCK_CLOSE_WAIT)) {
            if( ++wait_cycles > MAX_WAIT_CYCLES ) {
                return -1;
            }
            wait_10ms(1);
            sock_status = GetSn_SSR(sock);
        }
        dev_pack_size = IINCHIP_READ(fifo_addr);
        /* PRINTB0('%'); */
        /* ut_mint((chanid_t)0,dev_pack_size); */
        if( dev_pack_size > TCP_MAX_MTU ) {
            to_recv = TCP_MAX_MTU;
            tcp_pack_remain[i] = dev_pack_size - to_recv;
        } else {
            to_recv = dev_pack_size;
            tcp_pack_remain[i] = 0;
        }
        /* PRINTB0('#'); */
        /* ut_mint((chanid_t)0,tcp_pack_remain[i]); */
    }
    tcp_pack_size[i] = to_recv;
    tcp_pack_remain[i] = tcp_pack_size[i];
    /* PRINTB0('+'); */
    /* ut_mint((chanid_t)0,tcp_pack_size); */
    /* PRINTB0('+'); */
    /* PRINTB0(tcp_buf[0]); */
    asm volatile ( "
 movea.l %0,a0
 movea.l %1,a1
 moveq.l #0,d0
 moveq.l #0,d1
 move.w  %2,d0
 lsr.w   #1,d0
 addx.w  d1,d0
unroll_start:
 cmpi.w  #20,d0
 blt     copy_loop_end
 move.w  (a0),(a1)+
 move.w  (a0),(a1)+
 move.w  (a0),(a1)+
 move.w  (a0),(a1)+
 move.w  (a0),(a1)+
 move.w  (a0),(a1)+
 move.w  (a0),(a1)+
 move.w  (a0),(a1)+
 move.w  (a0),(a1)+
 move.w  (a0),(a1)+
 move.w  (a0),(a1)+
 move.w  (a0),(a1)+
 move.w  (a0),(a1)+
 move.w  (a0),(a1)+
 move.w  (a0),(a1)+
 move.w  (a0),(a1)+
 move.w  (a0),(a1)+
 move.w  (a0),(a1)+
 move.w  (a0),(a1)+
 move.w  (a0),(a1)+
 sub.w   #19,d0
 dbra    d0,unroll_start
 bra     copy_loop_end
copy_loop: move.w  (a0),(a1)+
copy_loop_end: dbra    d0,copy_loop
         " :
           : "a" (fifo_addr), "a" (buf), "d" (tcp_pack_size[i])
           : "a0", "a1", "d0", "d1"
        );
    /* PRINTB0('+'); */
    /* PRINTB0(tcp_buf[0]); */
    if( !tcp_pack_remain[i] > 0 ) {
        /* PRINTB0('<'); */
        /* PRINTB0(10); */
        setSn_CR(sock,Sn_CR_RECV);
        while(getSn_CR(sock) != 0); /* Wait for w5300 to ack cmd by clearing the CR */
    }
    return tcp_pack_size[i];
}

uint32   sendto(SOCKET s, uint8 * buf, uint32 len, uint8 * addr, uint16 port) {
   uint8 status=0;
   uint8 isr=0;
   uint32 ret=0;

   TRACE(("Sendto buf %08x\n",buf));
   TRACE(("%d : sendto():%d.%d.%d.%d(%d), len=%d\n",s, addr[0], addr[1], addr[2], addr[3] , port, len));
   #ifdef __DEF_IINCHIP_DBG__
      printf("%d : sendto():%d.%d.%d.%d(%d), len=%d\r\n",s, addr[0], addr[1], addr[2], addr[3] , port, len);
   #endif

   if (((addr[0] == 0x00) && (addr[1] == 0x00) && (addr[2] == 0x00) && (addr[3] == 0x00)) ||
      ((port == 0x00)) ||(len == 0))
   {
       TRACE(("%d : Fail[%d.%d.%d.%d, %.d, %d]\n",s, addr[0], addr[1], addr[2], addr[3] , port, len));
      #ifdef __DEF_IINCHIP_DBG__
         printf("%d : Fail[%d.%d.%d.%d, %.d, %d]\r\n",s, addr[0], addr[1], addr[2], addr[3] , port, len);
      #endif
      return 0;
   }


   /* if (len > getIINCHIP_TxMAX(s)) { ret = getIINCHIP_TxMAX(s); } /\* check size not to exceed MAX size. *\/ */
   /* else { ret = len; } */
   ret = len;

   TRACE(("sendto ret == %d\n", ret));

   /* set destination IP address */
   w5300_write_reg32(W5300_Sn_DIPR(s), *((uint32 *)addr));
   /* set destination port number */
   w5300_write_reg16(W5300_Sn_DPORTR(s), port);
   /* copy data */
   w5300_write_fifo(s, (uint16 *)buf, ret);
   /* send */
   w5300_write_reg32(W5300_Sn_TX_WRSR(s), ret);
   w5300_write_reg16(W5300_Sn_CR(s), W5300_Sn_CR_SEND);
   /* wait SEND command completion */
   while (!((isr = w5300_read_reg16(W5300_Sn_IR(s))) & W5300_Sn_IR_SENDOK)) {
      /* warning --------------------------------------- */
      /* Sn_IR_TIMEOUT causes the decrement of Sn_TX_FSR */
      /* ----------------------------------------------- */
      status = w5300_read_reg16(W5300_Sn_SSR(s));
      if ((status == W5300_SOCK_CLOSED) || (isr & W5300_Sn_IR_TIMEOUT))
      {
          TRACE(("%d: send fail.status=0x%02x,isr=%02x\r\n",s,status,isr));
         #ifdef __DEF_IINCHIP_DBG__
            printf("%d: send fail.status=0x%02x,isr=%02x\r\n",s,status,isr);
         #endif
         w5300_write_reg16(W5300_Sn_IR(s), W5300_Sn_IR_TIMEOUT);
         return 0;
      }
   }
   w5300_write_reg16(W5300_Sn_IR(s), W5300_Sn_IR_SENDOK);

   TRACE(("%d : send()end ret %u\r\n",s,ret));
   #ifdef __DEF_IINCHIP_DBG__
      printf("%d : send()end ret %u\r\n",s,ret);
   #endif

   return ret;
}

uint32   recvfrom(SOCKET s, uint8 * buf, uint32 len, uint8 * addr, uint16  *port)
{
   static uint16 head[4] __attribute__((aligned(2)));
   static uint32 data_len;
   static uint16 crc[2];

   data_len = 0;
   TRACE(("Recvfrom buf %08x\n",buf));
   #ifdef __DEF_IINCHIP_DBG__
      printf("recvfrom()\r\n");
   #endif

   if ( len > 0 )
   {
      switch (w5300_read_reg16(W5300_Sn_MR(s)) & W5300_Sn_MR_MODE_MASK) {
         case W5300_Sn_MR_UDP:
            /* extract PACKET-INFO */
            w5300_read_fifo(s, head, 8);
            /* read peer's IP address, port number. */
            addr[0] = (uint8)(head[0] >> 8);       /* destination IP address */
            addr[1] = (uint8)head[0];
            addr[2] = (uint8)(head[1]>>8);
            addr[3] = (uint8)head[1];
            *port = head[2];                       /* destination port number */
            data_len = (uint32)head[3];            /* DATA packet length */

            TRACE(("UDP msg arrived:%d(0x%04x)\n",data_len,data_len));
            #ifdef __DEF_IINCHIP_DBG__
               printf("UDP msg arrived:%d(0x%04x)\r\n",data_len,data_len);
               printf("source Port : %d\r\n", *port);
               printf("source IP : %d.%d.%d.%d\r\n", addr[0], addr[1], addr[2], addr[3]);
            #endif

            /* data copy. */
            w5300_read_fifo(s, (uint16 *)buf, data_len);
            break;
         case Sn_MR_IPRAW :
            /* extract PACKET-INFO */
            w5300_read_fifo(s, head, 8);
            addr[0] = (uint8)(head[0] >> 8);       /* destination IP address */
            addr[1] = (uint8)head[0];
            addr[2] = (uint8)(head[1]>>8);
            addr[3] = (uint8)head[1];
            data_len = (uint32)head[2];            /* DATA packet length */

            #ifdef __DEF_IINCHIP_DBG__
               printf("IP RAW msg arrived\r\n");
               printf("source IP : %d.%d.%d.%d\r\n", addr[0], addr[1], addr[2], addr[3]);
            #endif

            /* data copy. */
            w5300_read_fifo(s, (uint16 *)buf, data_len);
            break;
         case Sn_MR_MACRAW :
            /* extract PACKET-INFO */
            w5300_read_fifo(s, head, 2);
            data_len = (uint32)head[0];            /* DATA packet length */
            w5300_read_fifo(s, (uint16 *)buf, data_len);
            w5300_read_fifo(s, crc, 4);
            break;
         default :
            break;
      }
      w5300_write_reg16(W5300_Sn_CR(s), Sn_CR_RECV);
   }
   #ifdef __DEF_IINCHIP_DBG__
      printf("recvfrom() end ..\r\n");
   #endif

   return data_len;
}
