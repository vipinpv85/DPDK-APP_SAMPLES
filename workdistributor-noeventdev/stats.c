
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
pkt_stats_t prtPktStats;

static struct rte_timer fetchStats = {0};
static struct rte_timer displayStats = {0};

uint8_t doStatsDisplay = 1;

FILE *fp = NULL;

/* EXTERN */
extern numaInfo_t nodeCorePortInfo;
extern uint8_t fifoWrk; 
extern uint8_t queueLvl; 
extern sqmInfo_t sqmInst0;
extern sqmInfo_t sqmInst1;

void sigExtraStats(__attribute__((unused)) int signo)
{
  int32_t i = 0;

  doStatsDisplay = 0;

  /* clear screen */
  STATS_CLR_SCREEN;

  rte_qm_dump(stdout, sqmInst0.qm);
  fflush(fp);
  rte_delay_ms(10000);

  fprintf(fp, "\033[2;1H");
  fprintf(fp, "NUMA INDEX");
  fprintf(fp, "\033[3;1H");
  fprintf(fp, BLUE "*************************************************" RESET);
  fprintf(fp, "\033[5;1H");
  fprintf(fp, "-  1G Cnt:");
  fprintf(fp, "\033[6;1H");
  fprintf(fp, "-  1G Map:");
  fprintf(fp, "\033[7;1H");
  fprintf(fp, "- 10G Cnt:");
  fprintf(fp, "\033[8;1H");
  fprintf(fp, "- 10G Map:");
  fprintf(fp, "\033[9;1H");
  fprintf(fp, "- 40G Cnt:");
  fprintf(fp, "\033[10;1H");
  fprintf(fp, "- 40 Map:");
  fprintf(fp, "\033[11;1H");
  fprintf(fp, "-Core Cnt:");
  fprintf(fp, "\033[12;1H");
  fprintf(fp, "-Core Map:");
  fprintf(fp, "\033[14;1H");
  fprintf(fp, BLUE "*************************************************" RESET);

  for (i=0; i < MAX_NUMANODE; i++)
  {
    fprintf(fp, "\033[2;%dH", (12+ 10 * i));
    fprintf(fp, " %8u ", i);
    fprintf(fp, "\033[5;%dH", (12+ 10 * i));
    fprintf(fp, " %u", nodeCorePortInfo.port1G_count[i]);
    fprintf(fp, "\033[6;%dH", (12+ 10 * i));
    fprintf(fp, " %x", nodeCorePortInfo.port1G_map[i]);
    fprintf(fp, "\033[7;%dH", (12+ 10 * i));
    fprintf(fp, " %u", nodeCorePortInfo.port10G_count[i]);
    fprintf(fp, "\033[8;%dH", (12+ 10 * i));
    fprintf(fp, " %x", nodeCorePortInfo.port1G_map[i]);
    fprintf(fp, "\033[9;%dH", (12+ 10 * i));
    fprintf(fp, " %u", nodeCorePortInfo.port40G_count[i]);
    fprintf(fp, "\033[10;%dH", (12+ 10 * i));
    fprintf(fp, " %x", nodeCorePortInfo.port40G_map[i]);
    fprintf(fp, "\033[11;%dH", (12+ 10 * i));
    fprintf(fp, " %u", nodeCorePortInfo.core_count[i]);
    fprintf(fp, "\033[12;%dH", (20+ 10 * i));
    fprintf(fp, " %"PRIx64, nodeCorePortInfo.core_map[i]);
  }

  fflush(fp);
  rte_delay_ms(10000);

  show_static_display();

  doStatsDisplay = 1;
  return;
}

void sigDetails(__attribute__((unused)) int signo)
{
    struct rte_eth_stats stats;
    struct rte_eth_dev_info devInfo;
    struct rte_eth_link link;
    struct rte_qm_stats qstats; 

    int32_t i, port = rte_eth_dev_count();

    doStatsDisplay = 0;

    /* clear screen */
    STATS_CLR_SCREEN;

    for (i = 0; i < port; i++) {
      fprintf(fp, "\n port index to check %d", i);
      if (((1 << i) & nodeCorePortInfo.port40G_map[0]) || 
          ((1 << i) & nodeCorePortInfo.port40G_map[1]) || 
          ((1 << i) & nodeCorePortInfo.port40G_map[2]) || 
          ((1 << i) & nodeCorePortInfo.port40G_map[3])) {
        /* port Info */
        rte_eth_dev_info_get(i, &devInfo);
        fprintf(fp, "\033[1;1H Intf: %d port: %d driver %s", i, devInfo.if_index, devInfo.driver_name);
        fprintf(fp, "\033[2;1H ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^");
        if (devInfo.pci_dev) {
            fprintf(fp, "\033[3;4H PCI INFO");
            fprintf(fp, "\033[4;6H ADDR: domain:bus:devid.function %04x:%02x:%02x.%x",
                  devInfo.pci_dev->addr.domain,
                  devInfo.pci_dev->addr.bus,
                  devInfo.pci_dev->addr.devid,
                  devInfo.pci_dev->addr.function);
            fprintf(fp, "\033[5;6H PCI ID: vendor:device:sub-vendor:sub-device %04x:%04x:%04x:%04x",
                  devInfo.pci_dev->id.vendor_id,
                  devInfo.pci_dev->id.device_id,
                  devInfo.pci_dev->id.subsystem_vendor_id,
                  devInfo.pci_dev->id.subsystem_device_id);
            fprintf(fp, "\033[6;6H numa node: %d", devInfo.pci_dev->numa_node);
            fprintf(fp, "\033[7;1H ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^");
        }
        
        rte_eth_link_get(i, &link);
        fprintf(fp, "\033[10;1H  Speed: %d",link.link_speed);
        fprintf(fp, "\033[11;1H Duplex: %s",(link.link_duplex == ETH_LINK_FULL_DUPLEX)?"Full":"Half");
        fprintf(fp, "\033[12;1H Status: %s",(link.link_status == 1)?"up":"down");
        fprintf(fp, "\033[13;1H ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^");

        if (likely(rte_eth_stats_get(i, &stats) == 0)) {
            fprintf(fp, "\033[15;1H Packets ");
            fprintf(fp, "\033[16;1H  -  Input: %" PRIu64, stats.ipackets);
            fprintf(fp, "\033[17;1H  - Output: %" PRIu64, stats.opackets);
            fprintf(fp, "\033[18;1H Bytes ");
            fprintf(fp, "\033[19;1H  -  Input: %" PRIu64 "MB", stats.ibytes/(1024 * 1024));
            fprintf(fp, "\033[20;1H  - Output: %" PRIu64 "MB", stats.obytes/(1024 * 1024));
            fprintf(fp, "\033[21;1H Errors: ");
            fprintf(fp, "\033[22;1H  -  Input: %" PRIu64, stats.ierrors);
            fprintf(fp, "\033[23;1H  - Output: %" PRIu64, stats.oerrors);
            fprintf(fp, "\033[24;1H Input Missed: %" PRIu64, stats.imissed);
            fprintf(fp, "\033[25;1H RX no Mbuffs: %" PRIu64, stats.rx_nombuf);
            fprintf(fp, "\033[26;1H ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^");
        }
      }
    }

    if (rte_qm_stats_get(sqmInst0.qm, &qstats) == 0) {
      STATS_CLR_SCREEN;
      fprintf(fp, "\n rx_pkt %"PRIu64, qstats.rx_pkts);
      fprintf(fp, "\n rx_drp %"PRIu64, qstats.rx_dropped);
      fprintf(fp, "\n tx_pkt %"PRIu64, qstats.tx_pkts);

      fprintf(fp, "\n QID");
      for (i = 0; i < 4; i++) 
      {
        fprintf(fp, "\n Qid: %d, pkts RX %"PRIu64", drp %"PRIu64", TX %"PRIu64,
                        i, qstats.qid_rx_pkts[i], qstats.qid_rx_dropped[i], qstats.qid_tx_pkts[i]);
      }

      fprintf(fp, "\n PORTS");
      for (i = 0; i < 12; i++) 
      {
        fprintf(fp, "\n Port: %d\n  rx pkts %"PRIu64" drp %"PRIu64, i, qstats.port_rx_pkts[i], qstats.port_rx_dropped[i]);
        fprintf(fp, "\n   inflight %"PRIu64, qstats.port_inflight[i]);
        fprintf(fp, "\n   TX %"PRIu64, qstats.port_tx_pkts[i]);
      }
    }

    fflush(fp);
    rte_delay_ms(10000);

    show_static_display();

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
      if (((1 << i) & nodeCorePortInfo.port40G_map[0]) || 
          ((1 << i) & nodeCorePortInfo.port40G_map[1]) || 
          ((1 << i) & nodeCorePortInfo.port40G_map[2]) || 
          ((1 << i) & nodeCorePortInfo.port40G_map[3])) {
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

void get_process_stats(__attribute__((unused)) struct rte_timer *t, 
                       __attribute__((unused)) void *arg)
{
    int32_t i, j, k, port = rte_eth_dev_count();

    if (likely(doStatsDisplay)) {
      for (i = 0; i < port; i++)
      {
        if (((1 << i) & nodeCorePortInfo.port40G_map[0]) || 
            ((1 << i) & nodeCorePortInfo.port40G_map[1]) || 
            ((1 << i) & nodeCorePortInfo.port40G_map[2]) || 
            ((1 << i) & nodeCorePortInfo.port40G_map[3]))
        {
          /*PKTS_PER_SEC_RX*/
          fprintf(fp, "\033[5;%dH", (15 + 15 * i));
          fprintf(fp, "  %-13lu ", prtPktStats.port_rxPkts[i]);

          /*PKTS_PER_SEC_TX*/
          fprintf(fp, "\033[6;%dH", (15 + 15 * i));
          fprintf(fp, "  %-13lu ", prtPktStats.port_txPkts[i]);

          /*MB_RX*/
          fprintf(fp, "\033[7;%dH", (15 + 15 * i));
          fprintf(fp, "  %-13lu ", prtPktStats.port_rxBytes[i]);

          /*MB_TX*/
          fprintf(fp, "\033[8;%dH", (15 + 15 * i));
          fprintf(fp, "  %-13lu ", prtPktStats.port_txBytes[i]);

          /* INTF STATS */

          /* RX miss */
          fprintf(fp, "\033[10;%dH", (15 + 15 * i));
          fprintf(fp, "  %-13lu ", prtPktStats.port_rxMissed[i]);

          /* RX err */
          fprintf(fp, "\033[11;%dH", (15 + 15 * i));
          fprintf(fp, "  %-13lu ", prtPktStats.port_rxErr[i]);

          /* TX err */
          fprintf(fp, "\033[12;%dH", (15 + 15 * i));
          fprintf(fp, "  %-12lu ", prtPktStats.port_txErr[i]);

          /* RX no mbuf */
          fprintf(fp, "\033[13;%dH", (15 + 15 * i));
          fprintf(fp, "  %-13lu ", prtPktStats.port_rxNoMbuff[i]);

          /*RX_IPV4*/
          fprintf(fp, "\033[15;%dH", (15 + 15 * i));
          fprintf(fp, "  %-13lu ", prtPktStats.rx_ipv4[i]);

          /*RX_IPV6*/
          fprintf(fp, "\033[16;%dH", (15 + 15 * i));
          fprintf(fp, "  %-13lu ", prtPktStats.rx_ipv6[i]);

          /*ERR NON IP*/
          fprintf(fp, "\033[17;%dH", (15 + 15 * i));
          fprintf(fp, "  %-12lu ", prtPktStats.non_ip[i]);

          /*ERR Dropped*/
          fprintf(fp, "\033[18;%dH", (15 + 15 * i));
          fprintf(fp, "  %-13lu ", prtPktStats.dropped[i]);
        }
      }

      if (sqmInst0.qm) {
        for (j = 0; j < nodeCorePortInfo.port40G_count[0]; j++) {
          fprintf(fp, "\033[%d;2H", (21 + j));
          fprintf(fp, " %-2u, %-3u, %-12"PRIu64", %-12"PRIu64", %-12"PRIu64", %-12"PRIu64", %-12"PRIu64,
                          0, j, 
                          prtPktStats.queue_rx_add[0][j], prtPktStats.queue_rx_drp[0][j], 
                          prtPktStats.rxPkts[0][j], prtPktStats.rxErr[0][j],
                          prtPktStats.queue_rx_fls[0][j]);
        }

        for (j = 0; j < nodeCorePortInfo.port40G_count[0]; j++) {
          fprintf(fp, "\033[%d;2H", (22 + nodeCorePortInfo.port40G_count[0] + j));
          fprintf(fp, " %-2u, %-3u, %-12"PRIu64", %-12"PRIu64", %-12"PRIu64", %-12"PRIu64", %-12"PRIu64,
                          0, j, 
                          prtPktStats.queue_tx_fet[0][j], prtPktStats.queue_tx_drp[0][j], 
                          prtPktStats.txPkts[0][j], prtPktStats.txErr[0][j],
                          prtPktStats.queue_tx_fls[0][j]);
        }

        for (k = 0; k < queueLvl; k++) {
          for (j = 0; j < fifoWrk; j++) {
            /*QUEUE stats*/
            fprintf(fp, "\033[%d;2H", (23 + (2 * nodeCorePortInfo.port40G_count[0]) + k*fifoWrk + j));
            fprintf(fp, " %-2u, %-2u, %-3u, %-12"PRIu64", %-12"PRIu64", %-12"PRIu64", %-12"PRIu64", %-12"PRIu64", %-12"PRIu64, 
                            0, k, j, 
                            prtPktStats.queue_wrk_add[0][k][j], 
                            prtPktStats.queue_wrk_fet[0][k][j], 
                            prtPktStats.queue_wrk_drp[0][k][j], 
                            prtPktStats.wrkPkts[0][k][j], 
                            prtPktStats.wrkErr[0][k][j],
                            prtPktStats.queue_wrk_fls[0][k][j]);
          }
        }
      }
    }

    fflush(fp);
    return;
}

void show_static_display(void)
{
    struct rte_eth_link link;
    int32_t i, ports = rte_eth_dev_count();

    /* clear screen */
    STATS_CLR_SCREEN;

    /* stats header */
    fprintf(fp, "\033[2;1H");
    fprintf(fp, " %-10s | ", "Cat|Intf");
    fprintf(fp, "\033[3;1H");
    fprintf(fp, BLUE "==============================================================================================" RESET);

    /*LINK_SPEED_STATE*/
    fprintf(fp, "\033[4;1H");
    fprintf(fp, BLUE " %-10s | ", "Speed-Dup");

    /*PKTS_PER_SEC_RX*/
    fprintf(fp, "\033[5;1H");
    fprintf(fp, BLUE " %-10s | ", "RX pkts/s");

    /*PKTS_PER_SEC_TX*/
    fprintf(fp, "\033[6;1H");
    fprintf(fp, BLUE " %-10s | ", "TX pkts/s");

    /*MB_RX*/
    fprintf(fp, "\033[7;1H");
    fprintf(fp, BLUE " %-10s | ", "RX MB");

    /*MB_TX*/
    fprintf(fp, "\033[8;1H");
    fprintf(fp, BLUE " %-10s | " RESET, "TX MB");

    /*PKT_INFO*/
    fprintf(fp, "\033[9;1H");
    fprintf(fp, CYAN " %-25s " RESET, "--- INTF STATS ---");

    /* RX miss*/
    fprintf(fp, "\033[10;1H");
    fprintf(fp, RED " %-10s | ", "RX Miss");

    /* RX Err */
    fprintf(fp, "\033[11;1H");
    fprintf(fp, " %-10s | ", "RX Err");

    /* RX no Mbuf */
    fprintf(fp, "\033[12;1H");
    fprintf(fp, " %-10s | ", "RX no MBUF");

    /* TX Err */
    fprintf(fp, "\033[13;1H");
    fprintf(fp, " %-10s | " RESET, "TX ERR");

    fprintf(fp, "\033[14;1H");
    fprintf(fp, CYAN " %-25s " RESET, "--- PKT STATS ---");

    /*RX_IPV4*/
    fprintf(fp, "\033[15;1H");
    fprintf(fp, YELLOW " %-10s | ", "RX IPv4");

    /*RX_IPV6*/
    fprintf(fp, "\033[16;1H");
    fprintf(fp, " %-10s | " RESET, "RX IPv6");

    /*NON IP*/
    fprintf(fp, "\033[17;1H");
    fprintf(fp, BOLDRED " %-10s | ", "NON IP");

    /*DROPPED*/
    fprintf(fp, "\033[18;1H");
    fprintf(fp, " %-10s | " RESET, "DROPPED");

    fprintf(fp, "\033[20;1H");
    fprintf(fp, CYAN " RX - Queue {Node, Port, ADD, DROP, Pkts, Err, flush}");
    fprintf(fp, "\033[%d;1H", (21 + nodeCorePortInfo.port40G_count[0]));
    fprintf(fp, " TX - Queue {Node, Port, FETCH, DROP, Pkts, Err, flush}");
    fprintf(fp, "\033[%d;1H", (22 + 2 * nodeCorePortInfo.port40G_count[0]));
    fprintf(fp, " WRK - Queue {Node, Level, Worker, ADD, FETCH, DROP, Pkts, Err, flush}" RESET);

    /* fetch port info and display */
    for (i =0 ; i < ports; i++)
    {
      /*if (((1 << i) & nodeCorePortInfo.port40G_map[0]) || 
          ((1 << i) & nodeCorePortInfo.port40G_map[1]) || 
          ((1 << i) & nodeCorePortInfo.port40G_map[2]) || 
          ((1 << i) & nodeCorePortInfo.port40G_map[3])) */
      {
        rte_eth_link_get_nowait(i, &link);

        /* DPDK port id - up|down */
        fprintf(fp, "\033[2;%dH", (15 + 10 * i));
        if (link.link_status)
            fprintf(fp, "  %d-" GREEN "up" RESET, i);
        else
            fprintf(fp, "  %d-" RED "down" RESET, i);

        /*LINK_SPEED_STATE*/
        fprintf(fp, "\033[4;%dH", (15 + 10 * i));
        if (link.link_duplex == ETH_LINK_HALF_DUPLEX) {
            fprintf(fp, " %5d-%-2s ", 
               ((link.link_speed  == ETH_LINK_SPEED_10M_HD)?10:
                (link.link_speed  == ETH_LINK_SPEED_100M_HD)?100:
                (link.link_speed  == ETH_LINK_SPEED_1G)?1000:
                (link.link_speed  == ETH_LINK_SPEED_10G)?10000:
                (link.link_speed  == ETH_LINK_SPEED_40G)?40000:
                link.link_speed),
                "HD");
        } else {
            fprintf(fp, " %5d-%-2s ", 
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

    fflush(fp);
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

