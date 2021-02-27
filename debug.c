#include <stdarg.h>
#include <stdio.h>
#include <qdos.h>
#include "debug.h"

char membuf[DEBUG_BUF_SIZE];
char *write_ptr = membuf;
char *guard;

unsigned int get_buf_base() {
    guard = write_ptr+DEBUG_BUF_SIZE-100;
    return (unsigned int)&membuf;
}

inline void dbg_printf(const char *fmt, ...)
{
    static int len;
    static va_list args;
    va_start(args, fmt);
#ifdef QE_DEBUG_BUF
    len = vsprintf(write_ptr, fmt, args);
    write_ptr += len;
    if(write_ptr > guard) {
        write_ptr = membuf;
    }
#else
    /* vfprintf(stderr, fmt, args); */
    vsprintf(membuf,fmt,args);

    (void)io_sstrg( (chanid_t)0, (timeout_t)0, membuf, strlen(membuf) );
#endif
    va_end(args);
}
