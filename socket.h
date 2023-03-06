#ifndef	_SOCKET_H_
#define	_SOCKET_H_

/**
 * \file    socket.h
 * WIZnet SOCKET API function definition
 *
 * For user application, WIZnet provides SOCKET API functions 
 * which are similar to Berkeley SOCKET API.\n
 *
 * \author MidnightCow
 * \date 04/07/2008
 * \version 1.1.1
 *
 * Modify the warning-block code in recv(). Refer to M_15052008.
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
 */
#include <stdlib.h>

#include "types.h"

/**********************************
 * define function of SOCKET APIs * 
 **********************************/

/*
Linux socket compatibility recv flags
*/
#define	MSG_OOB		0x1		/* process out-of-band data */
#define	MSG_PEEK	0x2		/* peek at incoming message */
#define	MSG_DONTROUTE	0x4		/* send without using routing tables */
#define	MSG_EOR		0x8		/* data completes record */
#define	MSG_TRUNC	0x10		/* data discarded before delivery */
#define	MSG_CTRUNC	0x20		/* control data lost before delivery */
#define	MSG_WAITALL	0x40		/* wait for full request or error */

#define TCP_MAX_MTU 1500
#define MAX_WAIT_CYCLES 100
#define MAX_SOCK_NUM 8

extern char *recv_buf[];
extern int tcp_pack_remain[];
extern int tcp_pack_size[];
extern char *recv_buf_ptr[];
extern uint16 curr_byte[];

struct peek_cache_entry {
	size_t bytes_available;
	void *read_position;
	void *buffer;
};

#ifndef min
#define min(x,y) ((x) > (y) ? (y) : (x))
#endif


/**
 * Open a SOCKET.
 */ 
uint8    open_socket(SOCKET s, uint8 protocol, uint16 port, uint16 flag);
uint8 is_closed( SOCKET s );

uint16 bytes_available( SOCKET s );
void socket_drain(SOCKET s);
/**
 * Close a SOCKET.
 */ 
void     socket_close(SOCKET s);                                                           /* Close socket */

/**
 * It tries to connect a client.
 */
uint8    connect(SOCKET s, uint8 * addr, uint16 port);

/**
 * It tries to disconnect a connection SOCKET with a peer.
 */
void     disconnect(SOCKET s); 

/**
 * It is listening to a connect-request from a client.
 */
uint8    listen(SOCKET s);	    

/**
 * It sends TCP data on a connection SOCKET
 */
int16   socket_send(int s, uint8 * buf, uint16 len);

/**
 * It receives TCP data on a connection SOCKET
 */
/* int socket_recv (int s, void *b, uint16 ln, unsigned int flag); */
int32 socket_recv(SOCKET sock, char* buf );
uint32   recv_w5300(SOCKET s, uint8 * buf, uint32 len);

/**
 * It sends UDP, IPRAW, or MACRAW data 
 */
uint32   sendto(SOCKET s, uint8 * buf, uint32 len, uint8 * addr, uint16 port); 

/**
 * It receives UDP, IPRAW, or MACRAW data
 */
uint32   recvfrom(SOCKET s, uint8 * buf, uint32 len, uint8 * addr, uint16  *port);

#endif
/* _SOCKET_H_ */
