#ifndef _DRV_TMPL_HEAP_H
#define _DRV_TMPL_HEAP_H

extern char * sv_memalloc( int );
extern void sv_memfree( char * );
extern int get_alloc_balance( void );

#endif
