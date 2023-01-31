#include <qdos.h>
#include "types.h"
#include "chan_ops.h"
#include "socket.h"
#include "debug.h"

// linefeed character
#define CHR_LF 0x0a

static inline short get_next_packet( int socknum, char *buf ) {
    register uint32 bytes_received = 0;
    bytes_received = socket_recv( (SOCKET)socknum, buf );
    if( bytes_received <= 0 ) {
        return -1;
    }
    tcp_pack_remain[socknum] = tcp_pack_size[socknum];
    /* TRACE(("PR: %d ", tcp_pack_remain[socknum])); */
    recv_buf_ptr[socknum] = buf;
    return tcp_pack_remain[socknum];
}

void pend( char *chanblk, int *status ) {
  register int i;
  i = ((qe_chandef_t *)chanblk)->socket_num;
  if( tcp_pack_remain[i] ){
    *status = ERR_OK;
    return;
  }
  if (bytes_available((SOCKET)i)) {
    *status = ERR_OK;
    return;
  }
  *status = ERR_NC; /* No pending input */
}

char fbyte( char *chanblk, int *error_code ) {
    register int i;
    register char c;
    register short status;

    i = ((qe_chandef_t *)chanblk)->socket_num;
    if( tcp_pack_remain[i] ) {
        tcp_pack_remain[i]--;
        c = *recv_buf_ptr[i]++;
        *error_code = ERR_OK;
        return c;
    }
    if( bytes_available( (SOCKET)i ) ) {
        status = get_next_packet(i, recv_buf[i]);
        if( status < 0 ) {
            *error_code = ERR_EF;
            return 0;
        }
        tcp_pack_remain[i]--;
        c = *recv_buf_ptr[i]++;
        *error_code = ERR_OK;
        return c;
    }
    if ( is_closed( (SOCKET)i )) {
        *error_code = ERR_EF;
        tcp_pack_remain[i] = 0;
        tcp_pack_size[i] = 0;
        recv_buf_ptr[i] = recv_buf[i];
        return 0;
    }
    // No bytes available + the socket is still open
    *error_code = ERR_NC;
    return 0;
}

int sstrg( char *chanblk, unsigned long timeout, int count, char **addr1 ) {
    static char *src;
    static int socknum;

    src = *addr1;
    socknum = ((qe_chandef_t *)chanblk)->socket_num;
    TRACE(( "(%d) : Write %d bytes.\n", socknum, count ));
    TRACE(("sstrg: s=%d,buf=%08x,len=%08x\n", socknum, src, count));
    count = socket_send( socknum, src, count );
    TRACE(( "(%d) : Sent %d bytes.\n", socknum, count ));
    *addr1 += count;
    return count;
}

uint16 fline( char *chanblk, unsigned long timeout, uint16 buf_len, char **h_buf, int *error_code ) {
    register char *buf = *h_buf;
    register char c = 0;
    register uint16 num_read = 0;
    register int i;

    i = ((qe_chandef_t *)chanblk)->socket_num;
    while ( CHR_LF != c && num_read < buf_len ) {
        if ( tcp_pack_remain[i] == 0 ) {
            if ( bytes_available( (SOCKET)i ) ) {
                if( get_next_packet( i, recv_buf[i] ) <= 0 ) {
                    *error_code = ERR_NC;
                    *h_buf = buf;
                    return num_read;
                }
            } else {
                // Can't read next packet - return appropriate error depending on socket state
                if ( is_closed( (SOCKET)i )) {
                    *error_code = ERR_EF;
                } else {
                    *error_code = ERR_NC;
                }
                *h_buf = buf;
                return num_read;
            }
        }
        tcp_pack_remain[i]--;
        num_read++;
        c = *recv_buf_ptr[i];
        TRACE(("%c", c));
        recv_buf_ptr[i]++;
        *buf++ = c;
    }
    if( num_read == buf_len && CHR_LF != c ) {
        *error_code = ERR_BO;
    } else {
        *error_code = ERR_OK;
    }
    *h_buf = buf;
    return num_read;
}

uint32 ip_recv( char *chanblk, unsigned long timeout, uint32 buf_len, char **h_buf, int *error_code ) {
    register char *buf = *h_buf;
    register char c = 0;
    register uint32 num_read = 0;
    register int i;

    i = ((qe_chandef_t *)chanblk)->socket_num;
    while ( num_read < buf_len ) {
        if ( tcp_pack_remain[i] == 0 ) {
            if ( bytes_available( (SOCKET)i ) ) {
                if( get_next_packet( i, recv_buf[i] ) <= 0 ) {
                    *error_code = ERR_OK;
                    *h_buf = buf;
                    return num_read == 0 ? -1 : num_read;
                }
            } else {
                // Can't read next packet - return appropriate error depending on socket state
                if ( is_closed( (SOCKET)i )) {
                    *error_code = ERR_EF;
                } else {
                    *error_code = ERR_OK;
                }
                *h_buf = buf;
                return num_read == 0 ? -1 : num_read;
            }
        }
        tcp_pack_remain[i]--;
        num_read++;
        c = *recv_buf_ptr[i];
        /* TRACE(("%c", c)); */
        recv_buf_ptr[i]++;
        *buf++ = c;
    }
    *error_code = ERR_OK;
    *h_buf = buf;
    return num_read;
}
