#ifndef	_DEBUG_H_
#define	_DEBUG_H_

#define QE_DEBUG 1
/* #define QE_DEBUG_BUF 1 */
#define DEBUG_BUF_SIZE 10000

#define TRACE(x) do { if (QE_DEBUG) dbg_printf x; } while (0)
#define PRINTB0( ch ) (void)io_sbyte( (chanid_t)0, (timeout_t)0, ch )
#define PRINT0( msg ) (void)io_sstrg( (chanid_t)0, (timeout_t)0, msg, strlen(msg))

extern unsigned int get_buf_base(void);
extern inline void dbg_printf(const char *fmt, ...);
#endif
