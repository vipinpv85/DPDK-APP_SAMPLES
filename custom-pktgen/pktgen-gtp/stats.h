#ifndef __STATS_H_
#define __STATS_H_

#include <inttypes.h>

#include "config.h"

#include <rte_timer.h>
#include <rte_cycles.h>

#define STATS_CLR_SCREEN   printf("\033[2J")
//#define STATS_ROW(x)       printf("\033[x;1H")
//#define STATS_ROW_COL(x,y) printf("\033[x;yH")
#define STATS_ROW(x)       "\033[x;1H"
#define STATS_ROW_COL(x,y) "\033[x;yH"
#define STATS_POS_OFFSET   2

typedef enum {
    LINK_SPEED = 4,
    NUMA_SOCKET,
    LINK_SPEED_STATE,
    PKTS_PER_SEC_RX,
    PKTS_PER_SEC_TX,
    MB_RX,
    MB_TX,
    PKT_INFO,
    DST_MAC,
    SRC_MAC,
    BS_SRC_IP,
    BS_DST_IP,
    GTP_TYPE_VER,
    GTP_TEID,
    UE_SRC_IP,
    UE_DST_IP,
    UE_IP_PROTO,
    GTPC_V1_TX_IPV4,
    GTPC_V1_TX_IPV6,
    GTPC_V2_TX_IPV4,
    GTPC_V2_TX_IPV6,
    GTPC_V1_RX_IPV4,
    GTPC_V1_RX_IPV6,
    GTPC_V2_RX_IPV4,
    GTPC_V2_RX_IPV6,
    GTPU_TX_IPV4,
    GTPU_TX_IPV6,
    GTPU_RX_IPV4,
    GTPU_RX_IPV6,
    SURICATA_QUEUE_RX,
    SURICATA_QUEUE_TX,
} statsDisplayPos;

typedef struct pkt_stats_s {
    uint64_t rx_gtpc_v1_ipv4;
    uint64_t rx_gtpc_v2_ipv4;
    uint64_t rx_gtpc_v1_ipv6;
    uint64_t rx_gtpc_v2_ipv6;
    uint64_t rx_gptu_ipv4;
    uint64_t rx_gptu_ipv6;

    uint64_t tx_gtpc_v1_ipv4;
    uint64_t tx_gtpc_v2_ipv4;
    uint64_t tx_gtpc_v1_ipv6;
    uint64_t tx_gtpc_v2_ipv6;
    uint64_t tx_gptu_ipv4;
    uint64_t tx_gptu_ipv6;

    uint64_t dropped;

    uint64_t suricata_rx_queue;
    uint64_t suricata_tx_queue;

    uint64_t rxPkts;
    uint64_t txPkts;
    uint64_t rxBytes;
    uint64_t txBytes;
    uint64_t rxMissed;
    uint64_t rxErr;
    uint64_t txErr;
    uint64_t rxNoMbuff;
} pkt_stats_t; /* per interface */

void get_link_stats(__attribute__((unused)) struct rte_timer *t, 
                    __attribute__((unused)) void *arg);
void get_process_stats(__attribute__((unused)) struct rte_timer *t, 
                       __attribute__((unused)) void *arg);

void set_stats_timer(void);
void show_static_display(void);

#endif /* __STATS_H__ */

