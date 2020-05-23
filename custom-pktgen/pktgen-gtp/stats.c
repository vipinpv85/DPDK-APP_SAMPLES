#include "stats.h"

pkt_stats_t prtPktStats [GTP_PKTGEN_MAXPORTS];

static struct rte_timer fetchStats;
static struct rte_timer displayStats;

void get_link_stats(__attribute__((unused)) struct rte_timer *t, 
                    __attribute__((unused)) void *arg)
{
    int32_t i, ports =  rte_eth_dev_count();

    /* get link status for DPDK ports */
    struct rte_eth_stats stats;

    for (i = 0; i < ports; i++)
    {
        /* ToDo: use numa info to identify the ports */
        if (likely(rte_eth_stats_get(i, &stats) == 0)) {
            prtPktStats[i].rxPkts    = (stats.ipackets - prtPktStats[i].rxPkts);
//
//                                        1000000;
            prtPktStats[i].txPkts    = (stats.opackets - prtPktStats[i].txPkts);
//
  //                                      1000000;
            prtPktStats[i].rxBytes   = stats.ibytes/(1024 * 1024); 
            prtPktStats[i].txBytes   = stats.obytes/(1024 * 1024); 
            prtPktStats[i].rxMissed  = stats.imissed; 
            prtPktStats[i].rxErr     = stats.ierrors; 
            prtPktStats[i].txErr     = stats.oerrors; 
            prtPktStats[i].rxNoMbuff = stats.rx_nombuf; 
        }
    }

    return;
}

void get_process_stats(__attribute__((unused)) struct rte_timer *t, 
                       __attribute__((unused)) void *arg)
{
    int32_t i, ports =  rte_eth_dev_count();

    for (i = 0; i < ports; i++)
    {
        /* display dpdk stats from ports */
#if 0
        prtPktStats[i].rxPkts
        prtPktStats[i].txPkts
        prtPktStats[i].rxMissed
        prtPktStats[i].rxErr
        prtPktStats[i].txErr
#endif

        /* display calculated stats */

        /*NUMA_SOCKET*/
        printf("\033[4;%dH", (15 + 10 * i));
        //printf("%-8d |", );

        /*PKTS_PER_SEC_RX*/
        printf("\033[5;%dH", (15 + 10 * i));
        printf("  %-12lu ", prtPktStats[i].rxPkts);

        /*PKTS_PER_SEC_TX*/
        printf("\033[6;%dH", (15 + 10 * i));
        printf("  %-12lu ", prtPktStats[i].txPkts);

        /*MB_RX*/
        printf("\033[7;%dH", (15 + 10 * i));
        printf("  %-12lu ", prtPktStats[i].rxBytes);

        /*MB_TX*/
        printf("\033[8;%dH", (15 + 10 * i));
        printf("  %-12lu ", prtPktStats[i].txBytes);

        /* INTF STATS */
        /* Drop */
        printf("\033[11;%dH", (15 + 10 * i));
        printf("  %-12lu ", prtPktStats[i].dropped);

        /* RX miss */
        printf("\033[12;%dH", (15 + 10 * i));
        printf("  %-12lu ", prtPktStats[i].rxMissed);

        /* RX err */
        printf("\033[13;%dH", (15 + 10 * i));
        printf("  %-12lu ", prtPktStats[i].rxErr);

        /* RX no mbuf */
        printf("\033[14;%dH", (15 + 10 * i));
        printf("  %-12lu ", prtPktStats[i].rxNoMbuff);

        /* TX err */
        printf("\033[15;%dH", (15 + 10 * i));
        printf("  %-12lu ", prtPktStats[i].txErr);

        /*GTPC_V1_TX_IPV4*/
        printf("\033[18;%dH", (15 + 10 * i));
        printf("  %-12lu ", prtPktStats[i].tx_gtpc_v1_ipv4);

        /*GTPC_V1_TX_IPV6*/
        printf("\033[19;%dH", (15 + 10 * i));
        printf("  %-12lu ", prtPktStats[i].tx_gtpc_v1_ipv6);

        /*GTPC_V2_TX_IPV4*/
        printf("\033[20;%dH", (15 + 10 * i));
        printf("  %-12lu ", prtPktStats[i].tx_gtpc_v2_ipv4);

        /*GTPC_V2_TX_IPV6*/
        printf("\033[21;%dH", (15 + 10 * i));
        printf("  %-12lu ", prtPktStats[i].tx_gtpc_v2_ipv6);

        /*GTPC_V1_RX_IPV4*/
        printf("\033[22;%dH", (15 + 10 * i));
        printf("  %-12lu ", prtPktStats[i].rx_gtpc_v1_ipv4);

        /*GTPC_V1_RX_IPV6*/
        printf("\033[23;%dH", (15 + 10 * i));
        printf("  %-12lu ", prtPktStats[i].rx_gtpc_v1_ipv6);

        /*GTPC_V2_RX_IPV4*/
        printf("\033[24;%dH", (15 + 10 * i));
        printf("  %-12lu ", prtPktStats[i].rx_gtpc_v2_ipv4);

        /*GTPC_V2_RX_IPV6*/
        printf("\033[25;%dH", (15 + 10 * i));
        printf("  %-12lu ", prtPktStats[i].rx_gtpc_v2_ipv6);

        /*GTPU_TX_IPV4*/
        printf("\033[26;%dH", (15 + 10 * i));
        printf("  %-12lu ", prtPktStats[i].tx_gptu_ipv4);

        /*GTPU_TX_IPV6*/
        printf("\033[27;%dH", (15 + 10 * i));
        printf("  %-12lu ", prtPktStats[i].tx_gptu_ipv6);

        /*GTPU_RX_IPV4*/
        printf("\033[28;%dH", (15 + 10 * i));
        printf("  %-12lu ", prtPktStats[i].rx_gptu_ipv4);

        /*GTPU_RX_IPV6*/
        printf("\033[29;%dH", (15 + 10 * i));
        printf("  %-12lu ", prtPktStats[i].rx_gptu_ipv6);

        /*SURICATA_QUEUE_RX*/
        printf("\033[31;%dH", (15 + 10 * i));
        printf("  %-12lu ", prtPktStats[i].suricata_rx_queue);

        /*SURICATA_QUEUE_TX*/
        printf("\033[32;%dH", (15 + 10 * i));
        printf("  %-12lu ", prtPktStats[i].suricata_tx_queue);
    }

    fflush(stdout);
    return;
}

void show_static_display(void)
{
    struct rte_eth_link link;
    int32_t i, ports =  rte_eth_dev_count();

    /* clear screen */
    STATS_CLR_SCREEN;

    /* stats header */
    printf("\033[2;1H");
    printf(" %-10s | ", "Cat|Intf");
    printf("\033[3;1H");
    printf("================================================================");

    /*NUMA_SOCKET*/
    printf("\033[4;1H");
    printf(" %-10s | ", "Socket");

    /*LINK_SPEED_STATE*/
    printf("\033[4;1H");
    printf(" %-10s | ", "Speed-Dup");

    /*PKTS_PER_SEC_RX*/
    printf("\033[5;1H");
    printf(" %-10s | ", "RX Mpkts/s");

    /*PKTS_PER_SEC_TX*/
    printf("\033[6;1H");
    printf(" %-10s | ", "TX Mpkts/s");

    /*MB_RX*/
    printf("\033[7;1H");
    printf(" %-10s | ", "RX MB");

    /*MB_TX*/
    printf("\033[8;1H");
    printf(" %-10s | ", "TX MB");

    /*PKT_INFO*/
    printf("\033[10;1H");
    printf(" %-25s ", "--- INTF STATS ---");

    /* Dropped */
    printf("\033[11;1H");
    printf(" %-10s | ", "Drp");

    /* RX miss*/
    printf("\033[12;1H");
    printf(" %-10s | ", "RX Miss");

    /* RX Err */
    printf("\033[13;1H");
    printf(" %-10s | ", "RX Err");

    /* RX no Mbuf */
    printf("\033[14;1H");
    printf(" %-10s | ", "RX no MBUF");

    /* TX Err */
    printf("\033[15;1H");
    printf(" %-10s | ", "TX ERR");

    printf("\033[17;1H");
    printf(" %-25s ", "--- PKT STATS ---");

    /*GTPC_V1_TX_IPV4*/
    printf("\033[18;1H");
    printf(" %-10s | ", "TX V1C-4");

    /*GTPC_V1_TX_IPV6*/
    printf("\033[19;1H");
    printf(" %-10s | ", "TX V1C-6");

    /*GTPC_V2_TX_IPV4*/
    printf("\033[20;1H");
    printf(" %-10s | ", "TX V2C-4");

    /*GTPC_V2_TX_IPV6*/
    printf("\033[21;1H");
    printf(" %-10s | ", "TX V2C-6");

    /*GTPC_V1_RX_IPV4*/
    printf("\033[22;1H");
    printf(" %-10s | ", "RX V1C-4");

    /*GTPC_V1_RX_IPV6*/
    printf("\033[23;1H");
    printf(" %-10s | ", "RX V2C-6");

    /*GTPC_V2_RX_IPV4*/
    printf("\033[24;1H");
    printf(" %-10s | ", "RX V2C-4");

    /*GTPC_V2_RX_IPV6*/
    printf("\033[25;1H");
    printf(" %-10s | ", "RX V2C-6");

    /*GTPU_TX_IPV4*/
    printf("\033[26;1H");
    printf(" %-10s | ", "TX U-4");

    /*GTPU_TX_IPV6*/
    printf("\033[27;1H");
    printf(" %-10s | ", "TX U-6");

    /*GTPU_RX_IPV4*/
    printf("\033[28;1H");
    printf(" %-10s | ", "RX U-4");

    /*GTPU_RX_IPV6*/
    printf("\033[29;1H");
    printf(" %-10s | ", "RX U-6");

    /*SURICATA_QUEUE_RX*/
    printf("\033[31;1H");
    printf(" %-10s | ", "RX SUR-Q");

    /*SURICATA_QUEUE_TX*/
    printf("\033[32;1H");
    printf(" %-10s | ", "TX SUR-Q");

    /* fetch port info and display */
    for (i =0 ; i < ports; i++)
    {
       rte_eth_link_get_nowait(i, &link);

        /* DPDK port id - up|down */
        printf("\033[2;%dH", (15 + 10 * i));
        printf("  %d-%s  ", i, link.link_status?"up":"down");

        /*LINK_SPEED_STATE*/
        printf("\033[4;%dH", (15 + 10 * i));
        printf(" %5d-%-2s ", 
               ((link.link_speed  == ETH_LINK_SPEED_10)?10:
                (link.link_speed  == ETH_LINK_SPEED_100)?100:
                (link.link_speed  == ETH_LINK_SPEED_1000)?1000:
                (link.link_speed  == ETH_LINK_SPEED_10000)?10000:0),
               ((link.link_duplex == ETH_LINK_HALF_DUPLEX)?"HD":"FD"));
    }

    fflush(stdout);
    return;
}

void set_stats_timer (void)
{
    int32_t lcoreId = rte_get_master_lcore();

    /* initialize the stats fetch and display timers */
    rte_timer_init(&fetchStats);
    rte_timer_init(&displayStats);

    /* periodic reload for every 1 sec for stats fetch */
    rte_timer_reset(&fetchStats, rte_get_timer_hz(), PERIODICAL, lcoreId, 
                    get_link_stats, NULL);
    /* periodic reload for every 2 sec for stats display */
    rte_timer_reset(&displayStats, rte_get_timer_hz(), PERIODICAL, lcoreId,
                    get_process_stats, NULL);

    return;
}

