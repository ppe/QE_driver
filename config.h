/* Configured Items - Config Level 1
 *
 * Name     Description
 * STRA0    IP address of card
 */

static unsigned short conf1a[] = {
12328,2,16944,4,28672,20085,
15420, 20803, 18008, 15934, 12337, 3, 28784, 25856, 6, 
12590, 12337, 8224,         /* Vers 1.01    */
0,36,0,65494,6,24,65535,18,18768,8289,25700,29285,29555,8303,26144,25441,
29284,0 };

static unsigned char conf1b[] = {
0,2,0,16,0,8,49,48,46,48,46,48,46,49,0,0,
0,0,0,0,0,0,0 };

static unsigned char *STRA0 = conf1b+6;
