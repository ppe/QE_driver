#include <stdio.h>
#include <qdos.h>

#define PRINTB0( ch ) (void)io_sbyte( (chanid_t)0, (timeout_t)0, ch )

QLD_LINK_t linkblk;

long ch_open() {
    register QLSTR_t *name asm( "a0" );
    register long ret asm("d0");
    static void * name_store;

    /* static QLSTR_t *name_store; */
    name_store = name;
    /* PRINTB0('x'); */
    asm( " move.l %0,a0" : : "m" (name_store));
    return ERR_NF;
}

static long ch_close() {
    return ERR_OK;
}

long ch_io() {
    register char *in_chanblk asm( "a0" );
    char *chanblk = in_chanblk;

    asm( " move.l %0,a0" : : "m" (chanblk) );
    return ERR_OK;
}

int main(int ac, char **av) {
    int num_read = 0;
    char *chan_def = "www.hut.com:80";
    char address[256],rest[2];
    int n=4;
    unsigned int port;

    rest[0] = rest[1] = 0;
    num_read = sscanf( chan_def, "%255[^:]:%u%1s%n", address, &port,rest,  &n );
    printf("Address: %s\n",address);
    printf("Rest: %s\n",rest);
    printf("Port: %u\n", port);
    printf("n: %d\n", n);
    printf("num_read: %d\n", num_read);
    // linkblk.ld_next will be populate by QDOS when driver is linked
    linkblk.ld_io = ch_io;
    linkblk.ld_open = ch_open;
    linkblk.ld_close = ch_close;
    mt_liod( &linkblk );
    /* link_driver(); */
}
