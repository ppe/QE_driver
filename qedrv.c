#include <qdos.h>
#include <string.h>
#include "types.h"
#include "resolv.h"
#include "chan_ops.h"
#include "heap.h"
#include "socket.h"
#include "debug.h"
#include "w5300.h"

#define PRINTB0( ch ) (void)io_sbyte( (chanid_t)0, (timeout_t)0, ch )
#define PRINT0( msg ) (void)io_sstrg( (chanid_t)0, (timeout_t)0, msg, strlen(msg))

// Total size of channel block
#define CHAN_BLOCK_SIZE 0x100

// QDOS IO Sub System operation codes
#define IO_PEND 0
#define IO_FBYTE 1
#define IO_FLINE 2
#define IO_FSTRG 3
#define IO_SSTRG 7
#define SD_CHENQ 0x0b
#define IP_RECV  0x53

// IO_OPEN open types passed in D3.L
#define TYPE_OPEN 0 // No host/port required
#define TYPE_OPEN_IN 1 // Host/port must be specified, used for outgoing connections
#define TYPE_OPEN_NEW 2 // Host/port must be specified, used for incoming connections

// Print a msg that is a QLSTR_t to channel 0
//#define PRINT0( msg ) (void)io_sstrg( (chanid_t)0, (timeout_t)0, msg.qs_str, msg.qs_strlen )

/* TCP and UDP correspond to Sn_MR_TCP and Sb_MR_UDP defined in w5300_h */
typedef enum e_sock_type {SCK, TCP, UDP} sock_type;

typedef struct ip_peers {
	sock_type type;
	unsigned long ip;
	unsigned short port;
} ip_peer;

/* 4 bytes of ASCII in uppercase */
const uint32 SCK_=0x53434b5f, TCP_=0x5443505f, UDP_=0x5544505f;
ip_peer * peers[8] = {NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};


// This driver's QDOS driver linkage block
QLD_LINK_t linkblk;

static QLSTR_INIT(err_memcfg,"Memory configuration error, can't initialize!\n");
static QLSTR_INIT(msg_configured, "W5300 network card configured.\n");
static QLSTR_INIT(msg_wait_begin, "Reset wait begin...");
static QLSTR_INIT(msg_wait_end, "end\n");

static int configure_W5300() {
    // Writing a 0 to the HW reset address resets the W5300
    *hw_reset_addr = 0;
    PRINT0("Reset begin...");
    wait_10ms(1); /* Wait 200ms for PLL to synchronize */
    PRINT0("end.\n");
    if(!sysinit(tx_mem_conf,rx_mem_conf))
    {
        PRINT0(&err_memcfg);
        return -1;
    }
    setSHAR(mac);                                      /* set source hardware address */
    setGAR(gw);                                     /* set gateway IP address */
    setSUBR(sn);                                    /* set subnet mask address */
    setSIPR(ip);                                    /* set source IP address */
    PRINT0(&msg_configured);
    return 0;
}

int parse_channel_def( const QLSTR_t *name, ip_peer *peer, unsigned short socket_num ) {
    static unsigned short namelen;

    static char *chan_def, *alloc_address;
    static char address[256],rest[2];
    static unsigned int port, a3,a2,a1,a0;
    static int num_read = 0, n = 0;
    static unsigned long name_prefix;

    rest[0] = rest[1] = 0;
    namelen = name->qs_strlen;
    /* TRACE(("Namelen: %d\n", namelen)); */
    name_prefix = *( (unsigned long *)name->qs_str ) & 0xdfdfdfff;
    if( SCK_==name_prefix ) { peer->type = SCK; }
    else if ( TCP_ == name_prefix ) { peer->type = TCP; }
    else if ( UDP_ == name_prefix ) { peer->type = UDP; }
    if ( 4 == namelen ) {
        TRACE(("no peer address info\n"));
        return 0;
    }
    /* parse address and port */
    if( !(alloc_address = chan_def = sv_memalloc( namelen + 1)))  {
        return -1;
    }
    /* TRACE(("alloc at %08x\n", alloc_address)); */
    strncpy( chan_def, name->qs_str+4, namelen-4 );
    chan_def[namelen-4] = 0;
    /* TRACE(("Chandef: '%s'\n",chan_def)); */
    num_read = sscanf( chan_def, "%255[^:]:%u%1s%n", address, &port, rest, &n );
    /* TRACE(("addr=%s\n",address)); */
    /* TRACE(("n=%d\n",n)); */
    /* TRACE(("num_read=%d\n",num_read)); */

    if( 2 != num_read ) {
        /* PRINT0(chan_def); */
        /* PRINTB0('/'); */
        /* ut_mint((chanid_t)0,num_read); */
        /* PRINTB0('x'); */
        sv_memfree( alloc_address );
        return ERR_BP;
        /* Malformed host:port string */
    }
    peer->port = port;

    num_read = sscanf( address, "%u.%u.%u.%u",&a3,&a2,&a1,&a0 );
    if ( 4 == num_read && (!(a3 & 0xffffff00)) && (!(a2 & 0xffffff00)) && (!(a1 & 0xffffff00)) && (!(a0 & 0xffffff00))) {
        /* Valid ip address */
        TRACE(("valid ip address %u.%u.%u.%u\n",a3,a2,a1,a0));
        peer->ip = a3<<24 | a2<<16 | a1<<8 | a0;
        TRACE(("Stored ip %08x at peer %08x\n", peer->ip, peer ));
    } else {
        struct hostent *hostent;
        /* TRACE(("Resolve address %s\n", address)); */
        hostent = gethostbyname_impl( address, (chanid_t)socket_num);
        if( NULL != hostent && NULL != (unsigned int *)hostent->h_addr ) {
            peer->ip = *((unsigned int *)hostent->h_addr);
        }
        /* resolve address */
    }
    /* TRACE(("Free %08x\n", alloc_address)); */
    sv_memfree( alloc_address );
    return 0;
}

long ch_open() {
    register QLSTR_t *name asm( "a0" );
    register unsigned long open_type asm( "d3" );
    static QLSTR_t *name_store;
    static unsigned long open_type_store;
    static qe_chandef_t *channel_block;

    static unsigned short namelen;
    static unsigned long name_prefix;
    static long size = sizeof(ip_peer);
    static int err;
    static int i;
    static ip_peer *peer;
    static char *tmp;

    name_store = name;
    open_type_store = open_type;
    size = sizeof(ip_peer);
    namelen = name_store->qs_strlen;
    if( namelen < 4 ) {
        /* The name is too short - it can't be this driver */
        TRACE(( "Name too short\n" ));
        /* asm(" move.l %0,a0" : : "m" (channel_block)); */
        // a0 must be returned unmodified if open is unsuccesful
        asm( " move.l %0,a0" : : "m" (name_store));
        return ERR_NF;
    }
    /* uppercase the first three characters of the channel name */
    name_prefix = *( (unsigned long *)name_store->qs_str ) & 0xdfdfdfff;
    /* TRACE(( "Prefix=%08x\n",name_prefix )); */
    if( !( SCK_ == name_prefix ) && !( TCP_ == name_prefix ) && !( UDP_ == name_prefix )) {
        /* Wrong prefix for channels this driver supports */
        /* TRACE(( "Wrong prefix\n" )); */
        // a0 must be returned unmodified if open is unsuccesful
        asm( " move.l %0,a0" : : "m" (name_store));

        // Not Found == Name does not match this driver
        return ERR_NF;
    }
    // $18 bytes for header + whatever is required by implementation
    // http://www.qdosmsq.dunbar-it.co.uk/doku.php?id=qdosmsq:sbinternal:chandef&s[]=channel&s[]=definition&s[]=block
    if( !(channel_block = (qe_chandef_t *)sv_memalloc( CHAN_BLOCK_SIZE ))) {
        // Out of Memory
        return ERR_OM;
    }
    for ( i = 7; i > -1; i-- ) {
        if( NULL ==  peers[i] ) {
            channel_block->socket_num = i;
            break;
        }
    }
    if( -1 == i ) {
        // All sockets were reserved
        sv_memfree( channel_block );
        asm( " move.l %0,a0" : : "m" (name_store));
        return ERR_IU;
    }
    peer = sv_memalloc( sizeof( ip_peer ) );
    if( !peer ) { sv_memfree( channel_block ); return ERR_OM; }
    peer->ip=0;peer->port=0;
    err = parse_channel_def( name_store, peer, i );
    if( err ) {
        TRACE(("error parsing channel def %d\n", err));
        sv_memfree( (char *)peer );
        sv_memfree( channel_block );
        asm( " move.l %0,a0" : : "m" (name_store));
        return err;
    }
    peers[i] = peer;
    TRACE(("recv_buf[%d]=%08x, ", i, recv_buf[i]));
    if ( ! recv_buf[i] ) {
        tmp = sv_memalloc( TCP_MAX_MTU );
        /* TODO: sv_memalloc error checking */
        TRACE((" alloc buf at %08x\n", tmp));
        recv_buf[i] = tmp;
    } else {
        TRACE(("NO alloc\n"));
    }
    recv_buf_ptr[i] = recv_buf[i];
    tcp_pack_remain[i] = 0;
    tcp_pack_size[i] = 0;
    TRACE(( "Allocated peer struct at %08x, ref = %u\n", peer, i ));
    TRACE(( "Type: %u, IP: %08x, port %u\n", peer->type, peer->ip, peer->port ));
    if( SCK != peer->type ) {
        open_socket( (SOCKET)i, peer->type, peer->port, 0 );
        if( TCP == peer->type && 0 != peer->ip && 0 != peer->port ) {
            connect( (SOCKET)i, (uint8 *)&(peer->ip), peer->port );
        }
    }
    // Address of channel definition block must be returned in A0
    asm(" move.l %0,a0" : : "m" (channel_block));
    return ERR_OK;
}

static long ch_close() {
    register qe_chandef_t *chan_blk asm( "a0" );
    register int socknum;
    register uint32 available;
    qe_chandef_t *chan_blk_store = chan_blk;

    socknum = chan_blk_store->socket_num;
    /* TRACE(("Releasing socket %d : disconnect, ", socknum)); */
    disconnect( socknum );
    /* TRACE(("close, ", socknum)); */
    socket_close( (SOCKET)socknum );
    socket_drain( (SOCKET)socknum );
    /* TRACE(("free peer, ", socknum)); */
    sv_memfree( (char *)(peers[socknum]) );
    /* TRACE(("free channel block, ", socknum)); */
    sv_memfree( (char *)chan_blk_store );
    /* TRACE(("erase peer entry...", socknum)); */
    peers[socknum] = NULL;
    if ( recv_buf[socknum] ) {
        TRACE(("Free socket %d recv buf\n",socknum));
        sv_memfree( recv_buf[socknum]); }
    recv_buf[socknum] = NULL;
    recv_buf_ptr[socknum] = NULL;
    tcp_pack_remain[socknum] = 0;
    tcp_pack_size[socknum] = 0;
    /* TRACE(("done.\n", socknum)); */
    available = bytes_available( socknum );
    /* W5300 internal buffer gets cleared and available bytes reset when socket is re-opened */
    TRACE(( "Socket %d close, bytes remaining: %d\n", socknum, available ));
    return ERR_OK;
}

long ch_io() {
    register unsigned char in_optype asm( "d0" );
    register unsigned long in_param1 asm( "d1" );
    register unsigned long in_param2 asm( "d2" );
    register unsigned long in_timeout asm( "d3" );
    register char *in_chanblk asm( "a0" );
    register char *in_addr1 asm( "a1" );
    unsigned char optype = in_optype;
    unsigned long param1 = in_param1;
    unsigned long param2 = in_param2;
    unsigned long timeout = in_timeout;
    char *chanblk = in_chanblk;
    char *addr1 = in_addr1;


    switch( optype ) {
    case IO_PEND:
    {
      int status = ERR_NC; /* ERR_NC = no pending input, ERR_OK = input pending */
      pend( chanblk, &status );
      {
      asm(" move.l %0,a0
            move.l %1,d3
          " : : "m"(chanblk),
                "m"(timeout));
      return status;
      }
    }
    case IO_FBYTE:
    {
        char c;
        int status = ERR_OK;

        c = fbyte( chanblk, &status );
        asm( " move.l %0,a0
                       move.b %1,d1
                     " : : "m" (chanblk),
             "m" (c) );
        return status;
    }
    break;
    case IO_SSTRG:
    {
        int bytes_sent;

        /*
          param2 = number of bytes to write, size == word
          addr1 = pointer to start of sequence of bytes to write
          passed as a handle so that implementation can update addr1
          in accordance with bytes written, see return parameters below
        */
        bytes_sent = sstrg( chanblk, timeout, (int)( param2 & 0x0000FFFF ), &addr1 );
        /*
          The following must be returned:
          a0 = pointer to driver linkage block passed in when this function called
          a1 = pointer to one byte past the last byte that was written
          d1 = number of bytes written
        */
        asm( " move.l %0,a0
                       move.l %1,a1
                       move.l %2,d1
                     " : : "m" (chanblk),
             "m" (addr1),
             "m" (bytes_sent) );
        return ERR_OK;
    }
            break;
    case IO_FLINE:
    {
        uint16 bytes_read = 0;
        int error_code = ERR_OK;

        bytes_read = fline( chanblk, timeout, (uint16)( param2 & 0x0000FFFF ), &addr1, &error_code );
        asm( " move.l %0,a0
                       move.l %1,a1
                       move.l %2,d1
                     " : : "m" (chanblk),
             "m" (addr1),
             "m" (bytes_read) );
        return error_code;

    }
    break;
    case SD_CHENQ:
    {
        asm( " move.l %0,a0" : : "m" (chanblk) );
        return ERR_BP;
    }
    case IP_RECV: {
        uint32 bytes_read = 0;
        int error_code = ERR_OK;
        bytes_read = ip_recv( chanblk, timeout, param2, &addr1, &error_code );
        asm( " move.l %0,a0
                       move.l %1,a1
                       move.l %2,d1
                     " : : "m" (chanblk),
             "m" (addr1),
             "m" (bytes_read) );
        return error_code;
    }
    }
    asm( " move.l %0,a0" : : "m" (chanblk) );
    return ERR_OK;
}

int main( int ac, char **av ) {
    register short i;
    /* char foo[256]; */
    /* QLSTR_t *puup; */
    /* QLSTR_DEF(a,256); */
    /* QLSTR_INIT( msg_init, "Initializing QE driver..." ); */
    /* QLSTR_INIT( msg_done, "done.\n" ); */
    /* char *base = (char *)get_buf_base(); */

    /* *base = 0; */
    /* sprintf(foo, "Trace buffer at %08x\n", get_buf_base()); */
    /* puup = cstr_to_ql(&a, foo); */
    /* PRINT0( a ); */
    /* PRINT0( msg_init ); */
    PRINT0("QE Driver for W5300\n");
    TRACE(("Linking QE driver._-O-_.\n"));
    // linkblk.ld_next will be populate by QDOS when driver is linked
    linkblk.ld_io = ch_io;
    linkblk.ld_open = ch_open;
    linkblk.ld_close = ch_close;
    mt_liod( &linkblk );
    for ( i = MAX_SOCK_NUM - 1; i>= 0; i--) {
        recv_buf[i] = NULL;
        tcp_pack_remain[i] = 0;
        tcp_pack_size[i] = 0;
        recv_buf_ptr[i] = NULL;
    }
    configure_W5300();
    /* PRINT0( msg_done ); */
}
