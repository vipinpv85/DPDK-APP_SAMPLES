#include "stats.h"

/* GLOBAL */
pkt_stats_t prtPktStats [PKTGEN_MAXPORTS];

static struct rte_timer fetchStats;
static struct rte_timer displayStats;

uint8_t doStatsDisplay = 1;
uint8_t sigusr2Flag = 0;
/* EXTERN */
extern port_config_t config [PKTGEN_MAXPORTS];
extern numa_Info_t numaNodeInfo[MAX_NUMANODE];

void get_link_stats(__attribute__((unused)) struct rte_timer *t, 
                    __attribute__((unused)) void *arg)
{
    int32_t i, ports =  rte_eth_dev_count();
    static uint64_t tx_currStat[PKTGEN_MAXPORTS] = {0};
    static uint64_t tx_prevStat[PKTGEN_MAXPORTS] = {0};

    /* get link status for DPDK ports */
    struct rte_eth_stats stats;

    for (i = 0; i < ports; i++)
    {
        /* ToDo: use numa info to identify the ports */
        if (likely(rte_eth_stats_get(i, &stats) == 0)) {
            tx_currStat[i] =  stats.opackets;

            prtPktStats[i].txPkts = (tx_currStat[i] - tx_prevStat[i]);

            tx_prevStat[i] =  stats.opackets;

            prtPktStats[i].txBytes   = stats.obytes/(1024 * 1024); 
            prtPktStats[i].txErr     = stats.oerrors; 
        }
    }

    return;
}

void get_process_stats(__attribute__((unused)) struct rte_timer *t, 
                       __attribute__((unused)) void *arg)
{
    int32_t i, ports =  rte_eth_dev_count();

    if (likely(doStatsDisplay)) {
     for (i = 0; i < ports; i++)
     {
         /* display dpdk stats from ports */
#if 0
         prtPktStats[i].txPkts
         prtPktStats[i].txErr
#endif

         /* display calculated stats */

         /* INTF STATS */

         /*NUMA_SOCKET*/
         printf("\033[4;%dH", (15 + 10 * i));
         //printf("%-8d |", );

         /*PKTS_PER_SEC_TX*/
         printf("\033[6;%dH", (15 + 10 * i));
         printf("  %-12lu ", prtPktStats[i].txPkts);

         /*MB_TX*/
         printf("\033[7;%dH", (15 + 10 * i));
         printf("  %-12lu ", prtPktStats[i].txBytes);

         /* TX err */
         printf("\033[8;%dH", (15 + 10 * i));
         printf("  %-12lu ", prtPktStats[i].txErr);

         /*  stats */

         /*ARP_TX*/
         printf("\033[11;%dH", (15 + 10 * i));
         printf("  %-12lu ", prtPktStats[i].tx_arp);

         /*ICMP_TX_IPV4*/
         printf("\033[12;%dH", (15 + 10 * i));
         printf("  %-12lu ", prtPktStats[i].tx_icmp4);

         /*ICMP_TX_IPV6*/
         printf("\033[13;%dH", (15 + 10 * i));
         printf("  %-12lu ", prtPktStats[i].tx_icmp6);

         /*IPV4_TX*/
         printf("\033[14;%dH", (15 + 10 * i));
         printf("  %-12lu ", prtPktStats[i].tx_ipv4);

         /*IPV6_TX*/
         printf("\033[15;%dH", (15 + 10 * i));
         printf("  %-12lu ", prtPktStats[i].tx_ipv6);

         /*TCP_TX_IPV4*/
         printf("\033[16;%dH", (15 + 10 * i));
         printf("  %-12lu ", prtPktStats[i].tx_tcp4);
        
         
         /*TCP_TX_IPV6*/
         printf("\033[17;%dH", (15 + 10 * i));
         printf("  %-12lu ", prtPktStats[i].tx_tcp6);

         /*UDP_TX_IPV4*/
         printf("\033[18;%dH", (15 + 10 * i));
         printf("  %-12lu ", prtPktStats[i].tx_udp4);

         /*UDP_TX_IPV6*/
         printf("\033[19;%dH", (15 + 10 * i));
         printf("  %-12lu ", prtPktStats[i].tx_udp6);
       
     }
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
    printf(BOLDBLUE " %-10s | ", "Intf");
    printf("\033[3;1H");
    printf("===============================================" RESET);

    /*NUMA_SOCKET*/
    printf("\033[4;1H");
    printf(GREEN" %-10s | ", "NUMA");

    /*LINK_SPEED_STATE*/
    printf("\033[5;1H");
    printf(" %-10s | ", "Speed-Dup");

    /*PKTS_PER_SEC_TX*/
    printf("\033[6;1H");
    printf(" %-10s | ", "TX pkts/s");

    /*MB_TX*/
    printf("\033[7;1H");
    printf(" %-10s | ", "TX MB");

    /* TX Err */
    printf("\033[8;1H");
    printf(" %-10s | " RESET, "TX ERR");

    printf("\033[10;1H");
    printf(CYAN " %-25s " RESET, "--- TX PKT STATS ---");

    /*ARP*/
    printf("\033[11;1H");
    printf(MAGENTA " %-10s | ", "ARP");

    /*ICMP4*/
    printf("\033[12;1H");
    printf(" %-10s | ", "ICMP4");

    /*ICMP6*/
    printf("\033[13;1H");
    printf(" %-10s | ", "ICMP6");

    /*IPV4*/
    printf("\033[14;1H");
    printf(" %-10s | ", "IPV4");

    /*IPV6*/
    printf("\033[15;1H");
    printf(" %-10s | ", "IPV6");

    /*TCP4*/
    printf("\033[16;1H");
    printf(" %-10s | ", "TCP4");

    /*TCP6*/
    printf("\033[17;1H");
    printf(" %-10s | ", "TCP6");

    /*UDP4*/
    printf("\033[18;1H");
    printf(" %-10s | ", "UDP4");

    /*UDP6*/
    printf("\033[19;1H");
    printf(" %-10s | " RESET, "UDP6");

    /* fetch port info and display */
    for (i =0 ; i < ports; i++)
    {
       rte_eth_link_get_nowait(i, &link);

        /* DPDK port id - up|down */
        printf("\033[2;%dH", (15 + 10 * i));
        if (link.link_status)
            printf("  %d-" GREEN "up" RESET, i);
        else
            printf("  %d-" RED "down" RESET, i);

        /* NUMA SOCKET details */
        printf("\033[4;%dH", (15 + 10 * i));

        /*LINK_SPEED_STATE*/
        printf("\033[5;%dH", (15 + 10 * i));
        printf(" %5d-%-2s ", 
               ((link.link_speed  == ETH_LINK_SPEED_10)?10:
                (link.link_speed  == ETH_LINK_SPEED_100)?100:
                (link.link_speed  == ETH_LINK_SPEED_1000)?1000:
                (link.link_speed  == ETH_LINK_SPEED_10000)?10000:0),
               ((link.link_duplex == ETH_LINK_HALF_DUPLEX)?"HD":"FD"));
    }

    printf("\033[21;1H");
    printf(BOLDGREEN " --- TX PKT BUFF DETAILS ---" RESET);
    printf("\033[22;1H");
    printf(BLUE "=================================================" RESET);
    printf("\033[23;1H");
    printf(BOLDMAGENTA " %-10s ", "INTF");
    printf("\033[24;1H");
    printf(" + %8s |", "Type");
    printf("\033[25;1H");
    printf(" + %8s |", "Index");
    printf("\033[26;1H");
    printf(" + %8s |" RESET, "Size");
    printf("\033[28;2H");
    printf(BOLDGREEN " --- NUMA (INFO) ---  " RESET );
    printf("\033[29;1H");
    printf(BLUE "=================================================" RESET);
    printf("\033[30;1H");
    printf(BOLDRED " %-10s " RESET, "Index");
    printf("\033[31;12H");
    printf(BOLDMAGENTA " --- LCORE-INTF ---" RESET);
    printf(BOLDGREEN"\033[32;1H");
    printf(" + %8s | ", "Avail");
    printf("\033[33;1H");
    printf(" + %8s |", "Used");
    printf("\033[34;1H");
    printf(" + %8s |", "Total");
    printf("\033[35;1H");
    printf(" + %8s |" RESET, "Inuse");
    printf("\033[36;2H");
    printf(BLUE ">> Socket 0 is checked for master core, hence assign 1 extra <<"RESET);

    for (i = 0; i < ports; i ++)
    {
        printf("\033[23;%dH", (15 + 10 * i));
        printf(" %8u ", i);
        printf("\033[24;%dH", (15 + 10 * i));
        printf(" %8s ", (config[i].type == ARP)?"ARP":
                        (config[i].type == ICMP4)?"ICMP4":
                        (config[i].type == ICMP6)?"ICMP6":
                        (config[i].type == IPV4)?"IPV4":
                        (config[i].type == IPV6)?"IPV6":
                        (config[i].type == TCP4)?"TCP4":
                        (config[i].type == TCP6)?"TCP6":
                        (config[i].type == UDP4)?"UDP4":
                        (config[i].type == UDP6)?"UDP6":
                        "Unknown");
        printf("\033[25;%dH", (15 + 10 * i));
        printf(" %8u ", config[i].pktIndex);
        printf("\033[26;%dH", (15 + 10 * i));
        printf(" %8u ", config[i].pktSize);
    }

    for (i=0; i < MAX_NUMANODE; i++)
    {
        printf("\033[30;%dH", (15 + 10 * i));
        printf(" %8u ", i);
        printf("\033[32;%dH", (15 + 10 * i));
        printf(" %4x-%-4x ", numaNodeInfo[i].lcoreAvail, numaNodeInfo[i].intfAvail);
        printf("\033[33;%dH", (15 + 10 * i));
        printf(" %4x-%-4x ", numaNodeInfo[i].lcoreUsed, numaNodeInfo[i].intfUsed);
        printf("\033[34;%dH", (15 + 10 * i));
        printf(" %4u-%-4u ", numaNodeInfo[i].lcoreTotal, numaNodeInfo[i].intfTotal);
        printf("\033[35;%dH", (15 + 10 * i));
        printf(" %4u-%-4u ", numaNodeInfo[i].lcoreInUse, numaNodeInfo[i].intfInUse);
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
    rte_timer_reset_sync(&fetchStats, rte_get_timer_hz(), 
                          PERIODICAL, lcoreId, 
                          get_link_stats, NULL);

    /* periodic reload for every 2 sec for stats display */
    rte_timer_reset_sync(&displayStats, rte_get_timer_hz() * 2, 
                          PERIODICAL, lcoreId,
                          get_process_stats, NULL);

    return;
}

