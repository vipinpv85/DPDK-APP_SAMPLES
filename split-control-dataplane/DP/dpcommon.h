#ifndef DP_COMMON_H_
#define DP_COMMON_H_

#ifdef __cplusplus
extern "C" {
#endif

/* INCLUDE */
#include <inttypes.h>

#include <rte_ethdev.h>
#include <rte_ring.h>
#include <rte_branch_prediction.h>
#include <rte_ether.h>
#include <rte_ethdev.h>
#include <rte_ip.h>
#include <rte_tcp.h>
#include <rte_udp.h>
#include <rte_arp.h>
#include <rte_icmp.h>
#include <rte_sctp.h>

#include "dptables.h"

/* DEFINE */
#define BYPASS_MODE         0x01

/* Port Map */
#define PORTMAP_DROP        (0xFF)
#define PORTMAP_ISDRP       PORTMAP_DROP

/* Port Actions */
#define PORTACT_DROP        (0x01)
#define PORTACT_FDROP       (0x02)
#define PORTACT_ACL         (0x04)
#define PORTACT_ST          (0x08)

#define PORTACT_CMPLT       (PORTACT_ACL | PORTACT_ST)
#define PORTACT_NOPRC       (PORTACT_PASS)

#define RTE_PORT_ALL        (0xFF)
#define RTE_RING_SIZE       (16)

#define MBUF_SIZE           (2048 + sizeof(struct rte_mbuf) + RTE_PKTMBUF_HEADROOM)
/*#define NB_MBUF             (8192)*/
#define NB_MBUF             (8192 * 2)
#define MAX_PKT_BURST       (32)

#define S_DPDK_MAX_ETHPORTS (4)
#define RTE_TEST_RX_DESC_DEFAULT (128)
#define RTE_TEST_TX_DESC_DEFAULT (512)

#define ETH_HEADER_LEN      (14)
#define IPV4_LEN            (20)
#define IPV6_LEN            (36)
#define UDP_LEN              (8)

#define PROTO_ICMP          (0x01)
#define PROTO_TCP           (0x06)
#define PROTO_UDP           (0x11)
#define PROTO_SCTP          (0x84)
#define PROTO_GRE           (0x2F)
#define PROTO_IPV4          (0x0800)
#define PROTO_VLAN          (0x8100)
#define PROTO_IPV6          (0x86DD)
#define PROTO_ARP           (0x0806)

#define MSG(m, ...) fprintf(stderr, "\n %s %d - " m " \n", __func__, __LINE__, ##__VA_ARGS__)

/* ENUM */
typedef enum {
     MEMEORY = 1,
     INTERFACE
} error_cause_t;

typedef enum proto
{
    ETH,
    VLAN,
    IPv4,
    IPv6,
    ARP,
    ICMP,
    TCP,
    UDP,
    DEFAULT
}proto_t;

typedef struct dp_stats {
    uint64_t eth_cnt;
    uint64_t ipv4_cnt;
    uint64_t ipv6_cnt;
    uint64_t tcp_cnt;
    uint64_t http_cnt;
    uint64_t http_data;
    uint64_t udp_cnt;
    uint64_t icmp_cnt;
    uint64_t arp_cnt;
    uint64_t vlan_cnt;
} dp_stats_t;

typedef struct rxfp_stats {
    uint64_t pmap_drop;
    uint64_t pact_drop;
    uint64_t acl_Lkp;
    uint64_t acl_nLkp;
    uint64_t acl_hit;
    uint64_t acl_miss;
    uint64_t st_fwd;
    uint64_t st_err;
    uint64_t pfwd;
    uint64_t pdrop;
    uint64_t f4drp;
} rxfp_stat_t;

typedef struct dp_metadata {
    uint8_t portid;
    proto_t prev_proto;
    dp_stats_t dp_cnt;
    uint16_t l2_offset;
    uint8_t l3_len;
    uint8_t l4_len;
    uint16_t l3_offset;
    uint16_t l4_offset;
    uint8_t vlan:3;
    uint8_t ipv4:1;
    uint8_t ipv4_fragments:2;
    uint8_t ipv6:1;
    uint8_t arp:1;
    uint8_t tcp:1;
    uint8_t udp:1;
    uint8_t icmp:1;
    uint8_t gre:1;
    uint8_t sctp:1;
    uint8_t ssh:1;
    uint8_t nd:1;
    uint8_t rtsp:1;
    uint8_t http:1;
    uint8_t rtp:1;
    uint8_t gtp:1;
    uint8_t vxlan:1;
    uint64_t app_data:3;
} dp_metadata_t;

/* PROTOTYPE */
void sigHandler(int signo);
void printPktStats(void);
void dp_cnt_dummy(struct rte_mbuf *m, dp_metadata_t *dp_metadata);
void dp_parse_pkt_dummy(struct rte_mbuf *m ,dp_metadata_t *dp_metadata);

int32_t check_all_ports_link_status(uint32_t port_mask);
int32_t frameParseSetup(__attribute__((unused)) void *arg);
int32_t etherParse(__attribute__((unused)) void *arg);
int32_t userThread(__attribute__((unused)) void *arg);
int32_t ethDevStart(void);
int32_t ethDevSetup(void);
int32_t userFrameProcess(uint32_t userData, uint8_t coreId, struct rte_mbuf *m);

typedef void (*dp_counter_incr_func)(struct rte_mbuf *,dp_metadata_t *);
typedef void (*dp_parser_func)(struct rte_mbuf *,dp_metadata_t *);

#ifdef __cplusplus
}
#endif


#endif /* DP_COMMON */
