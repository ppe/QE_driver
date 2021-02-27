#include "debug.h"
#include "resolv.h"
#include "in.h"
#include "heap.h"

#include <netdb.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <qdos.h>

/* Adapted from http://www.binarytides.com/blog/dns-query-code-in-c-with-winsock/ */
#define CACHED_ENTRIES 10
static struct dns_cache_entry entry_cache[CACHED_ENTRIES];
static int initialized = 0;
static unsigned long cache_refcount;

/* Linked list of hostent storage for channels */
static struct hostent_list_node *head = NULL;
/* hostent result structures for each channel */
const static size_t new_channel_alloc_size =
        sizeof(struct hostent_list_node) +
        sizeof(struct channel_hostent) + 
        sizeof(struct hostent);

unsigned short readWord(char *addr) {
    unsigned short val = 0;
    uint8 byte=0;
    byte = *addr;
    val = byte << 8;
    byte = *(addr + 1);
    val += byte;
    return val;
}

void initialize_cache() {
    int i;
    #ifdef __DEF_IINCHIP_DBG__
        printf("Initializing resolver cache.\n");
    #endif

    for (i = 0; i < CACHED_ENTRIES; i++) {
        entry_cache[i].hostname = NULL;
        entry_cache[i].address = 0;
        entry_cache[i].refcount = 0;
    }
}

unsigned long get_from_cache( char * hostname ) {
    int i;

	(void)io_sstrg((chanid_t)0,(timeout_t)0,"Find: ",6);
	(void)io_sstrg((chanid_t)0,(timeout_t)0,hostname,strlen(hostname));
	(void)io_sstrg((chanid_t)0,(timeout_t)0,"\n",1);

	
    if(NULL == hostname) {
        return 0;
    }
    #ifdef __DEF_IINCHIP_DBG__
        printf("Lookup '%s' in cache.\n", hostname);
    #endif

    for(i = 0;i < CACHED_ENTRIES; i++) {
        char *cached_name = entry_cache[i].hostname;

        if( NULL != cached_name && !strcmp(hostname, cached_name )) {
            entry_cache[i].refcount = ++cache_refcount;
            #ifdef __DEF_IINCHIP_DBG__
                printf("Found address %x for '%s' in cache.\n", entry_cache[i].address, hostname);
                printf("Set refcount for index %d to %d.\n", i, cache_refcount);
            #endif
            return entry_cache[i].address;
        }
    }
    #ifdef __DEF_IINCHIP_DBG__
        printf("Address for '%s' NOT in cache.\n", hostname);
    #endif
    return 0;
}

void set_cache_entry( int i, char *hostname, unsigned long address) {
	(void)io_sstrg((chanid_t)0,(timeout_t)0,"Store: ",7);
	(void)io_sstrg((chanid_t)0,(timeout_t)0,hostname,strlen(hostname));
	(void)io_sstrg((chanid_t)0,(timeout_t)0,"\n",1);

    #ifdef __DEF_IINCHIP_DBG__
        printf("Set cache entry at %d to for host '%s' to %x.\n", i, hostname, address);
    #endif

    if( 0 <= i && i < CACHED_ENTRIES ) {
        entry_cache[i].hostname = strdup(hostname);
        entry_cache[i].address = address;
        entry_cache[i].refcount = ++cache_refcount;
        #ifdef __DEF_IINCHIP_DBG__
            printf("Set refcount for index %d to %d.\n", i, cache_refcount);
        #endif
    }
}

int find_LRU_cache_entry() {
    unsigned long min = ULONG_MAX;
    int min_index = 0;
    int i;

    #ifdef __DEF_IINCHIP_DBG__
        printf("Find cache LRU entry.\n");
    #endif
    for(i = 0;i < CACHED_ENTRIES; i++) {
        int refcount = entry_cache[i].refcount;
        #ifdef __DEF_IINCHIP_DBG__
            printf("refcount[%d] = %d.\n", i, refcount);
        #endif
        if (refcount < min) {
            min = refcount;
            min_index = i;
        }
    }
    #ifdef __DEF_IINCHIP_DBG__
        printf("LRU index = %d.\n", min_index);
    #endif

    return min_index;
}

void store_in_cache( char * hostname, unsigned long address ) {
    int i;

    #ifdef __DEF_IINCHIP_DBG__
        printf("Store host '%s' with address %x in cache.\n", hostname, address);
    #endif

    if(NULL == hostname || 0 == address || 0xffffffff == address) {
        #ifdef __DEF_IINCHIP_DBG__
            printf("Not storing, invalid hostname or address.\n");
        #endif
        return;
    }

    if (get_from_cache( hostname ) > 0 ) {
        #ifdef __DEF_IINCHIP_DBG__
            printf("Not storing, already in cache.\n");
        #endif
        return;
    }
    for(i = 0;i < CACHED_ENTRIES; i++) {
        char *cached_name = entry_cache[i].hostname;
        if(NULL == cached_name) {
            #ifdef __DEF_IINCHIP_DBG__
                printf("Found unused slot at index %d.\n", i);
            #endif
            set_cache_entry(i, hostname, address);
            return;
        }
    }
    #ifdef __DEF_IINCHIP_DBG__
        printf("Cache full, find LRU entry to evict.\n");
    #endif
    {
        int evicted_index = find_LRU_cache_entry();
        sv_memfree(entry_cache[evicted_index].hostname);
        set_cache_entry(evicted_index, hostname, address);
    }
}

void send_udp(SOCKET s, char * buf, uint32 len)
{
 /* uint8 destip[4] = {8,8,8,8}; */
 uint8 destip[4] = {192,168,1,1};
 uint16 destport = 53;
 uint32 sent = 0;

 switch(getSn_SSR(s))
 {
  case SOCK_UDP:
      /* dump(buf,len); */
  sent = sendto(s,(uint8 *)buf,len,destip,destport);
  if(len != sent)
  {
     TRACE(("%d : Sendto Fail.len=%d, sent=%u, ",s,len, sent));
     TRACE(("%d.%d.%d.%d(%d)\r\n",destip[0],destip[1],destip[2],destip[3],destport));
 }
 break;
 default:
 TRACE(("UDP send failed. Socket not in SOCK_UDP state. Closing socket.\n"));
 socket_close(s);
 break;
}
}

void read_udp(SOCKET s, char * buf, uint32 len)
{
  static uint8 destip[4];
  static uint16 destport;
  static int i;

  switch(getSn_SSR(s))
    {
    case SOCK_UDP:
      for(i=0;i<10;i++) {
        if((getSn_RX_RSR(s)) > 0)                   /* check the size of received data */
          {
            len = recvfrom(s,(uint8 *)buf,len,destip,&destport);  /* receive data from a destination */
            TRACE(("Received %d bytes\n",len));
            /* dump(buf,len); */
            break;
          } else {
          TRACE(("Still waiting for answer... "));
          sleep(1);
        }
      }
      break;
    case SOCK_CLOSED:                                  /* CLOSED */
      socket_close(s);                                       /* close the SOCKET */
      break;
    default:
      break;
    }
}

char* ReadName(char* reader,char* buffer,int* count)
{
    char *name;
    unsigned int p=0,jumped=0,offset;
    int i , j;

    *count = 1;
    name = (char*)sv_memalloc(256);

    name[0]='\0';

    /*read the names in 3www6google3com format */
    while(*reader!=0)
    {
        if(*reader>=192)
        {
            offset = (*reader)*256 + *(reader+1) - 49152; /*49152 = 11000000 00000000 ;) */
            reader = buffer + offset - 1;
            jumped = 1; /*we have jumped to another location so counting wont go up! */
        }
        else
        {
            name[p++]=*reader;
        }

        reader=reader+1;

        if(jumped==0) *count = *count + 1; /*if we havent jumped to another location then we can count up */
    }

    name[p]='\0'; /*string complete */
    if(jumped==1)
    {
        *count = *count + 1; /*number of steps we actually moved forward in the packet */
    }

    /*now convert 3www6google3com0 to www.google.com */
    for(i=0;i<(int)strlen((const char*)name);i++)
    {
        p=name[i];
        for(j=0;j<(int)p;j++)
        {
            name[i]=name[i+1];
            i=i+1;
        }
        name[i]='.';
    }

    name[i-1]='\0'; /*remove the last dot */

    return name;
}


void ChangetoDnsNameFormat(char* dns, char* host)
{
    int lock=0 , i;
    char *orig_dns;

    orig_dns = dns;

    for(i=0 ; i<(int)strlen(host) ; i++)
    {
        if(host[i]=='.')
        {
            *dns++=i-lock;
            for(;lock<i;lock++)
            {
                *dns++=host[lock];
            }
            lock++; /*or lock=i+1; */
        }
    }
    *dns++='\0';
}

void fillHeader(struct DNS_HEADER * dnsHeader) {
    dnsHeader->qr = 0; /*This is a query */
    dnsHeader->opcode = 0; /*This is a standard query */
    dnsHeader->aa = 0; /*Not Authoritative */
    dnsHeader->tc = 0; /*This message is not truncated */
    dnsHeader->rd = 1; /*Recursion Desired */
    dnsHeader->ra = 0;
    dnsHeader->z = 0;
    dnsHeader->rcode = 0;

    dnsHeader->id = (unsigned short)42; /*Query id of 42*/
    dnsHeader->q_count = 1; /*we have only 1 question */
    dnsHeader->ans_count = 0;
    dnsHeader->ns_count = 0;
    dnsHeader->ar_count = 0;
}

void fillQuery(char * dnsQueryBuffer, char* host) {
    char *qname;
    struct QUESTION *qinfo;
    /*point to the query portion */
    qname =(char*)&(dnsQueryBuffer[sizeof(struct DNS_HEADER)]);
    ChangetoDnsNameFormat(qname,host);
    qinfo =(struct QUESTION*)&(dnsQueryBuffer[sizeof(struct DNS_HEADER) + (strlen((const char*)qname) + 1)]); /*fill it */
    qinfo->qtype_h = 0; /* we are requesting the ipv4 address */
    qinfo->qtype_l = 1; /* we are requesting the ipv4 address */
    qinfo->qclass_h = 0; /* internet */
    qinfo->qclass_l = 1; /* internet */
}

void sendQueryAndReceiveResponse(char *buf, uint32 bufSize ,uint32 len) {
    SOCKET s = 7;  /* Number of socket to use: 0-7 */
    TRACE(("socket\n"));
    open_socket(s,Sn_MR_UDP,4224,0);
    TRACE(("send_udp\n"));
    send_udp(s, buf, len);
    TRACE(("read_udp\n"));
    read_udp(s, buf, bufSize);
    socket_close(s);
    return;
}

uint32 findIp(char * buf) {
    char *answer, *qname;
    uint32 ipAddress;
    struct RES_RECORD dnsAnswer;
    struct DNS_HEADER *dns = NULL;
    int i,j,stop;

    dns = (struct DNS_HEADER *)buf;
    qname =(char*)&(buf[sizeof(struct DNS_HEADER)]);
    answer=&(buf[sizeof(struct DNS_HEADER) + (strlen((const char*)qname)+1) + sizeof(struct QUESTION)]);

    /*printf("\nThe response contains : ");
    printf("\n %d Questions.",dns->q_count);
    printf("\n %d Answers.",dns->ans_count);
    printf("\n %d Authoritative Servers.",dns->auth_count);
    printf("\n %d Additional records.\n\n",dns->add_count);
    */
    /*reading answers */
    stop=0;

    for(i=0;i<dns->ans_count;i++)
    {
        unsigned short type;

        dnsAnswer.name=ReadName(answer,buf,&stop);
        answer = answer + stop;
        dnsAnswer.resource = (struct R_DATA*)(answer);
        answer = answer + sizeof(struct R_DATA);
        type = readWord((char *)&(dnsAnswer.resource->type));
        if( type == T_A) /*if its an ipv4 address */
        {
            uint8 *ipAddrPtr = (uint8 *)&ipAddress;
            for(j=0 ; j<4 ; j++) {
                uint8 num = answer[j];
                ipAddrPtr[j]=num;
            }
            return ipAddress;
        }
        else /* It's an alias name */
        {
            ReadName(answer,buf,&stop);
            answer = answer + stop;
        }
    }
    return (uint32)0;
}

struct channel_hostent *get_channel_hostent( const chanid_t channel ) {
    struct hostent_list_node *current = head, *new_node, *tmp;
    struct channel_hostent *chan_hostent = NULL, *new_chan_hostent;
    struct hostent *new_hostent;
    void *hostent_content_buf, *new_channel_alloc;

    while(NULL != current) {
        if(current->channel == channel) {
            chan_hostent = current->content;
            break;
        }
        current = current->next;
    }
    if( NULL != chan_hostent ) {
        return chan_hostent;
    }
    new_channel_alloc = sv_memalloc(new_channel_alloc_size);
    if(NULL == new_channel_alloc) {
        return NULL;
    }
    /* Separate malloc for the result content buffer
       because it may need to be reallocated to be larger */
    /*
    hostent_content_buf = malloc(HOSTENT_CONTENT_BUF_INITIAL_SIZE);
    if(NULL == hostent_content_buf) {
        free(new_channel_alloc);
        return NULL;
    }
    */

    tmp = (void *)((char *)new_channel_alloc + sizeof(struct hostent_list_node));
    new_node = (struct hostent_list_node *)new_channel_alloc;
    new_chan_hostent = (struct channel_hostent *)tmp;
    new_hostent = (struct hostent *)((char *)new_channel_alloc +
        sizeof(struct hostent_list_node) +
        sizeof(struct channel_hostent));
    TRACE(("New hostent @%08x\n",new_hostent));
    new_chan_hostent->content_buf_size = HOSTENT_CONTENT_BUF_INITIAL_SIZE;
    /* Will be allocated for each response separately */
    new_chan_hostent->hostent_content_buf = NULL;
    new_chan_hostent->hostent = new_hostent;
    new_node->content = new_chan_hostent;
    new_node->channel = channel;
    new_node->next = head;
    head = new_node;
    return new_chan_hostent;
}

void close_channel(const chanid_t channel) {
    struct hostent_list_node *current = head,*prev = NULL;
    while(NULL != current) {
        if( current->channel == channel ) {
            sv_memfree((void *)(current->content->hostent_content_buf));
            if(NULL == prev) {
                head = current->next;
            } else {
                prev->next = current->next;
            }
            TRACE(("Releasing %08x\n", current->content));
            /*  TODO: free the inner mem reservations also (hostent buf) */
            sv_memfree((void *)current);
            break;
        }
        prev = current;
        current = current->next;
    }
}

unsigned short findMallocSize(struct dns_reply_info *info) {
    char *question_start, *current_loc, *name_ptr, *dnsreply=info->reply;
    uint8 label_len=0;
    unsigned int p=0;
    unsigned short qdcount=0,ans_count=0,malloc_size=0,num_a_records=0,num_cname_records=0,
        max_a_name_len=0;
    int i , j;
    struct DNS_HEADER *dns;

    /* TODO check response status: ok or error (or maybe in code calling this function s*/

    dns = (struct DNS_HEADER *)dnsreply;
    /* Jump to beginnning of question section */
    current_loc = (char*)&(dnsreply[sizeof(struct DNS_HEADER)]);

    /*printf("Buffer at %x, current_loc at %x\n", dns, current_loc);*/

    qdcount = dns->q_count; /* Number of questions. usually 1 */
    ans_count = dns->ans_count; /* Number of answers */

    /*printf("%d questions, %d answers\n", qdcount, ans_count);*/
    
    /*  Jump over questions to start of answer section */
    while(qdcount--) {
        /* Jump over query string in DNS label format */
        while(label_len = *current_loc) {
            current_loc += label_len + 1;
        };
        current_loc += 1 + sizeof(struct QUESTION); /* Jump over last "0" of label plus fixed size fields */
    }
    /* current_loc now points at start of answer section */

    info->answer_start = current_loc;

    while(ans_count--) {
        uint16 name_len=0,offset=0;
        uint8 part_len=0,jumped=0;
        struct R_DATA *res_data;
        char *res;
        unsigned short type = 0;

        /* printf("Current_loc now at %x\n", current_loc); */
        name_ptr = current_loc;
        while(part_len = *name_ptr) {
            /*printf("Part len : %d\n", part_len);*/
            if(part_len>=192) {
                if(part_len > 192) {
                    /*printf("*** Part len %x\n", part_len);*/
                }
                offset = (*name_ptr)*256 + *(name_ptr+1) - 49152; /*49152 = 11000000 00000000 ;) */
                /* printf("Offset: %d\n", offset); */
                name_ptr = dnsreply + offset - 1;
                if(!jumped) { /* First jump from current answer*/
                    current_loc += 2; /* Advance location past jump point */
                }
                jumped = 1; /* Remember that we jumped to another location to read rest of label */
            }
            else {
                name_len += part_len+1; /* Including the dot or ending null */
                name_ptr += part_len;
            }
            name_ptr=name_ptr+1; /* Advance to next part len byte */
        }
        /* current_loc points to start of struct R_DATA */
        res_data = (struct R_DATA *)current_loc;
        res = current_loc;
        /*printf("R_data header at %x, Name len: %d, data_len: %d, ttl: %d\n", res_data, name_len, res_data->data_len, res_data->ttl);
        printf("data_len: %d\n", readWord(res+8));*/
        type = readWord(res);
        if( T_CNAME == type ) {
            /*  For CNames we need space for the name plus the pointer to it */
            malloc_size += name_len + sizeof(char *); /* Actual name + a pointer to it */
            num_cname_records++;
        } else if( T_A == type ) {
            if( name_len > max_a_name_len ) {
                max_a_name_len = name_len;
            }
            /* For A records we need space for the address plus a pointer to it
               Name will go to h_name in struct hostent (need space for that, too,
               but only once per reply since multiple A records mean multiple host addresses
               for a single hostname */
            malloc_size += 4 + sizeof(char *); /* 4 bytes for IPv4 address */
            if(!num_a_records) {
                /* If this is the first A record count space for the hostname */
                malloc_size += name_len; /* Space for the pointer is in struct hostent */
            }
            num_a_records++;
        }
        current_loc += sizeof(struct R_DATA) + readWord(res+8);
    }
    malloc_size += 2 * sizeof(char *); /* For null pointers at ends of pointer tables */
    /* For 2nd pass we may need temporary storage for 1 max len A record name */
    malloc_size += max_a_name_len + 1; /* Plus terminating null */
    info->num_cnames = num_cname_records;
    info->num_as = num_a_records;
    info->malloc_size = malloc_size;
    info->a_name_max_len = max_a_name_len;
    return malloc_size;
}

void name_to_buf(char **src_h, char **target_h, char *dns_reply) {
    char *src = *src_h,*target = *target_h;
    char *reply_loc = src;
    uint16 offset=0;
    uint8 label_len=0,jumped=0;

    while(label_len = *src) {
        if(label_len>=192) {
            offset = (*src)*256 + *(src+1) - 49152; /*49152 = 11000000 00000000 ;) */
            src = dns_reply + offset;
            if(!jumped) { /* First jump from current answer*/
                reply_loc += 2; /* Advance location past jump point */
            }
            jumped = 1; /* Remember that we jumped to another location to read rest of label */
        }
        else {
            src++; /* Move past the length byte to start of label string  */
            while(label_len--) {
                *target++ = *src++;
                if(!jumped) {
                    reply_loc++;
                }
            }
            *target++ = '.';
        }
    }
    *(--target) = '\0';
    *src_h = reply_loc;
    *target_h = target+1;
}

void fill_hostent( struct hostent *hostent, const struct dns_reply_info *info, const char *buf ) {
    static uint16 ans_count;
    /* Start of buf contains pointer table to IPv4 addresses terminated by null pointer
    and a pointer table to cnames, again terminated by a null pointer 
    Offset points to past of these two pointer tables */
    static uint32 *addr_table;
    static char *content;
    static char *prev_start;
    static char *res_pointer;
    static char **alias_table_ptr;
    static char **addr_table_ptr;
    static uint8 a_record_found;

    ans_count = info->num_as+info->num_cnames;
    /* Start of buf contains pointer table to IPv4 addresses terminated by null pointer
       and a pointer table to cnames, again terminated by a null pointer 
       Offset points to past of these two pointer tables */
    addr_table = buf + (ans_count + 2)*sizeof(char *);
    content = ((char *)addr_table) + info->num_as * sizeof(uint32);
    prev_start = content;
    res_pointer = info->answer_start;
    alias_table_ptr;
    addr_table_ptr = (char **)buf;
    a_record_found = 0;

    TRACE(("#ans: %d, hostent: %08x, reply: %08x, answer: %08x, buf: %08x, addr_table_ptr %08x\n",
           ans_count, hostent, info->reply, res_pointer, buf, addr_table_ptr));
    hostent->h_addrtype = CLASS_IN;
    hostent->h_length = sizeof(uint32);
    hostent->h_addr_list = (char **)buf;
    hostent->h_aliases = (char **)(buf + (info->num_as + 1)*sizeof(char *));
    alias_table_ptr = hostent->h_aliases;
    TRACE(("h_addr_list: %08x, h_aliases: %08x\n", hostent->h_addr_list, hostent->h_aliases));
    while(ans_count--) {
        static uint32 *ptr_addr;
        static uint16 name_len,offset;
        static uint8 part_len,jumped;
        static struct R_DATA *res_data;
        static char *res;
        static unsigned short type;

        TRACE(("ans_count: %d\n",ans_count));
        name_len = offset = 0;
        part_len = jumped = 0;
        type = 0;

        name_to_buf(&res_pointer, &content, info->reply);
        TRACE(("Found name, "));
        type = readWord(res_pointer);
        if( T_CNAME == type ) {
            TRACE(("alias\n"));
            *alias_table_ptr++ = prev_start;
            prev_start = content;
        } else if( T_A == type ) {
            uint32 address = 0;
            unsigned short a_h, a_l;
            a_h = readWord(res_pointer + 10);
            a_l = readWord(res_pointer + 12);
            address = a_h << 16;
            address += a_l;
            if(!a_record_found) {
                hostent->h_name = prev_start;
                prev_start = content;
            } else {
                content = prev_start;
                a_record_found++;
            }
            TRACE(("address %08x\n",address));
            *addr_table = address;
            *addr_table_ptr++ = addr_table;
            addr_table++;
        }
        res_pointer += sizeof(struct R_DATA) + readWord(res_pointer + 8);
    }
    *addr_table_ptr=NULL;
    *alias_table_ptr=NULL;
}

struct hostent *gethostbyname_impl(const char *hostname, const chanid_t channel) {
    static char *buf,*qname,*content,*orig_buf;
    static char host[260];
    static int i , j, pton_result;
    /* response code from server for error checking */
    static uint8 rcode = 0;
    static uint32 ipv4address = 0;
    static struct DNS_HEADER *dns = NULL;
    static struct channel_hostent *chan_hostent;
    /* hostname + host terminating null + null alias table entry + address table + one address */
    unsigned short malloc_size=strlen(host)+1+4+8+4;
    static struct dns_reply_info info;

    if(hostname[0]==0) {
        return NULL;
    }
    strcpy(host,hostname);
    chan_hostent = get_channel_hostent( channel );
    if( NULL == chan_hostent ) {
        /* TODO: set errno */
        TRACE(("Could not retrive chan_hostent\n"));
        return NULL;
    }
    TRACE(("Channel hostent at %08x, hostent: %08x\n", chan_hostent,chan_hostent->hostent));
    /* If this hostent was used previously it will have  allocated
       space for the previous result. Free and reallocate to the new size */
    if( content = chan_hostent->hostent_content_buf ) {
        TRACE(("Freeing previous content buf at %0x\n", content));
        sv_memfree( content );
    }
    /* First check if the host string is an ip address */
    pton_result = inet_pton(AF_INET, host, (uint8 *)(&ipv4address));
    if( pton_result ) {
        static char *content;
        static char *h_name;
        static char **table;
        static struct hostent *he;

        content = sv_memalloc(malloc_size);
        if ( NULL == content ) {
            /* TODO set errno */
            return NULL;
        }
        h_name = content + 16;
        table = (char **)content;
        he = chan_hostent->hostent;

        TRACE(("Allocated %u bytes for hostent content at %08x\n", malloc_size, content));
        chan_hostent->hostent_content_buf = (void *)content;
        table[3] = (char *)ipv4address;
        table[0] = (char *)&(table[3]);
        table[1] = (char *)NULL;
        table[2] = (char *)NULL;
        strcpy(h_name,host);
        he->h_name  = h_name;
        he->h_addr_list = &table[0];
        he->h_aliases = &table[2];

        return chan_hostent->hostent;
    }
    if(host[strlen(host)-1]!='.') {
        strcat(host,".");
    }

    TRACE(("# Channel hostent at %08x, hostent: %08x\n", chan_hostent,chan_hostent->hostent));
    orig_buf = buf = (char *)sv_memalloc(UDP_MSG_BUF_SIZE);
    TRACE(("## Channel hostent at %08x, hostent: %08x\n", chan_hostent,chan_hostent->hostent));
    if( NULL == buf ) {
        /* TODO set errno */
        return NULL;
    }
    TRACE(("Allocated %u bytes for DNS query/reply buf at %08x\n", UDP_MSG_BUF_SIZE, orig_buf));
    dns = (struct DNS_HEADER *)buf;
    qname =(char*)&(buf[sizeof(struct DNS_HEADER)]);
    TRACE(("BFFH -> Channel hostent at %08x, hostent: %08x\n", chan_hostent,chan_hostent->hostent));
    fillHeader(dns);
    /* dump((char *)dns,sizeof(struct DNS_HEADER)); */
    /* TRACE(("Query host '%s'\n",host)); */
    TRACE(("BFQ -> Channel hostent at %08x, hostent: %08x\n", chan_hostent,chan_hostent->hostent));
    fillQuery(buf, (char *)host);
    /* TODO correlate query and answer with an Id*/
    sendQueryAndReceiveResponse(buf, UDP_MSG_BUF_SIZE, (uint32)sizeof(struct DNS_HEADER) + (strlen((const char*)qname)+1) + sizeof(struct QUESTION));
    /*
    SOCKET s = 0; 
    socket(s,Sn_MR_UDP,4224,0);
    send_udp(s, buf, len);
    read_udp(s, buf, bufSize);
    return;
    close(s);
    */

    rcode = (uint8)buf[3] & 0xF;
    if( 0 != rcode ) {
        /* TODO set errno */
        TRACE(("Return code from DNS query: %d\n",rcode));
        return NULL;
    }
    info.reply = buf;
    malloc_size = findMallocSize(&info);
    content = sv_memalloc( malloc_size );
    if( NULL == content ) {
        /* TODO set errno */
        return NULL;
    }
    TRACE(("Allocated %u bytes for hostent content at %08x\n", malloc_size, content));
    chan_hostent->hostent_content_buf = (void *)content;
    TRACE(("Call fill_hostent, hostent: %08x, buf: %08x\n", chan_hostent->hostent, content));
    fill_hostent( chan_hostent->hostent, &info, content );

    /* { */
    /*     struct hostent *h = chan_hostent->hostent; */
        /* TRACE(("Host: %s, %d addresses\n",h->h_name,h->h_length)); */
        /* int i,n,*q; */
        /* for(i = 0; q = (int *)(h->h_addr_list[i]); i++) */
        /* { */
        /*     n = *q; */
        /*     TRACE(("IP   : %08x %08x\n", q,n)); */
        /* } */
    /* } */
    /* TRACE(("Freeing buf at %08x\n", orig_buf)); */
    sv_memfree(orig_buf);
    return chan_hostent->hostent;
}

uint32 ngethostbyname(char *host)
{
    char *buf,*qname;
    int i , j, pton_result;
    uint32 ipv4address = 0;
    struct DNS_HEADER *dns = NULL;

	if(host[0]==0) {
		return 0;
	}

    if(!initialized) {
        initialize_cache();
        initialized = 1;
    }
    /* First check if the host string is an ip address */
    pton_result = inet_pton(AF_INET, host, (uint8 *)(&ipv4address));
	if( pton_result ) {
		return ipv4address;
	}
	if(host[strlen(host)-1]!='.') {
		strcat(host,".");
	}
	/* Try cache */
	ipv4address = get_from_cache(host);
	if( ipv4address > 0) {
		(void)io_sstrg((chanid_t)0,(timeout_t)0,"Cache hit\n",10);
		return ipv4address;
	}
	/* Otherwise make a DNS query with the name */
	/*Set the DNS structure to standard queries */
	(void)io_sstrg((chanid_t)0,(timeout_t)0,"Cache miss\n",11);
	buf = (char *)sv_memalloc(UDP_MSG_BUF_SIZE);
	dns = (struct DNS_HEADER *)buf;
	qname =(char*)&(buf[sizeof(struct DNS_HEADER)]);
	fillHeader(dns);
	fillQuery(buf, (char *)host);
	sendQueryAndReceiveResponse(buf, UDP_MSG_BUF_SIZE, (uint32)sizeof(struct DNS_HEADER) + (strlen((const char*)qname)+1) + sizeof(struct QUESTION));
	ipv4address = findIp(buf);
	sv_memfree(buf);
	store_in_cache(host, ipv4address);
	return ipv4address;
}

inline void dump(uint8* buf, uint32 len) {
    static uint32 i = 0;
    TRACE(("@%d:",buf));
    for(i = 0;i< len;i++) {
        TRACE(("%02x.",buf[i]));
    }
}
