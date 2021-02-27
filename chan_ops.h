#ifndef _DRV_TMPL_CHAN_OPS_H
#define _DRV_TMPL_CHAN_OPS_H

#include "types.h"

// Offsets to channel block to store info that should be persisted across IO calls
#define READ_PTR 0x18
#define END_PTR 0x1C
#define BUF_START 0x20
// Maximum length of characters that this driver can accept in a SSTRG
#define MAX_LEN 0xE0

extern char fbyte( char *, int * );
extern int sstrg( char *, unsigned long, int, char ** );
extern uint16 fline( char *, unsigned long , uint16, char **, int * );

#endif