#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <stdio.h>
#include <stdint.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>

#include <rte_debug.h>
#include <rte_cfgfile.h>
#include <rte_malloc.h>
#include <rte_common.h>
#include <rte_eal.h>
#include <rte_lcore.h>
#include <rte_branch_prediction.h>
#include <rte_mbuf.h>
#include <rte_memcpy.h>
#include <rte_ethdev.h>
#include <rte_errno.h>


/* D E F I N E S */   
#define CONFIGFILE      "config.ini"
#define PKTGEN_INTFTAG  "INTF_"

#define MAX_NUMANODE    4
#define MAX_LCORECOUNT  32
#define MAX_INTFCOUNT   4
#define PKTGEN_MAXPORTS 8

#define ARP_MAXCOUNT   2
#define ICMP4_MAXCOUNT 2
#define ICMP6_MAXCOUNT 2
#define IPV4_MAXCOUNT  4
#define IPV6_MAXCOUNT  4
#define TCP4_MAXCOUNT  4
#define TCP6_MAXCOUNT  4
#define UDP4_MAXCOUNT  4
#define UDP6_MAXCOUNT  4


#define NB_MBUF   (8192 * 2)
#define MBUF_SIZE (2048 + sizeof(struct rte_mbuf) + RTE_PKTMBUF_HEADROOM)

#define STRCPY(x, y) strpy((char *)x, (const char *)y)
#define STRCMP(x, y) strcmp((const char *)x, (const char *)y)

#define GTPU_ICMPREQ {0x00, 0x0c, 0x29, 0xda, 0xd1, 0xde, 0x00, 0x0c, 0x29, 0xe3, 0xc6, 0x4d, 0x08, 0x00, 0x45, 0x00, 0x00, 0x7c, 0x00, 0x00, 0x40, 0x00, 0x40, 0x11, 0x67, 0xbb, 0xc0, 0xa8, 0x28, 0xb3, 0xc0, 0xa8, 0x28, 0xb2, 0x08, 0x68, 0x08, 0x68, 0x00, 0x68, 0xbf, 0x64, 0x32, 0xff, 0x00, 0x58, 0x00, 0x00, 0x00, 0x01, 0x28, 0xdb, 0x00, 0x00, 0x45, 0x00, 0x00, 0x54, 0x00, 0x00, 0x40, 0x00, 0x40, 0x01, 0x5e, 0xa5, 0xca, 0x0b, 0x28, 0x9e, 0xc0, 0xa8, 0x28, 0xb2, 0x08, 0x00, 0xbe, 0xe7, 0x00, 0x00, 0x28, 0x7b, 0x04, 0x11, 0x20, 0x4b, 0xf4, 0x3d, 0x0d, 0x00, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37}
#define GTPU_ICMPREP {0x00, 0x0c, 0x29, 0xe3, 0xc6, 0x4d, 0x00, 0x0c, 0x29, 0xda, 0xd1, 0xde, 0x08, 0x00, 0x45, 0x00, 0x00, 0x7c, 0x00, 0x00, 0x40, 0x00, 0x40, 0x11, 0x67, 0xbb, 0xc0, 0xa8, 0x28, 0xb2, 0xc0, 0xa8, 0x28, 0xb3, 0x08, 0x68, 0x08, 0x68, 0x00, 0x68, 0xc1, 0xc6, 0x32, 0xff, 0x00, 0x58, 0x00, 0x00, 0x00, 0x01, 0x26, 0x79, 0x00, 0x00, 0x45, 0x00, 0x00, 0x54, 0x06, 0x74, 0x00, 0x00, 0x40, 0x01, 0x98, 0x31, 0xc0, 0xa8, 0x28, 0xb2, 0xca, 0x0b, 0x28, 0x9e, 0x00, 0x00, 0xc6, 0xe7, 0x00, 0x00, 0x28, 0x7b, 0x04, 0x11, 0x20, 0x4b, 0xf4, 0x3d, 0x0d, 0x00, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37}

#define GTPC_CREATEPDU_REQ {0x00, 0x05, 0x85, 0xa6, 0x54, 0x5f, 0x00, 0x1e, 0xdf, 0x2d, 0x82, 0x8e, 0x08, 0x00, 0x45, 0x00, 0x00, 0xad, 0x13, 0xae, 0x40, 0x00, 0x3f, 0x11, 0x30, 0x62, 0xc0, 0xa9, 0x64, 0x01, 0x0a, 0x64, 0xc8, 0x21, 0x85, 0xe1, 0x08, 0x4b, 0x00, 0x99, 0xc6, 0xb3, 0x32, 0x10, 0x00, 0x89, 0x00, 0x00, 0x00, 0x00, 0x13, 0x0b, 0x00, 0x00, 0x02, 0x64, 0x00, 0x40, 0x01, 0x00, 0x00, 0x01, 0xf1, 0x03, 0x64, 0xf0, 0x60, 0xff, 0xfe, 0xff, 0x0e, 0xb0, 0x0f, 0xfd, 0x10, 0x32, 0xf0, 0x2b, 0xf9, 0x11, 0x32, 0xf0, 0x2b, 0xf9, 0x14, 0x05, 0x80, 0x00, 0x02, 0xf1, 0x21, 0x83, 0x00, 0x07, 0x06, 0x65, 0x65, 0x74, 0x65, 0x73, 0x74, 0x84, 0x00, 0x1a, 0x80, 0x80, 0x21, 0x16, 0x01, 0x01, 0x00, 0x16, 0x03, 0x06, 0x00, 0x00, 0x00, 0x00, 0x81, 0x06, 0x00, 0x00, 0x00, 0x00, 0x83, 0x06, 0x00, 0x00, 0x00, 0x00, 0x85, 0x00, 0x04, 0xc0, 0xa9, 0x64, 0x01, 0x85, 0x00, 0x04, 0xc0, 0xa9, 0x64, 0x01, 0x86, 0x00, 0x08, 0x91, 0x68, 0x51, 0x22, 0x01, 0x00, 0x01, 0xf1, 0x87, 0x00, 0x0c, 0x02, 0x1b, 0x42, 0x1f, 0x73, 0x8c, 0x40, 0x40, 0x74, 0x4b, 0x40, 0x40, 0x97, 0x00, 0x01, 0x02, 0x99, 0x00, 0x02, 0x23, 0x20, 0xff, 0x00, 0x05, 0x2a, 0xab, 0x02, 0x01, 0x03}
#define GTPC_CREATEPDU_REP {0x00, 0x1e, 0xdf, 0x2d, 0x82, 0x8e, 0x00, 0x05, 0x85, 0xa6, 0x54, 0x5f, 0x08, 0x00, 0x45, 0x00, 0x00, 0x89, 0xe3, 0x60, 0x00, 0x00, 0x3f, 0x11, 0xa0, 0xd3, 0x0a, 0x64, 0xc8, 0x21, 0xc0, 0xa9, 0x64, 0x01, 0x08, 0x4b, 0x85, 0xe1, 0x00, 0x75, 0xa7, 0x6d, 0x32, 0x11, 0x00, 0x65, 0x32, 0xf0, 0x2b, 0xf9, 0x13, 0x0b, 0x00, 0x00, 0x01, 0x80, 0x08, 0xfe, 0x0e, 0x18, 0x10, 0x10, 0x00, 0x00, 0x85, 0x11, 0x10, 0x00, 0x00, 0x80, 0x14, 0x05, 0x7f, 0x06, 0x23, 0xa7, 0xc9, 0x80, 0x00, 0x06, 0xf1, 0x21, 0xc0, 0xa8, 0xfc, 0x82, 0x84, 0x00, 0x21, 0x80, 0x80, 0x21, 0x10, 0x04, 0x01, 0x00, 0x10, 0x81, 0x06, 0x00, 0x00, 0x00, 0x00, 0x83, 0x06, 0x00, 0x00, 0x00, 0x00, 0x80, 0x21, 0x0a, 0x03, 0x01, 0x00, 0x0a, 0x03, 0x06, 0xc0, 0xa8, 0xfc, 0x82, 0x85, 0x00, 0x04, 0x0a, 0x64, 0xc8, 0x22, 0x85, 0x00, 0x04, 0x0a, 0x64, 0xc8, 0x31, 0x87, 0x00, 0x0c, 0x02, 0x1b, 0x42, 0x1f, 0x73, 0x8c, 0x40, 0x40, 0x74, 0x4b, 0x40, 0x40}
#define GTPC_ECHOREQ {0x01, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x45, 0x00, 0x00, 0x28, 0x00, 0x00, 0x40, 0x00, 0x40, 0x11, 0x3c, 0xc2, 0x7f, 0x00, 0x00, 0x02, 0x7f, 0x00, 0x00, 0x01, 0x08, 0x4b, 0x08, 0x4b, 0x00, 0x14, 0xfe, 0x28, 0x32, 0x01, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x00}
#define GTPC_ECHOREP {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x45, 0x00, 0x00, 0x2a, 0x00, 0x00, 0x40, 0x00, 0x40, 0x11, 0x3c, 0xc0, 0x07, 0xf0, 0x00, 0x00, 0x17, 0xf0, 0x00, 0x00, 0x20, 0x84, 0xb0, 0x84, 0xb0, 0x01, 0x6f, 0xe2, 0xa3, 0x20, 0x20, 0x00, 0x60, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x00, 0x00, 0x00, 0xe0, 0x1}

#define ARP_REQUEST {0x48, 0x5b, 0x39, 0x25, 0xe3, 0x10, 0x00, 0x1d, 0x09, 0x44, 0x14, 0x85, 0x08, 0x06, 0x00, 0x01, 0x08, 0x00, 0x06, 0x04, 0x00, 0x01, 0x00, 0x1d, 0x09, 0x44, 0x14, 0x85, 0x0a, 0x00, 0x00, 0x32, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0a, 0x00, 0x00, 0x0b  }

#define ARP_RESPONSE {0xd4, 0xbe, 0xd9, 0xd5, 0x35, 0x71, 0x78, 0x2b,0xcb, 0xab, 0x57, 0x3a, 0x08, 0x06, 0x00, 0x01, 0x08, 0x00, 0x06, 0x04, 0x00, 0x02, 0x78, 0x2b, 0xcb, 0xab, 0x57, 0x3a, 0xac, 0x14, 0x01, 0x36, 0xd4, 0xbe, 0xd9, 0xd5, 0x35, 0x71, 0xac, 0x14, 0x01, 0x52, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }

typedef enum {
ARP,
ICMP4,
ICMP6,
IPV4,
IPV6,
TCP4,
TCP6,
UDP4,
UDP6, 
} pktType;

/* PKT-BLSTER per port data */
typedef struct port_config_s {
    uint16_t pktSize;
    uint16_t prtIndex;
    uint16_t type;
    uint16_t pktIndex;
} port_config_t;

int32_t loadConfig(void);

#endif /*__CONFIG_H__*/
