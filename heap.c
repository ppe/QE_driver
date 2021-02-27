#include "debug.h"
int alloc_balance = 0;

char * sv_memalloc( int size ) {
    char *result;

    alloc_balance++;
    TRACE(("Alloc %d bytes, balance %d ... ", size, alloc_balance));
    asm( " move.l  a6,-(a7)
           moveq   #0,d0
           trap    #1
           movea.l a0,a6
           move.l  %1,d1
           suba.l  a2,a2
           movea.w $C0,a2
           jsr     (a2)
           move.l  a0,d0
           move.l   d0,%0
           move.l  (a7)+,a6
         " : "=d" (result)
           : "d" (size)
           : "d0","d1","d2","d3","a0","a1","a2","a3"
        );
    TRACE(("at %08x\n", result));
    return result;
}

void sv_memfree( char * address ) {
    alloc_balance--;
    TRACE(("Free block at %08x, alloc balance %d\n", address, alloc_balance));
    asm( " move.l  a6,-(a7)
           moveq   #0,d0
           trap    #1
           movea.l a0,a6
           move.l  %0,a0
           suba.l  a2,a2
           movea.w $C2,a2
           jsr     (a2)
           move.l  (a7)+,a6
         " : 
           : "d" (address)
           : "d0","d1","d2","d3","a0","a1","a2","a3"

    );
}

int get_alloc_balance() {
    return alloc_balance;
}
