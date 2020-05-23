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

#include "config.h"
#include "node.h"
#include "stats.h"

#include <getopt.h>

/*default port configuration*/
static struct rte_eth_conf port_conf_default = {
    .rxmode = {
        .split_hdr_size = 0,
        .header_split   = 0,    /**< Header Split disabled. */
        .hw_ip_checksum = 0,    /**< IP checksum offload disabled. */
        .hw_vlan_filter = 0,    /**< VLAN filtering enabled. */
        .hw_vlan_strip  = 0,    /**< VLAN strip enabled. */
        .hw_vlan_extend = 0,    /**< Extended VLAN disabled. */
        .jumbo_frame    = 0,    /**< Jumbo Frame Support disabled. */
        .hw_strip_crc   = 0,    /**< CRC stripping by hardware disabled. */
    },
    .txmode = {
        .mq_mode = ETH_MQ_TX_NONE,
    },
};

numaInfo_t  *nodePtr = NULL;

/*SW ring which acts as FIFO for worker threads*/

/*LKP Result*/
struct rte_hash* ipv4_hash_table[4] = {NULL};
struct rte_hash* ipv6_hash_table[4] = {NULL};

/* E X T E R N */
extern pkt_stats_t prtPktStats;
extern wrk_stats_t wrktStats;

/* 
 * Initialize a given ring port
 */
static inline int fifo_init(uint8_t fifoId, uint8_t numaNode)
{
  char srbName [64] = {0};
  struct rte_ring *srb = NULL;

  if (fifoId >= MAX_FIFO) {
    fprintf(stdout, "\n ERROR: FIFO: %d invalid!!!\n", fifoId);
    return -1;
  }

  sprintf(srbName, "SW-FIFO-%d-%d", numaNode,fifoId);
  fprintf(stdout, "\n DEBUG: fifo index:%d one Node %d %s", fifoId, numaNode, srbName);

  srb = rte_ring_create(srbName, RTE_RING_SIZE, numaNode, 0);
  if (NULL == srb) {
    rte_exit(EXIT_FAILURE, " Cannot create output ring %s on node %d\n", srbName, numaNode);
  }

  return 0;
}

/*
 * Initializes a given port using global settings and with the RX buffers
 * coming from the mbuf_pool passed as a parameter.
 */
static inline int port40G_init(uint8_t port, uint8_t numaNode)
{
    char mempoolName[64] = {0};
    struct rte_mempool *mbuff_pool = NULL;

    struct rte_eth_conf port_conf = port_conf_default;
    port_conf.link_speeds = ETH_LINK_SPEED_40G;

    int retval, i;
    
    struct rte_eth_dev_info devInfo;
    
    rte_eth_dev_stop(port);
    fprintf(stdout, "\n\n INFO: setting up port %d on node %d\n", port, numaNode);
    
    if (port >= rte_eth_dev_count())
      return -1;
    
    rte_eth_dev_info_get(port, &devInfo);

    /* allocate mempool TX and RX seperate */
    sprintf(mempoolName, "40G-%d-%d-%d-rx", devInfo.pci_dev->addr.function, port, numaNode);

    mbuff_pool = rte_mempool_create(mempoolName, NUM_MBUFS,
                                    MBUF_CACHE_SIZE, 0,
                                    RTE_MBUF_DEFAULT_BUF_SIZE, 
                                    rte_pktmbuf_pool_init, NULL,
                                    rte_pktmbuf_init, NULL,
                                    numaNode,
                                    MEMPOOL_F_SP_PUT | MEMPOOL_F_SC_GET);
    if (unlikely(mbuff_pool == NULL)) {
        rte_panic("\n ERROR: failed to get mem-pool for rx on node 0 intf %d on node %d\n", port, numaNode);
        return -1;
    }
    //fprintf(stdout, "\n INFO: created rx & tx mempool!!");

    /* Configure the Ethernet device. */
    retval = rte_eth_dev_configure(port, 1, 1, &port_conf);
    if (retval != 0)
      return retval;
    //fprintf(stdout, "\n INFO: configured port with 1 rx and 1 tx queues!!");

    /* Allocate and set up 1 RX queue per Ethernet port. */
    retval = rte_eth_rx_queue_setup(port, 0, RX_RING_SIZE, numaNode, NULL, mbuff_pool);
    if (retval < 0) {
      fprintf(stdout, "\n DEBUG: RX queue ret %d for intf %d on node %d!!", retval, port, numaNode);
      return retval;
    }
    //fprintf(stdout, "\n INFO: rx queue 0 setup!!");
    
    /* Allocate and set up 1 TX queue per Ethernet port. */
    for (i = 0; i < 1; i++) 
    {
      retval = rte_eth_tx_queue_setup(port, i, TX_RING_SIZE,
      			numaNode, NULL);
      if (retval < 0) {
        fprintf(stdout, "\n DEBUG: TX queue ret %d for intf %d on node %d!!", retval, port, numaNode);
        return retval;
      }
    }

    //fprintf(stdout, "\n INFO: tx queue 0 setup!!");
    
    /* Enable RX in promiscuous mode for the Ethernet device. */
    rte_eth_promiscuous_enable(port);
    
    /* Start the Ethernet port. */
    retval = rte_eth_dev_start(port);
    if (retval < 0)
    	return retval;
    
  return 0;
}

static inline int port10G_init(uint8_t port, uint8_t numaNode)
{
    char mempoolName[64] = {0};
    struct rte_mempool *mbuff_pool = NULL;

    struct rte_eth_conf port_conf = port_conf_default;
    port_conf.link_speeds = ETH_LINK_SPEED_10G;

    int retval, i;
    
    struct rte_eth_dev_info devInfo;
    
    rte_eth_dev_stop(port);
    fprintf(stdout, "\n\n INFO: setting up port %d on node %d\n", port, numaNode);
    
    if (port >= rte_eth_dev_count())
      return -1;
    
    rte_eth_dev_info_get(port, &devInfo);

    /* allocate mempool TX and RX seperate */
    sprintf(mempoolName, "10G-%d-%d-%d-rx", devInfo.pci_dev->addr.function, port, numaNode);

    mbuff_pool = rte_mempool_create(mempoolName, NUM_MBUFS,
                                    MBUF_CACHE_SIZE, 0,
                                    RTE_MBUF_DEFAULT_BUF_SIZE, 
                                    rte_pktmbuf_pool_init, NULL,
                                    rte_pktmbuf_init, NULL,
                                    numaNode,
                                    MEMPOOL_F_SP_PUT | MEMPOOL_F_SC_GET);
    if (unlikely(mbuff_pool == NULL)) {
        rte_panic("\n ERROR: failed to get mem-pool for rx on node 0 intf %d on node %d\n", port, numaNode);
        return -1;
    }
    //fprintf(stdout, "\n INFO: created rx & tx mempool!!");

    /* Configure the Ethernet device. */
    retval = rte_eth_dev_configure(port, 1, 1, &port_conf);
    if (retval != 0)
      return retval;
    //fprintf(stdout, "\n INFO: configured port with 1 rx and 1 tx queues!!");

    /* Allocate and set up 1 RX queue per Ethernet port. */
    retval = rte_eth_rx_queue_setup(port, 0, RX_RING_SIZE, numaNode, NULL, mbuff_pool);
    if (retval < 0) {
      fprintf(stdout, "\n DEBUG: RX queue ret %d for intf %d on node %d!!", retval, port, numaNode);
      return retval;
    }
    //fprintf(stdout, "\n INFO: rx queue 0 setup!!");
    
    /* Allocate and set up 1 TX queue per Ethernet port. */
    for (i = 0; i < 1; i++) 
    {
      retval = rte_eth_tx_queue_setup(port, i, TX_RING_SIZE,
      			numaNode, NULL);
      if (retval < 0) {
        fprintf(stdout, "\n DEBUG: TX queue ret %d for intf %d on node %d!!", retval, port, numaNode);
        return retval;
      }
    }

    //fprintf(stdout, "\n INFO: tx queue 0 setup!!");
    
    /* Enable RX in promiscuous mode for the Ethernet device. */
    rte_eth_promiscuous_enable(port);
    
    /* Start the Ethernet port. */
    retval = rte_eth_dev_start(port);
    if (retval < 0)
    	return retval;
    
  return 0;
}


/*
 * This is the main therad that does the work, reading from
 * the FIFO queue and process the pkts for transmit ports.
 */
static int lcore_fifoTransmit(void *arg)
{
  uint8_t fifoIndex = *((uint8_t *) arg);
  uint16_t nb_tx = 0;
  int32_t ret = 0, i = 0;
  char srbName[25] = {0};
  struct rte_ring *srb = NULL;

  uint32_t socketId = rte_lcore_to_socket_id(rte_lcore_id());


  sprintf(srbName, "SW-FIFO-%d-%d", socketId, fifoIndex);
  memcpy((void *)&(wrktStats.name[fifoIndex]), (void *)srbName, 25);
  srb = rte_ring_lookup(srbName);
  if (unlikely(srb == NULL)) {
    rte_exit(EXIT_FAILURE, "\n unable find the FIFO RING for node %d id %d", socketId, fifoIndex);
  }

  printf("\n fifo index %d srbName %s name %s", fifoIndex, srbName, wrktStats.name[fifoIndex]);
  printf("\n INFO: FIFO: %d on socket %d lcore %d", fifoIndex, socketId, rte_lcore_id());
  fflush(stdout);

  while (1) {
#if 0
    struct rte_mbuf *ptr[8] = {NULL};
    //ret = rte_ring_sc_dequeue_bulk(srb[fifoIndex], (void *)&ptr, 8);
    ret = rte_ring_dequeue_burst(srb, (void *)&ptr, 8);
    //fprintf(stdout, "\n dequeued packets: %d", ret);

    if (likely(ret > 0)) {
      /*transmit pkt 1 by 1*/
      for (i =0; i < ret; i++) {
        nb_tx = rte_eth_tx_burst(ptr[i]->port, 0, (struct rte_mbuf **)&(ptr[i]), 1);
        if (unlikely(nb_tx != 1)) {
            rte_pktmbuf_dump(stdout, ptr[i], 8);
            rte_pktmbuf_free(ptr[i]);
        }
      }
    }
#else
    struct rte_mbuf *ptr[8] = {NULL};
    //ret = rte_ring_sc_dequeue_bulk(srb[fifoIndex], (void *)&ptr, 8);
    ret = rte_ring_dequeue_burst(srb, (void *)&ptr, 8);
    //fprintf(stdout, "\n dequeued packets: %d", ret);

    if (likely(ret > 0)) {
      wrktStats.wrk_rxPkts[fifoIndex] += ret;
      /* Send burst of TX packets, to second port of pair. */
      const uint16_t nb_tx = rte_eth_tx_burst(ptr[0]->port, 0, ptr, ret);

      wrktStats.wrk_txPkts[fifoIndex] += nb_tx;
      /* Free any unsent packets. */
      if (unlikely(nb_tx < ret)) {
        wrktStats.wrk_drop[fifoIndex] += (ret - nb_tx);
        uint16_t buf;
        for (buf = nb_tx; buf < ret; buf++)
          rte_pktmbuf_free(ptr[buf]);
      }
    }
#if 0
    struct rte_mbuf *ptr = NULL;

    ret = rte_ring_dequeue(srb, &ptr);
    if (likely(ret == 0)) {
        nb_tx = rte_eth_tx_burst(ptr->port, 0, &ptr, 1);
        if (unlikely(nb_tx != 1)) {
            rte_pktmbuf_dump(stdout, ptr, 8);
            rte_pktmbuf_free(ptr);
        }
    }
#endif
#endif
  }

  return 0;
}

/*
 * This is the main thread that does the work, reading from
 * an input port and hashing to FIFO queues.
 */
static int lcore_pktFetch(void *arg)
{
    uint8_t rxport = *((uint8_t *) arg);
    uint8_t fifoIndex = 0;

    int32_t ret = 0, j = 0;

    int8_t core_socketId = rte_lcore_to_socket_id(rte_lcore_id());
    int8_t port_socketId = rte_eth_dev_socket_id(rxport);
    port_socketId = (port_socketId == -1)?0:port_socketId;

    struct rte_eth_link rxLink;

    struct rte_mbuf *m = NULL;

    struct ether_hdr *ethHdr = NULL;
    struct ipv4_hdr  *ipHdr  = NULL;
  
    char srbName [25];
    struct rte_ring *srb     = NULL;

    fprintf(stdout, "\n INFO: %s port RX: %d on socket %d lcore %d socket %d ", \
            __func__, rxport, port_socketId, rte_lcore_id(), core_socketId);

    struct rte_mbuf *bufs[BURST_SIZE];

    if (port_socketId != core_socketId)
            fprintf(stdout, "--- WARNING, RX port %u is on remote NUMA node to polling thread.\n Performance will not be optimal. ---", rxport);

    rte_eth_link_get(rxport, &rxLink);
    if (rxLink.link_status == ETH_LINK_DOWN) {
        fprintf(stderr, "\n WARN: port %u - Down", rxport);
        fflush(stdout); /*flsuh all details before we start the thread*/

        return 0;
    }

    sprintf(srbName, "SW-FIFO-%d-%d", core_socketId, rxport);
    srb = rte_ring_lookup(srbName);
    if (unlikely(srb == NULL))
      rte_panic("Failed to fetch launch worker for node %d fifo %d\n", core_socketId, fifoIndex);

    printf("\n port %d srbName %s", rxport, srbName);
    fflush(stdout); /*flsuh all details before we start the thread*/

    while (1) {
      const int16_t nb_rx = rte_eth_rx_burst(rxport, 0, bufs, BURST_SIZE);
      if (unlikely(nb_rx == 0)) {
        continue;
      }

#if 0
      /* Send burst of TX packets, to second port of pair. */
      const uint16_t nb_tx = rte_eth_tx_burst((rxport ^ 1), 0, bufs, nb_rx);

      /* Free any unsent packets. */
      if (unlikely(nb_tx < nb_rx)) {
        uint16_t buf;
        for (buf = nb_tx; buf < nb_rx; buf++)
          rte_pktmbuf_free(bufs[buf]);
      }
#else
      //fprintf(stdout, "\n INFO: recv pkt count: %d", nb_rx);
      //rte_pktmbuf_dump (stdout, bufs[0], 64); /*dump the first packet content*/

      /* prefetch packets for pipeline */
      for (j = 0; ((j < PREFETCH_OFFSET) && (j < nb_rx)); j++)
      {
        rte_prefetch0(rte_pktmbuf_mtod(bufs[j], void *));
      } /*for loop till PREFETCH_OFFSET*/

      for (j = 0; j < (nb_rx - PREFETCH_OFFSET); j++) /* Prefetch others packets */
      {
        m = bufs[j];
        rte_prefetch0(rte_pktmbuf_mtod(bufs[j + PREFETCH_OFFSET], void *));

        ethHdr = rte_pktmbuf_mtod(m, struct ether_hdr*);
        //fprintf(stdout, "\n ether type : %x\n", ethHdr->ether_type);

        //if (likely(ethHdr->ether_type == 0x0008)) {
        if (1) {
          prtPktStats.non_ip[rxport] += 1;
          //fprintf(stdout, "\n IP src:%x dst: %x", ipHdr->src_addr, ipHdr->dst_addr);

          /*check if the FIFO is free; check for other FIFO*/
          if (rte_ring_full(srb) == 0) {
            ret = rte_ring_mp_enqueue(srb, m);
            if (likely(ret == 0)) { 
              rte_pktmbuf_free(m);
              prtPktStats.dropped[rxport] += 1;
            }
            continue;
          }
          else {
              rte_pktmbuf_free(m);
              prtPktStats.dropped[rxport] += 1;
          }

        }
        else {
         rte_pktmbuf_free(m); /*Free the mbuff*/
        }
      }

      for (; j < nb_rx; j++)
      {
        m = bufs[j];

        ethHdr = rte_pktmbuf_mtod(m, struct ether_hdr*);
        //fprintf(stdout, "\n ether type : %x\n", ethHdr->ether_type);

        //if (likely(ethHdr->ether_type == 0x0008)) {
        if (1) {
          prtPktStats.non_ip[rxport] += 1;
          ipHdr = (struct ipv4_hdr *) ((char *)(ethHdr + 1));
          //fprintf(stdout, "\n IP src:%x dst: %x", ipHdr->src_addr, ipHdr->dst_addr);

          /*check if the FIFO is free; check for other FIFO*/
          if (rte_ring_full(srb) == 0) {
            ret = rte_ring_mp_enqueue(srb, m);
            if (likely(ret == 0)) { 
              rte_pktmbuf_free(m);
              prtPktStats.dropped[rxport] += 1;
            }
            continue;
          }
          else {
              rte_pktmbuf_free(m);
              prtPktStats.dropped[rxport] += 1;
          }

        }
        else {
         rte_pktmbuf_free(m); /*Free the mbuff*/
        }
      }
#endif
    }  

    return 0;
}

static int display(void)
{
    do {
      rte_delay_ms(1000);
      rte_timer_manage();
    }while(1);

    return 0;
}


/*
 * The main function, which does initialization and calls the per-lcore functions.
 */
int main(int argc, char *argv[])
{
  signed nb_ports, i, j;
  unsigned wlcore, index = 0;
  uint8_t portid = 0, argPort = 0;
  uint16_t tempPortMask;
  uint64_t tempCoreMask;

  const struct rte_memzone *mz = NULL;

  /*check if dpdk version is greater than or equal 16.07 to support SQM*/
  fprintf(stdout, "\n INFO: DPDK Ver: %s\n", rte_version());

  /* Initialize the Environment Abstraction Layer (EAL). */
  int ret = rte_eal_init(argc, argv);
  if (ret < 0)
  	rte_exit(EXIT_FAILURE, "with EAL initialization\n");
  
  argc -= ret;
  argv += ret;

  if (rte_eal_process_type() == RTE_PROC_PRIMARY) {
      fprintf(stdout, "Inside Primary!!");

      mz = rte_memzone_reserve("nodeinfo", sizeof(numaInfo_t), rte_socket_id(), 0);
      if (mz == NULL)
          rte_exit(EXIT_FAILURE, "Cannot reserve memory zone for node information\n");
      nodePtr = (numaInfo_t *)mz->addr;

      /* Register Signal */
      signal(SIGUSR1, sigExtraStats);
      signal(SIGUSR2, sigDetails);

      /*node info build*/
      if (populateNodeInfo(nodePtr) < 0)
        rte_exit(EXIT_FAILURE, "NUMA info build \n");

      displayNodeInfo();

      /*iterate ports to identify the 40G interface*/
      if ((getCount40gPorts(nodePtr) == 0) && (getCount10gPorts(nodePtr) == 0))
        rte_exit(EXIT_FAILURE, "Error: Ports for 40G or 10G are not available\n");

      /*
       * Initalizae the 40G & 10G ports under DPDK
       */
      nb_ports = rte_eth_dev_count();
      for (i = 0; i < nb_ports; i++) 
      {
        if ((1 << i) & nodePtr->port40G_map[0]) {
          argPort = i;
          if (port40G_init(argPort, 0) != 0)
            rte_exit(EXIT_FAILURE, "Cannot init port %"PRIu8 "on node 0\n", argPort);
        }
        else if ((1 << i) & nodePtr->port40G_map[1]) {
          argPort = i;
          if (port40G_init(argPort, 0) != 0)
            rte_exit(EXIT_FAILURE, "Cannot init port %"PRIu8 "on node 1\n", argPort);
        }
            else if ((1 << i) & nodePtr->port10G_map[0]) {
          argPort = i;
          if (port10G_init(argPort, 0) != 0)
            rte_exit(EXIT_FAILURE, "Cannot init port %"PRIu8 "on node 0\n", argPort);
        }
        else if ((1 << i) & nodePtr->port10G_map[1]) {
          argPort = i;
          if (port10G_init(argPort, 0) != 0)
            rte_exit(EXIT_FAILURE, "Cannot init port %"PRIu8 "on node 1\n", argPort);
        }
      }
      fprintf(stdout, "\nINFO: Initialized 40G & 10 ports\n");

      /*create the sw ring bufferes to carry data to workers*/
      for (portid = 0; portid < getCount40gPorts(nodePtr) + getCount10gPorts(nodePtr); portid++) {
        fprintf(stdout, "\nnode %d fifoworker %d", index, portid);
        argPort = portid;
        if (fifo_init(argPort, index) != 0)
          rte_exit(EXIT_FAILURE, "Cannot init SW RING %"PRIu8 " on node %d\n", argPort, index);
      }
  }

  mz = rte_memzone_lookup("nodeinfo");
  if (mz == NULL)
      rte_exit(EXIT_FAILURE, "Cannot fetch memory zone for node information\n");
  nodePtr = (numaInfo_t *)mz->addr;

  for (index = 0; index < 4; index++)
  {
    wlcore = -1;
    argPort = 0; 

    if (nodePtr->port40G_count[index] | nodePtr->port10G_count[index]) {
      tempPortMask = nodePtr->port40G_map[index] | nodePtr->port10G_map[index];
      tempCoreMask = nodePtr->core_map[index];

      if (rte_eal_process_type() == RTE_PROC_SECONDARY) {
        if (nodePtr->core_count[index] >= (getCount40gPorts(nodePtr) + getCount10gPorts(nodePtr)+ 1)) {
          /*start the workers*/
          j = 0;
          wlcore = 0;
          RTE_LCORE_FOREACH(wlcore) {
            if (j >= getCount40gPorts(nodePtr) + getCount10gPorts(nodePtr))
              break;

            if (rte_lcore_to_socket_id(wlcore) != index) {
              fprintf(stdout, "\n DBG: lcore %d does not reside CPU socket %d, fetch next", wlcore, index);
              continue;
            }
            else if (rte_eal_get_lcore_state(wlcore) == RUNNING) {
              fprintf(stdout, "\n DBG: lcore %d is already running, fetch next", wlcore);
              continue;
            }
            else if (rte_get_master_lcore() == wlcore) {
              fprintf(stdout, "\n DBG: lcore %d is master core, fetch next", wlcore);
              continue;
            }

            argPort = j;
            fprintf(stdout, "\n INFO: core %d for worker %d", wlcore, argPort);
            ret = rte_eal_remote_launch(lcore_fifoTransmit, (void *) &argPort, wlcore);
            if (ret)
              rte_panic("Failed to launch worker\n");

            fprintf(stdout, "\n lcore_fifoTransmit lcore %d with worker %d", wlcore, j);
            j++;

            fflush(stdout);
          }
        }
        else {
          fprintf(stderr, "\n ERR: insufficent cores to start the worker threads");
          ret = -1;
        }
      } 
      else if (rte_eal_process_type() == RTE_PROC_PRIMARY) {
        if (nodePtr->core_count[index] >= (nodePtr->port10G_count[index] + nodePtr->port40G_count[index] + 1)) { 
          /*start the ports*/

          argPort = 0;
          for (int32_t j = 0; j < (nodePtr->port40G_count[index] + nodePtr->port10G_count[index]); j++)
          {
            while ((tempPortMask & (1<< argPort)) == 0) {
              argPort += 1;
            }
            wlcore += 1;
            while ((tempCoreMask & (1 << wlcore)) == 0) {
              wlcore += 1;
            }

            fprintf(stdout, "\n INFO: port %d core %d!!", argPort, wlcore);
            ret = rte_eal_remote_launch(lcore_pktFetch, (void *) &argPort, wlcore);
            if (ret != 0)
              rte_exit(EXIT_FAILURE, "\n unable to launch on core id %d!!", wlcore);
            fprintf(stdout, "\n Initialized lcore %d with port %d", wlcore, argPort);

            rte_delay_ms(2000);
            argPort += 1;
            fflush(stdout);
          }
        }
        else {
          fprintf(stderr, "\n ERR: insufficent cores to start the worker threads");
          ret = -1;
        }
      }

    } /* check if itnerfaces are present*/
  } /* iterate for 4 multi socket*/

   if (rte_eal_process_type() == RTE_PROC_PRIMARY) {
     if (ret == 0) {
       STATS_CLR_SCREEN;

       set_stats_timer();
       show_static_prt_display();
      
       while (1)
        display();
     }
   }
   else {
     if (ret == 0) {
       rte_delay_ms(2000);
       STATS_CLR_SCREEN;

       show_static_wrk_display();

       while(1) {
         get_process_wrk_stats();
         rte_delay_ms(1000);
       }
     }
   }

   fprintf(stderr, "failed to start!!!!");

   return 0;
}

