/*
 * SPDX: MIT
 */

#ifndef	W5300_REGS_H
#define	W5300_REGS_H

/**
 * Memory mapped hardware reset
 */

#define W5300_MR              (0x00)
#define W5300_MR_DBW          (1 << 15)            /**< Data bus width bit of MR. */
#define W5300_MR_MPF          (1 << 14)            /**< Mac layer pause frame bit of MR. */
#define W5300_MR_WDF(X)       ((X & 0x07) << 11)   /**< Write data fetch time bit of  MR. */
#define W5300_MR_RDH          (1 << 10)            /**< Read data hold time bit of MR. */
#define W5300_MR_FS           (1 << 8)             /**< FIFO swap bit of MR. */
#define W5300_MR_RST          (1 << 7)             /**< S/W reset bit of MR. */
#define W5300_MR_MT           (1 << 5)             /**< Memory test bit of MR. */
#define W5300_MR_PB           (1 << 4)             /**< Ping block bit of MR. */
#define W5300_MR_PPPoE        (1 << 3)             /**< PPPoE bit of MR. */
#define W5300_MR_DBS          (1 << 2)             /**< Data bus swap of MR. */
#define W5300_MR_IND          (1 << 0)             /**< Indirect mode bit of MR. */

#define W5300_IDM_AR          (0x02)

#define W5200_IDM_DR          (0x04)
#define W5300_IR              (0x02)
#define W5300_IR_IPCF         (1 << 7)             /**< IP conflict bit of IR. */
#define W5300_IR_DPUR         (1 << 6)             /**< Destination port unreachable bit of IR. */
#define W5300_IR_PPPT         (1 << 5)             /**< PPPoE terminate bit of IR. */
#define W5300_IR_FMTU         (1 << 4)             /**< Fragment MTU bit of IR. */
#define W5300_IR_SnINT(n)     (1 << n)          /**< SOCKETn interrupt occurrence bit of IR. */

#define W5300_IMR             (0x04)
#define W5300_SHAR            (0x08)
#define W5300_GAR             (0x10)
#define W5300_SUBR            (0x14)
#define W5300_SIPR            (0x18)
#define W5300_RTR             (0x1C)
#define W5300_RCR             (0x1E)
#define W5300_TMS01R          (0x20)
#define W5300_TMSR0           (W5300_TMS01R)
#define W5300_RMS01R          (0x28)
#define W5300_RMSR0           (W5300_RMS01R)
#define W5300_MTYPER          (0x30)
#define W5300_PATR            (0x32)
#define W5300_PTIMER          (0x36)
#define W5300_PMAGICR         (0x38)
#define W5300_PSIDR           (0x3C)
#define W5300_PDHAR           (0x40)
#define W5300_UIPR            (0x48)
#define W5300_UPORTR          (0x4C)
#define W5300_FMTUR           (0x4E)

#define W5300_Pn_BRDYR(n)     (0x60 + n*4)
#define W5300_Pn_PEN          (1 << 7)             /**< PIN 'BRDYn' enable bit of Pn_BRDYR. */
#define W5300_Pn_MT           (1 << 6)             /**< PIN memory type bit of Pn_BRDYR. */
#define W5300_Pn_PPL          (1 << 5)             /**< PIN Polarity bit of Pn_BRDYR. */
#define W5300_Pn_SN(n)        ((n & 0x07) << 0)    /**< PIN Polarity bit of Pn_BRDYR. */

#define W5300_Pn_BDPTHR(n)    (0x60 + n*4 + 2)
#define W5300_IDR             (0xFE)

#define W5300_SOCKET_REG_BASE (0x200)
#define W5300_SOCKET_REG_SIZE (0x40)

#define W5300_Sn_MR(n)        (W5300_SOCKET_REG_BASE + n * W5300_SOCKET_REG_SIZE + 0x00)
#define W5300_Sn_MR_ALIGN     (1 << 8)             /**< Alignment bit of Sn_MR. */
#define W5300_Sn_MR_MULTI     (1 << 7)             /**< Multicasting bit of Sn_MR. */
#define W5300_Sn_MR_MF        (1 << 6)             /**< MAC filter bit of Sn_MR. */
#define W5300_Sn_MR_IGMPv     (1 << 5)             /**< IGMP version bit of Sn_MR. */
#define W5300_Sn_MR_ND        (1 << 5)             /**< No delayed ack bit of Sn_MR. */
#define W5300_Sn_MR_CLOSE     0x00                 /**< Protocol bits of Sn_MR. */
#define W5300_Sn_MR_TCP       0x01                 /**< Protocol bits of Sn_MR. */
#define W5300_Sn_MR_UDP       0x02                 /**< Protocol bits of Sn_MR. */
#define W5300_Sn_MR_IPRAW     0x03                 /**< Protocol bits of Sn_MR. */
#define W5300_Sn_MR_MACRAW    0x04                 /**< Protocol bits of Sn_MR. */
#define W5300_Sn_MR_PPPoE     0x05                 /**< Protocol bits of Sn_MR. */
#define W5300_Sn_MR_MODE_MASK 0x07                 /* And mask to extract protocol mode */

#define W5300_Sn_CR(n)        (W5300_SOCKET_REG_BASE + n * W5300_SOCKET_REG_SIZE + 0x02)
#define W5300_Sn_CR1(n)       (W5300_SOCKET_REG_BASE + n * W5300_SOCKET_REG_SIZE + 0x03)
#define W5300_Sn_CR_OPEN      0x01                 /**< OPEN command value of Sn_CR. */
#define W5300_Sn_CR_LISTEN    0x02                 /**< LISTEN command value of Sn_CR. */
#define W5300_Sn_CR_CONNECT   0x04                 /**< CONNECT command value of Sn_CR. */
#define W5300_Sn_CR_DISCON    0x08                 /**< DISCONNECT command value of Sn_CR. */
#define W5300_Sn_CR_CLOSE     0x10                 /**< CLOSE command value of Sn_CR. */
#define W5300_Sn_CR_SEND      0x20                 /**< SEND command value of Sn_CR. */
#define W5300_Sn_CR_SEND_MAC  0x21                 /**< SEND_MAC command value of Sn_CR. */
#define W5300_Sn_CR_SEND_KEEP 0x22                 /**< SEND_KEEP command value of Sn_CR */
#define W5300_Sn_CR_RECV      0x40                 /**< RECV command value of Sn_CR */
#define W5300_Sn_CR_PCON      0x23                 /**< PCON command value of Sn_CR */
#define W5300_Sn_CR_PDISCON   0x24                 /**< PDISCON command value of Sn_CR */
#define W5300_Sn_CR_PCR       0x25                 /**< PCR command value of Sn_CR */
#define W5300_Sn_CR_PCN       0x26                 /**< PCN command value of Sn_CR */
#define W5300_Sn_CR_PCJ       0x27                 /**< PCJ command value of Sn_CR */

/**
 *  SOCKETn interrupt mask register
 */
#define W5300_Sn_IMR(n)       (W5300_SOCKET_REG_BASE + n * W5300_SOCKET_REG_SIZE + 0x04)

/**
 *  SOCKETn interrupt register
 */
#define W5300_Sn_IR(n)        (W5300_SOCKET_REG_BASE + n * W5300_SOCKET_REG_SIZE + 0x06)
#define W5300_Sn_IR_PRECV     0x80                 /**< PPP receive bit of Sn_IR */
#define W5300_Sn_IR_PFAIL     0x40                 /**< PPP fail bit of Sn_IR */
#define W5300_Sn_IR_PNEXT     0x20                 /**< PPP next phase bit of Sn_IR */
#define W5300_Sn_IR_SENDOK    0x10                 /**< Send OK bit of Sn_IR */
#define W5300_Sn_IR_TIMEOUT   0x08                 /**< Timout bit of Sn_IR */
#define W5300_Sn_IR_RECV      0x04                 /**< Receive bit of Sn_IR */
#define W5300_Sn_IR_DISCON    0x02                 /**< Disconnect bit of Sn_IR */
#define W5300_Sn_IR_CON       0x01                 /**< Connect bit of Sn_IR */

/**
 *  SOCKETn status register
 */
#define W5300_Sn_SSR(n)       (W5300_SOCKET_REG_BASE + n * W5300_SOCKET_REG_SIZE + 0x08)
#define W5300_Sn_SSR1(n)      (W5300_Sn_SSR(n) + 1)
#define W5300_SOCK_CLOSED     0x00                 /**< SOCKETn is released */
#define W5300_SOCK_ARP        0x01                 /**< ARP-request is transmitted in order to acquire destination hardware address. */
#define W5300_SOCK_INIT       0x13                 /**< SOCKETn is open as TCP mode. */
#define W5300_SOCK_LISTEN     0x14                 /**< SOCKETn operates as "TCP SERVER" and waits for connection-request (SYN packet) from "TCP CLIENT". */
#define W5300_SOCK_SYNSENT    0x15                 /**< Connect-request(SYN packet) is transmitted to "TCP SERVER". */
#define W5300_SOCK_SYNRECV    0x16                 /**< Connect-request(SYN packet) is received from "TCP CLIENT". */
#define W5300_SOCK_ESTABLISHED 0x17                 /**< TCP connection is established. */
#define W5300_SOCK_FIN_WAIT   0x18                 /**< SOCKETn is closing. */
#define W5300_SOCK_CLOSING    0x1A                 /**< SOCKETn is closing. */
#define W5300_SOCK_TIME_WAIT  0x1B                 /**< SOCKETn is closing. */
#define W5300_SOCK_CLOSE_WAIT 0x1C                 /**< Disconnect-request(FIN packet) is received from the peer. */
#define W5300_SOCK_LAST_ACK   0x1D                 /**< SOCKETn is closing. */
#define W5300_SOCK_UDP        0x22                 /**< SOCKETn is open as UDP mode. */
#define W5300_SOCK_IPRAW      0x32                 /**< SOCKETn is open as IPRAW mode. */
#define W5300_SOCK_MACRAW     0x42                 /**< SOCKET0 is open as MACRAW mode. */
#define W5300_SOCK_PPPoE      0x5F                 /**< SOCKET0 is open as PPPoE mode. */

/**
 *  SOCKETn source port register
 */
#define W5300_Sn_PORTR(n)     (W5300_SOCKET_REG_BASE + n * W5300_SOCKET_REG_SIZE + 0x0A)

/**
 *  SOCKETn destination hardware address register
 */
#define W5300_Sn_DHAR(n)      (W5300_SOCKET_REG_BASE + n * W5300_SOCKET_REG_SIZE + 0x0C)

/**
 *  SOCKETn destination port register
 */
#define W5300_Sn_DPORTR(n)    (W5300_SOCKET_REG_BASE + n * W5300_SOCKET_REG_SIZE + 0x12)


/**
 *  SOCKETn destination IP address register
 */
#define W5300_Sn_DIPR(n)      (W5300_SOCKET_REG_BASE + n * W5300_SOCKET_REG_SIZE + 0x14)

/**
 *  SOCKETn maximum segment size register
 */
#define W5300_Sn_MSSR(n)      (W5300_SOCKET_REG_BASE + n * W5300_SOCKET_REG_SIZE + 0x18)

/**
 *  SOCKETn protocol of IP header field register
 */
#define W5300_Sn_PROTOR(n)		(W5300_SOCKET_REG_BASE + n * W5300_SOCKET_REG_SIZE + 0x1A)
/**
 *  SOCKETn keep alive timer register
 */
#define W5300_Sn_KPALVTR(n)   W5300_Sn_PROTOR(n)

/**
 *  SOCKETn IP type of service(TOS) register
 */
#define W5300_Sn_TOSR(n)      (W5300_SOCKET_REG_BASE + n * W5300_SOCKET_REG_SIZE + 0x1C)

/**
 *  SOCKETn IP time to live(TTL) register
 */
#define W5300_Sn_TTLR(n)      (W5300_SOCKET_REG_BASE + n * W5300_SOCKET_REG_SIZE + 0x1E)

/**
 *  SOCKETn TX write size register
 */
#define W5300_Sn_TX_WRSR(n)	(W5300_SOCKET_REG_BASE + n * W5300_SOCKET_REG_SIZE + 0x20)

/**
 *  SOCKETn TX free size register
 */
#define W5300_Sn_TX_FSR(n)    (W5300_SOCKET_REG_BASE + n * W5300_SOCKET_REG_SIZE + 0x24)

/**
 *  SOCKETn RX received size register
 */
#define W5300_Sn_RX_RSR(n)    (W5300_SOCKET_REG_BASE + n * W5300_SOCKET_REG_SIZE + 0x28)
#define W5300_Sn_RX_RSR2(n)    (W5300_SOCKET_REG_BASE + n * W5300_SOCKET_REG_SIZE + 0x2A)

/**
 *  SOCKETn fragment register
 */
#define W5300_Sn_FRAGR(n)     (W5300_SOCKET_REG_BASE + n * W5300_SOCKET_REG_SIZE + 0x2C)

/**
 *  SOCKETn TX FIFO register
 */
#define W5300_Sn_TX_FIFOR(n)  (W5300_SOCKET_REG_BASE + n * W5300_SOCKET_REG_SIZE + 0x2E)

/**
 *  SOCKET n RX FIFO register
 */
#define W5300_Sn_RX_FIFOR(n)  (W5300_SOCKET_REG_BASE + n * W5300_SOCKET_REG_SIZE + 0x30)

#define W5300_HW_RESET        (0x400)

/* IP PROTOCOL */
#define W5300_IPPROTO_IP      0   /* Dummy for IP */
#define W5300_IPPROTO_ICMP    1   /* Control message protocol */
#define W5300_IPPROTO_IGMP    2   /* Internet group management protocol */
#define W5300_IPPROTO_GGP     3   /* Gateway^2 (deprecated) */
#define W5300_IPPROTO_TCP     6   /* TCP */
#define W5300_IPPROTO_PUP     12  /* PUP */
#define W5300_IPPROTO_UDP     17  /* UDP */
#define W5300_IPPROTO_IDP     22  /* XNS idp */
#define W5300_IPPROTO_ND      77  /* UNOFFICIAL net disk protocol */
#define W5300_IPPROTO_RAW     255 /* Raw IP packet */

#endif /* W5300_REGS_H */
