/*
 *The MIT License (MIT)
 *Copyright (c) 2016 Vipin Varghese
 *
 *Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation 
 *files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, 
 *modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the 
 *Software is furnished to do so, subject to the following conditions:
 *
 *The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 *
 *THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE 
 *WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR 
 *COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 *ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */


#include "stats.h"

/* GLOBAL */
pkt_stats_t prtPktStats = {0};
wrk_stats_t wrktStats = {0};

static struct rte_timer fetchStats = {0};
static struct rte_timer displayStats = {0};

uint8_t doStatsDisplay = 1;

void sigExtraStats(__attribute__((unused)) int signo)
{
  doStatsDisplay = 0;

  /* clear screen */
  STATS_CLR_SCREEN;

  fflush(stdout);
  rte_delay_ms(10000);

  displayNodeInfo();

  fflush(stdout);
  rte_delay_ms(10000);

  show_static_prt_display();

  doStatsDisplay = 1;
  return;
}

void sigDetails(__attribute__((unused)) int signo)
{
    struct rte_eth_stats stats;
    struct rte_eth_dev_info devInfo;
    struct rte_eth_link link;

    int32_t i, port = rte_eth_dev_count();

    doStatsDisplay = 0;

    /* clear screen */
    STATS_CLR_SCREEN;

    for (i = 0; i < port; i++) {
      printf( "\n port index to check %d", i);
      { /* port Info */
        rte_eth_dev_info_get(i, &devInfo);
        printf( "\033[1;1H Intf: %d port: %d driver %s", i, devInfo.if_index, devInfo.driver_name);
        printf( "\033[2;1H ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^");
        if (devInfo.pci_dev) {
            printf( "\033[3;4H PCI INFO");
            printf( "\033[4;6H ADDR: domain:bus:devid.function %04x:%02x:%02x.%x",
                  devInfo.pci_dev->addr.domain,
                  devInfo.pci_dev->addr.bus,
                  devInfo.pci_dev->addr.devid,
                  devInfo.pci_dev->addr.function);
            printf( "\033[5;6H PCI ID: vendor:device:sub-vendor:sub-device %04x:%04x:%04x:%04x",
                  devInfo.pci_dev->id.vendor_id,
                  devInfo.pci_dev->id.device_id,
                  devInfo.pci_dev->id.subsystem_vendor_id,
                  devInfo.pci_dev->id.subsystem_device_id);
            printf( "\033[6;6H numa node: %d", devInfo.pci_dev->numa_node);
            printf( "\033[7;1H ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^");
        }
        
        rte_eth_link_get(i, &link);
        printf( "\033[10;1H  Speed: %d",link.link_speed);
        printf( "\033[11;1H Duplex: %s",(link.link_duplex == ETH_LINK_FULL_DUPLEX)?"Full":"Half");
        printf( "\033[12;1H Status: %s",(link.link_status == 1)?"up":"down");
        printf( "\033[13;1H ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^");

        if (likely(rte_eth_stats_get(i, &stats) == 0)) {
            printf( "\033[15;1H Packets ");
            printf( "\033[16;1H  -  Input: %" PRIu64, stats.ipackets);
            printf( "\033[17;1H  - Output: %" PRIu64, stats.opackets);
            printf( "\033[18;1H Bytes ");
            printf( "\033[19;1H  -  Input: %" PRIu64 "MB", stats.ibytes/(1024 * 1024));
            printf( "\033[20;1H  - Output: %" PRIu64 "MB", stats.obytes/(1024 * 1024));
            printf( "\033[21;1H Errors: ");
            printf( "\033[22;1H  -  Input: %" PRIu64, stats.ierrors);
            printf( "\033[23;1H  - Output: %" PRIu64, stats.oerrors);
            printf( "\033[24;1H Input Missed: %" PRIu64, stats.imissed);
            printf( "\033[25;1H RX no Mbuffs: %" PRIu64, stats.rx_nombuf);
            printf( "\033[26;1H ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^");
        }
      }
    }

    fflush(stdout);
    rte_delay_ms(10000);

    show_static_prt_display();

    doStatsDisplay = 1;
    return;
}

void get_link_stats(__attribute__((unused)) struct rte_timer *t, 
                    __attribute__((unused)) void *arg)
{
    int32_t i, port = rte_eth_dev_count();

    static uint64_t rx_currStat[MAX_INTFCOUNT] = {0};
    static uint64_t tx_currStat[MAX_INTFCOUNT] = {0};
    static uint64_t rx_prevStat[MAX_INTFCOUNT] = {0};
    static uint64_t tx_prevStat[MAX_INTFCOUNT] = {0};

    /* get link status for DPDK ports */
    struct rte_eth_stats stats;

    for (i = 0; i < port; i++)
    {
      {
        /* ToDo: use numa info to identify the ports */
        if (likely(rte_eth_stats_get(i, &stats) == 0)) {
            rx_currStat[i] =  stats.ipackets;
            tx_currStat[i] =  stats.opackets;

            prtPktStats.port_rxPkts[i] = (rx_currStat[i] - rx_prevStat[i]);
            prtPktStats.port_txPkts[i] = (tx_currStat[i] - tx_prevStat[i]);

            rx_prevStat[i] =  stats.ipackets;
            tx_prevStat[i] =  stats.opackets;

            prtPktStats.port_rxBytes[i]   = stats.ibytes/(1024 * 1024); 
            prtPktStats.port_txBytes[i]   = stats.obytes/(1024 * 1024); 
            prtPktStats.port_rxMissed[i]  = stats.imissed; 
            prtPktStats.port_rxErr[i]     = stats.ierrors; 
            prtPktStats.port_txErr[i]     = stats.oerrors; 
            prtPktStats.port_rxNoMbuff[i] = stats.rx_nombuf; 
        }
      }
    }

    return;
}

void get_process_wrk_stats(void)
{
    int32_t i;

    for (i = 0; i < 4; i++) {
        /* name */
        printf( "\033[2;%dH", (15 + 25 * i));
        printf( "  %-13s", (wrktStats.name[i] == "")?" -- ":wrktStats.name[i]);

        /* rx pkts */
        printf( "\033[5;%dH", (15 + 25 * i));
        printf( "  %-13lu ", wrktStats.wrk_rxPkts[i]);

        /* tx pkts */
        printf( "\033[6;%dH", (15 + 25 * i));
        printf( "  %-13lu ", wrktStats.wrk_txPkts[i]);

        /* rx bytes */
        printf( "\033[7;%dH", (15 + 25 * i));
        printf( "  %-13lu ", wrktStats.wrk_rxBytes[i]);

        /* tx bytes */
        printf( "\033[8;%dH", (15 + 25 * i));
        printf( "  %-13lu ", wrktStats.wrk_txBytes[i]);

        /* rx err */
        printf( "\033[9;%dH", (15 + 25 * i));
        printf( "  %-13lu ", wrktStats.wrk_rxErr[i]);

        /* tx err */
        printf( "\033[10;%dH", (15 + 25 * i));
        printf( "  %-13lu ", wrktStats.wrk_txErr[i]);

        /* drop */
        printf( "\033[11;%dH", (15 + 25 * i));
        printf( "  %-13lu ", wrktStats.wrk_drop[i]);
    } 

    return;
}

void get_process_prt_stats(__attribute__((unused)) struct rte_timer *t, 
                       __attribute__((unused)) void *arg)
{
    struct rte_eth_link link;
    int32_t i, port = rte_eth_dev_count();

    if (likely(doStatsDisplay)) {
      for (i = 0; i < port; i++)
      {
       {
        {
          /*PKTS_PER_SEC_RX*/
          printf( "\033[5;%dH", (15 + 15 * i));
          printf( "  %-13lu ", prtPktStats.port_rxPkts[i]);

          /*PKTS_PER_SEC_TX*/
          printf( "\033[6;%dH", (15 + 15 * i));
          printf( "  %-13lu ", prtPktStats.port_txPkts[i]);

          /*MB_RX*/
          printf( "\033[7;%dH", (15 + 15 * i));
          printf( "  %-13lu ", prtPktStats.port_rxBytes[i]);

          /*MB_TX*/
          printf( "\033[8;%dH", (15 + 15 * i));
          printf( "  %-13lu ", prtPktStats.port_txBytes[i]);

          /* INTF STATS */

          /* RX miss */
          printf( "\033[10;%dH", (15 + 15 * i));
          printf( "  %-13lu ", prtPktStats.port_rxMissed[i]);

          /* RX err */
          printf( "\033[11;%dH", (15 + 15 * i));
          printf( "  %-13lu ", prtPktStats.port_rxErr[i]);

          /* TX err */
          printf( "\033[12;%dH", (15 + 15 * i));
          printf( "  %-12lu ", prtPktStats.port_txErr[i]);

          /* RX no mbuf */
          printf( "\033[13;%dH", (15 + 15 * i));
          printf( "  %-13lu ", prtPktStats.port_rxNoMbuff[i]);

          /*RX_IPV4*/
          printf( "\033[15;%dH", (15 + 15 * i));
          printf( "  %-13lu ", prtPktStats.rx_ipv4[i]);

          /*RX_IPV6*/
          printf( "\033[16;%dH", (15 + 15 * i));
          printf( "  %-13lu ", prtPktStats.rx_ipv6[i]);

          /*ERR NON IP*/
          printf( "\033[17;%dH", (15 + 15 * i));
          printf( "  %-12lu ", prtPktStats.non_ip[i]);

          /*ERR Dropped*/
          printf( "\033[18;%dH", (15 + 15 * i));
          printf( "  %-13lu ", prtPktStats.dropped[i]);
        }
      }
    }

    /* fetch port info and display */
    for (i =0 ; i < port; i++)
    {
      {
        rte_eth_link_get_nowait(i, &link);

        /* DPDK port id - up|down */
        printf( "\033[2;%dH", (15 + 10 * i));
        if (link.link_status)
            printf( "  %d-" GREEN "up" RESET, i);
        else
            printf( "  %d-" RED "down" RESET, i);

        /*LINK_SPEED_STATE*/
        printf( "\033[4;%dH", (15 + 10 * i));
        if (link.link_duplex == ETH_LINK_HALF_DUPLEX) {
            printf( " %5d-%-2s ", 
               ((link.link_speed  == ETH_LINK_SPEED_10M_HD)?10:
                (link.link_speed  == ETH_LINK_SPEED_100M_HD)?100:
                (link.link_speed  == ETH_LINK_SPEED_1G)?1000:
                (link.link_speed  == ETH_LINK_SPEED_10G)?10000:
                (link.link_speed  == ETH_LINK_SPEED_40G)?40000:
                link.link_speed),
                "HD");
        } else {
            printf( " %5d-%-2s ", 
               ((link.link_speed  == ETH_LINK_SPEED_10M)?10:
                (link.link_speed  == ETH_LINK_SPEED_100M)?100:
                (link.link_speed  == ETH_LINK_SPEED_1G)?1000:
                (link.link_speed  == ETH_LINK_SPEED_10G)?10000:
                (link.link_speed  == ETH_LINK_SPEED_40G)?40000:
                link.link_speed),
                "FD");
        }
      }
    }

    fflush(stdout);
    return;
  }
}

void show_static_prt_display(void)
{
    /* clear screen */
    STATS_CLR_SCREEN;

    /* stats header */
    printf( "\033[2;1H");
    printf( " %-10s | ", "Cat|Intf");
    printf( "\033[3;1H");
    printf( BLUE "==============================================================================================" RESET);

    /*LINK_SPEED_STATE*/
    printf( "\033[4;1H");
    printf( BLUE " %-10s | ", "Speed-Dup");

    /*PKTS_PER_SEC_RX*/
    printf( "\033[5;1H");
    printf( BLUE " %-10s | ", "RX pkts/s");

    /*PKTS_PER_SEC_TX*/
    printf( "\033[6;1H");
    printf( BLUE " %-10s | ", "TX pkts/s");

    /*MB_RX*/
    printf( "\033[7;1H");
    printf( BLUE " %-10s | ", "RX MB");

    /*MB_TX*/
    printf( "\033[8;1H");
    printf( BLUE " %-10s | " RESET, "TX MB");

    /*PKT_INFO*/
    printf( "\033[9;1H");
    printf( CYAN " %-25s " RESET, "--- INTF STATS ---");

    /* RX miss*/
    printf( "\033[10;1H");
    printf( RED " %-10s | ", "RX Miss");

    /* RX Err */
    printf( "\033[11;1H");
    printf( " %-10s | ", "RX Err");

    /* RX no Mbuf */
    printf( "\033[12;1H");
    printf( " %-10s | ", "RX no MBUF");

    /* TX Err */
    printf( "\033[13;1H");
    printf( " %-10s | " RESET, "TX ERR");

    printf( "\033[14;1H");
    printf( CYAN " %-25s " RESET, "--- PKT STATS ---");

    /*RX_IPV4*/
    printf( "\033[15;1H");
    printf( YELLOW " %-10s | ", "RX IPv4");

    /*RX_IPV6*/
    printf( "\033[16;1H");
    printf( " %-10s | " RESET, "RX IPv6");

    /*NON IP*/
    printf( "\033[17;1H");
    printf( BOLDRED " %-10s | ", "NON IP");

    /*DROPPED*/
    printf( "\033[18;1H");
    printf( " %-10s | " RESET, "DROPPED");

    fflush(stdout);
    return;
}

void show_static_wrk_display(void)
{
    /* clear screen */
    STATS_CLR_SCREEN;

    /* stats header */
    printf( "\033[2;1H");
    printf( " %-10s | ", "Cat|Wrkr");
    printf( "\033[3;1H");
    printf( BLUE "==============================================================================================" RESET);

    /*MB_TX*/
    printf( "\033[5;1H");
    printf( BLUE " %-10s | " , "RX pkts");

    /*MB_TX*/
    printf( "\033[6;1H");
    printf( BLUE " %-10s | " , "TX pkts");

    /*MB_TX*/
    printf( "\033[7;1H");
    printf( BLUE " %-10s | " , "RX MB");

    /*MB_TX*/
    printf( "\033[8;1H");
    printf( BLUE " %-10s | " RESET, "TX MB");

    /*MB_TX*/
    printf( "\033[9;1H");
    printf( RED " %-10s | " , "RX ERR");

    /*MB_TX*/
    printf( "\033[10;1H");
    printf( RED " %-10s | " , "TX ERR");

    /*MB_TX*/
    printf( "\033[11;1H");
    printf( RED " %-10s | " RESET, "DROP");

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
                    get_process_prt_stats, NULL);

    return;
}

