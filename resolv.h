#include <sys/types.h>
#include <netdb.h>
#include "socket.h"


/*Type field of Query and Answer */
#define T_A 1 /* host address */
#define T_NS 2 /* authoritative server */
#define T_CNAME 5 /* canonical name */
#define T_SOA 6 /* start of authority zone */
#define T_PTR 12 /* domain name pointer */
#define T_MX 15 /* mail routing information */
#define UDP_MSG_BUF_SIZE 2048 /* UDP message buffer size */
/* Size of initial memory allocation for hostent static buffer size (per channel) */
#define HOSTENT_CONTENT_BUF_INITIAL_SIZE 256
#define CLASS_IN 1

/*Function Declarations */
extern uint32 ngethostbyname (char*);
extern struct hostent *gethostbyname_impl(const char *, const chanid_t);
extern void close_channel(const chanid_t);

struct channel_hostent {
	void *hostent_content_buf;
	uint16 content_buf_size;
	struct hostent *hostent;
};

struct hostent_list_node {
	struct channel_hostent *content;
	chanid_t channel;
	struct hostent_list_node *next;
};

struct dns_cache_entry {
	unsigned long address;
	unsigned long refcount;
	char * hostname;
};

struct dns_reply_info {
	unsigned short num_cnames;
	unsigned short num_as;
	unsigned short a_name_max_len;
	unsigned short malloc_size;
	unsigned char *reply;
	char *answer_start;
};

/*DNS header structure */
struct DNS_HEADER
{
  int id:16; /* identification number */
    int qr:1; /* query/response flag */
    int opcode:4; /* purpose of message */
    int aa:1; /* authoritive answer */
    int tc:1; /* truncated message */
    int rd:1; /* recursion desired */
    int ra:1; /* recursion available */
    int z:3; /* its z! reserved */
    int rcode:4; /* response code */
	unsigned short q_count; /* number of question entries */
	unsigned short ans_count; /* number of answer entries */
	unsigned short ns_count; /* number of authority entries */
	unsigned short ar_count; /* number of resource entries */
};

/*Constant sized fields of query structure */
struct QUESTION
{
	unsigned char qtype_h;
	unsigned char qtype_l;
	unsigned char qclass_h;
	unsigned char qclass_l;
};

/*Constant sized fields of the resource record structure */
struct R_DATA
{
	unsigned short type;
	unsigned short _class;
	unsigned int ttl;
	unsigned short data_len;
};

/*Pointers to resource record contents */
struct RES_RECORD
{
	char *name;
	struct R_DATA *resource;
	char *rdata;
};

/*Structure of a Query */
typedef struct
{
	char *name;
	struct QUESTION *ques;
} QUERY;
