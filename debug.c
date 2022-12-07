#include <stdarg.h>
#include <stdio.h>
#include <qdos.h>
#include "debug.h"

char membuf[DEBUG_BUF_SIZE];

inline void dbg_printf(const char *fmt, ...)
{
    static int len;
    static va_list args;
    va_start(args, fmt);
    /* vfprintf(stderr, fmt, args); */
    vsprintf(membuf,fmt,args);

    (void)io_sstrg( (chanid_t)0, (timeout_t)0, membuf, strlen(membuf) );
    va_end(args);
}
