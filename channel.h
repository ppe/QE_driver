#ifndef	_CHANNEL_H_
#define	_CHANNEL_H_

#include <qdos.h>

/* TCP and UDP correspond to Sn_MR_TCP and Sb_MR_UDP defined in w5300_h */
typedef enum e_sock_type {SCK, TCP, UDP} sock_type;

typedef struct ip_peers {
	sock_type type;
	unsigned long ip;
	unsigned short port;
} ip_peer;

/*
IOSS channel_open
returns: reference number to use in subsequent calls to channel_close and channel_io 
            negative on error, 0-7 == success
*/
extern int channel_open( const QLSTR_t * );
/* param: reference number (= =physical W5300 socket number) */
extern int channel_close(unsigned int);
extern int channel_io_sstrg( unsigned int, unsigned int, char * );
extern int channel_io_fstrg( int, unsigned int, char * );
#endif