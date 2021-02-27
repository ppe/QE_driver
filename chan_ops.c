#include <qdos.h>
#include "types.h"
#include "chan_ops.h"

// linefeed character
#define CHR_LF 0x0a

char fbyte( char *chanblk, int *error_code ) {
    char c;
    char *read_ptr = *(char **)(chanblk + READ_PTR);
    char *end_ptr = *(char **)(chanblk + END_PTR);

    if( read_ptr == end_ptr ) {
        // End of buffer reached, no more characters to read
        *error_code = ERR_EF;
        return 0;
    }
    c = *read_ptr++;
    *(char **)(chanblk + READ_PTR) = read_ptr;
    *error_code = ERR_OK;
    return c;
}

int sstrg( char *chanblk, unsigned long timeout, int count, char **addr1 ) {
    char *dest = chanblk + BUF_START;
    char *src = *addr1;
    int i;
    for( i = 0; i < count && i < MAX_LEN; i++ ) {
        *dest++ = *src++;
    }
    // will be returned from IOSS to caller, points to one past last character written
    *addr1 = src;
    // Update channel variables
    // Reset read pointer to start of buffer
    *(char **)(chanblk + READ_PTR) = chanblk + BUF_START;
    // Set end pointer to one past last character written
    *(char **)(chanblk + END_PTR) = dest;
    return count;
}

uint16 fline( char *chanblk, unsigned long timeout, uint16 buf_len, char **h_buf, int *error_code ) {
    char *read_ptr = *(char **)(chanblk + READ_PTR);
    char *end_ptr = *(char **)(chanblk + END_PTR);
    char *buf = *h_buf;
    char c = 0;
    uint16 num_read = 0;

    if( read_ptr == end_ptr ) {
        // End of buffer reached, no more characters to read
        *error_code = ERR_EF;
        return 0;
    }
    while( read_ptr <= end_ptr && num_read < buf_len ) {
        num_read++;
        c = *read_ptr++;
        *buf++ = c;
        if( CHR_LF == c ) { break; }
    }

    *(char **)(chanblk + READ_PTR) = read_ptr - 1; // Adjust for final increment in while loop
    *h_buf = buf;
    *error_code = ERR_OK;
    return num_read;
}
