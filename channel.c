#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "channel.h"
#include "resolv.h"
#include "debug.h"
#include "heap.h"

/* 4 byte ASCII in uppercase */
const unsigned long SCK_=0x53434b5f, TCP_=0x5443505f, UDP_=0x5544505f;
ip_peer * peers[8] = {NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};

ip_peer * ip_test;

int parse_channel_def( const QLSTR_t *name, ip_peer *peer, unsigned short socket_num ) {
	const unsigned short namelen = name->qs_strlen;

	char *chan_def, *alloc_address;
	char address[256],rest[1];
	unsigned int port, a3,a2,a1,a0;
	int num_read = 0, n = 0;
	unsigned long name_prefix;
	if( namelen < 4 ) {
		/* If the name is too short it can't be this driver */
		TRACE(("Name too short\n"));
		return -1;
	}
	/* uppercase the first three characters of the channel name */
	name_prefix = *( (unsigned long *)name->qs_str ) & 0xdfdfdfff;
	if( !(SCK_==name_prefix) && !(TCP_==name_prefix) && !(UDP_==name_prefix)) {
		/* Wrond prefix for channels this driver supports */
		TRACE(("Wrong prefix\n"));
		return -1;
	}
	if( SCK_==name_prefix ) { peer->type = SCK; }
	else if ( TCP_ == name_prefix ) { peer->type = TCP; }
	else if ( UDP_ == name_prefix ) { peer->type = UDP; }
	if ( 4 == namelen ) {
		TRACE(("No peer address info\n"));
		return 0;
	}
	/* Parse address and port */
	if( !(alloc_address = chan_def = sv_memalloc( namelen + 1)))  {
		return -1;
	}
	TRACE(("Alloc at %08x\n", alloc_address));
	strncpy( chan_def, name->qs_str+4, namelen );
	chan_def[namelen] = 0;
	num_read = sscanf( chan_def, "%255[^:]:%u%1s%n", address, &port, rest, &n );
	TRACE(("addr=%s\n",address));
	TRACE(("n=%d\n",n));
	TRACE(("num_read=%d\n",num_read));

	if( 2 != num_read ) {
		sv_memfree( alloc_address );
		return -2;
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
		TRACE(("Resolve address %s\n", address));
		hostent = gethostbyname_impl( address, (chanid_t)socket_num);
		if( NULL != hostent && NULL != (unsigned int *)hostent->h_addr ) {
			peer->ip = *((unsigned int *)hostent->h_addr);
		}
		/* resolve address */
	}
	TRACE(("Free %08x\n", alloc_address));
	sv_memfree( alloc_address );
	return 0;
}

int channel_open( const QLSTR_t *name ) {
	const unsigned short namelen = name->qs_strlen;
	unsigned long name_prefix;
	long size = sizeof(ip_peer);
	int err;
	int i;
	ip_peer *peer;

/*
	ip_test = (ip_peer *)mm_alchp (&size);
	if(size!=sizeof(ip_peer)) {return -3;}
	return 0;
*/
	if( namelen < 4 ) {
		/* The name is too short - it can't be this driver */
		TRACE(( "Name too short\n" ));
		return -1;
	}
	/* uppercase the first three characters of the channel name */
	name_prefix = *( (unsigned long *)name->qs_str ) & 0xdfdfdfff;
	TRACE(( "Prefix=%08x\n",name_prefix ));
	if( !( SCK_ == name_prefix ) && !( TCP_ == name_prefix ) && !( UDP_ == name_prefix )) {
		/* Wrong prefix for channels this driver supports */
		TRACE(( "Wrong prefix\n" ));
		return -1;
	}
	for ( i = 0; i < 8; i++ ) {
		if( NULL ==  peers[i] ) {
			long psize = sizeof( ip_peer );
			TRACE(("before memalloc i=%u\n",i));
			peer = sv_memalloc( sizeof( ip_peer ) );
			/*
			peer = mm_alchp(psize);
			peer = malloc( sizeof( ip_peer ) );
			*/
			TRACE(("after memalloc peer=%08x, psize=%d, i=%u\n",peer, psize, i));
			if( !peer ) { return ERR_OM; }
			peer->ip=0;peer->port=0;
			err = parse_channel_def( name, peer, i );
			if( err ) {
				TRACE(("error parsing channel def %d\n", err));
				sv_memfree( (char *)peer );
				return err;
			}
			peers[i] = peer;
			TRACE(( "Allocated peer struct at %08x, ref = %u\n", peer, i ));
			TRACE(( "Type: %u, IP: %08x, port %u\n", peer->type, peer->ip, peer->port ));
			if( SCK != peer->type ) {
				open_socket( (SOCKET)i, peer->type, peer->port, 0 );
				if( TCP == peer->type && 0 != peer->ip && 0 != peer->port ) {
					connect( (SOCKET)i, (uint8 *)&(peer->ip), peer->port );
				}
			}
			return i;
		}
	}
}

int channel_close( unsigned int channel_ref ) {
	ip_peer *peer = NULL;

	/*
	mm_rechp(ip_test);
	return 0;
	*/
	if ( channel_ref > 7 ) {
		return 0;
	}

	peer = peers[ channel_ref ];

	TRACE(( "Close channel %u\n", channel_ref ));
	if( peer ) {
		if( TCP == peer->type ) {
			TRACE(( "Disconnect TCP socket\n" ));
			disconnect( (SOCKET)channel_ref );
		}
		if( SCK != peer->type ) {
			TRACE(( "Close socket\n" ));
			socket_close((SOCKET)channel_ref);
		}
		TRACE(( "Free peer def at %08x\n", peer ));
		sv_memfree( (char *)peer );
		peers[ channel_ref ] = NULL;
	} else {
		TRACE(( "No peer for channel ref %u\n", channel_ref ));
	}
	return 0;
}

int channel_io_sstrg( unsigned int channel_ref, unsigned int bytes_to_send, char *buffer ) {
	int sent = 0;
	sent = (int)socket_send( (SOCKET)channel_ref, (uint8 *)buffer, (uint32)bytes_to_send );
	TRACE(( "%u : Sent %d of %u bytes\n", channel_ref, sent, bytes_to_send ));
	return sent;
}

int channel_io_fstrg( int channel_ref, unsigned int buflen, char *buffer ) {
	return socket_recv (channel_ref, (void *)buffer, buflen, 0 /*flags*/);
}
