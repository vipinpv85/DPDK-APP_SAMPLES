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

#include <rte_jhash.h>

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

/*config parameters*/
uint8_t fifoWrk = 8; /*fifo worker threads pkt processing*/

static struct rte_hash_parameters ipv4_hash_params = {
    .socket_id = 0,
    .name = "ipv4-hash",
    .entries = 8, /*keep entries in multiples of 8*/
    .key_len = 8,
    .hash_func = rte_jhash,
    .hash_func_init_val = 0,
};

static struct rte_hash_parameters ipv6_hash_params = {
    .socket_id = 0,
    .name = "ipv6-hash",
    .entries = 8,
    .key_len = 32,
    .hash_func = rte_jhash,
    .hash_func_init_val = 0,
};



/*SW ring which acts as FIFO for worker threads*/

/*LKP Result*/
struct rte_hash* ipv4_hash_table[4] = {NULL};
struct rte_hash* ipv6_hash_table[4] = {NULL};

/* E X T E R N */
extern pkt_stats_t prtPktStats;
extern numaInfo_t  nodeCorePortInfo;
	
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
  printf(stdout, "\n DEBUG: fifo index:%d one Node %d %s", fifoId, numaNode, srbName);

  srb = rte_ring_create(srbName, RTE_RING_SIZE, numaNode, 0);
  if (NULL == srb) {
    fprintf(stdout, "ERROR: Cannot create output ring %s on node %d\n", srbName, numaNode);
    return -2;
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
    retval = rte_eth_dev_configure(port, 1, fifoWrk, &port_conf);
    if (retval != 0)
      return retval;
    //fprintf(stdout, "\n INFO: configured port with 1 rx and 1 tx queues!!");

    /* Allocate and set up 1 RX queue per Ethernet port. */
    retval = rte_eth_rx_queue_setup(port, 0, RX_RING_SIZE, 
                                    numaNode, NULL, mbuff_pool);
    if (retval < 0) {
      fprintf(stdout, "\n DEBUG: RX queue ret %d for intf %d on node %d!!", retval, port, numaNode);
      return retval;
    }
    //fprintf(stdout, "\n INFO: rx queue 0 setup!!");
    
    /* Allocate and set up 1 TX queue per Ethernet port. */
    for (i = 0; i < fifoWrk; i++) 
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
    retval = rte_eth_dev_configure(port, 1, fifoWrk, &port_conf);
    if (retval != 0)
      return retval;
    //fprintf(stdout, "\n INFO: configured port with 1 rx and 1 tx queues!!");

    /* Allocate and set up 1 RX queue per Ethernet port. */
    retval = rte_eth_rx_queue_setup(port, 0, RX_RING_SIZE, 
                                    numaNode, NULL, mbuff_pool);
    if (retval < 0) {
      fprintf(stdout, "\n DEBUG: RX queue ret %d for intf %d on node %d!!", retval, port, numaNode);
      return retval;
    }
    //fprintf(stdout, "\n INFO: rx queue 0 setup!!");
    
    /* Allocate and set up 1 TX queue per Ethernet port. */
    for (i = 0; i < fifoWrk; i++) 
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
  uint8_t destport = 0;
  uint16_t nb_tx = 0;
  int32_t ret = 0, i = 0;
  uint32_t socketId = rte_lcore_to_socket_id(rte_lcore_id());

  struct rte_mbuf *ptr[8] = {NULL}, *m = NULL;

  char srbName[25] = {0};
  struct rte_ring *srb = NULL;

  sprintf(srbName, "SW-FIFO-%d-%d", socketId, fifoIndex);
  fprintf(stdout, "\n INFO: %s %s FIFO: %d on socket %d lcore %d", \
          __func__, srbName, fifoIndex, socketId, rte_lcore_id());

  srb = rte_ring_lookup(srbName);
  if (unlikely(srb == NULL)) {
    rte_panic("\n ERROR: unable find the FIFO RING for node %d id %d", socketId, fifoIndex);
  }

  while (1) {
    //ret = rte_ring_sc_dequeue_bulk(srb[fifoIndex], (void *)&ptr, 8);
    ret = rte_ring_dequeue_burst(srb, (void *)&ptr, 8);
    if (likely(ret > 0)) {
      prtPktStats.queue_wrk_fet[socketId][fifoIndex] += ret;

      /*transmit pkt 1 by 1*/
      for (i =0; i < ret; i++) {
        destport = (ptr[i]->port);
        nb_tx = rte_eth_tx_burst(destport, fifoIndex, &ptr[i], 1);
        if (unlikely(nb_tx != 1)) {
            /*rte_pktmbuf_dump(stdout, hdr[t], 64);*/
            prtPktStats.queue_wrk_drp[socketId][fifoIndex] += 1;
            rte_pktmbuf_free(ptr[i]);
        }
        else 
          prtPktStats.queue_wrk_fls[socketId][fifoIndex] += 1;
      }
    }
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

    uint8_t lkpPort = 0, lkpFifo = 0;

    int32_t ret = 0, i = 0, j = 0;

    int8_t core_socketId = rte_lcore_to_socket_id(rte_lcore_id());
    int8_t port_socketId = rte_eth_dev_socket_id(rxport);
    port_socketId = (port_socketId == -1)?0:port_socketId;

    uint64_t key = 0, data = 0;

    struct rte_eth_link rxLink;

    struct rte_mempool *mbuff_pool = NULL;

    struct rte_mbuf *m = NULL, *ptr[8] = {NULL};

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
        rte_panic("\n ERROR: ports %u - %s \n", 
                   rxport, (rxLink.link_status == ETH_LINK_DOWN)?"Down":"Up");
        return -1;
    }

    fflush(stdout); /*flsuh all details before we start the thread*/

    while (1) {
      const int16_t nb_rx = rte_eth_rx_burst(rxport, 0, bufs, BURST_SIZE);
      if (unlikely(nb_rx == 0)) {
        continue;
      }

      //fprintf(stdout, "\n INFO: recv pkt count: %d", nb_rx);
      //rte_pktmbuf_dump (stdout, ptr[0], 64); /*dump the first packet content*/

      /* prefetch packets for pipeline */
      for (j = 0; ((j < PREFETCH_OFFSET) &&
                   (j < nb_rx)); j++)
      {
        rte_prefetch0(rte_pktmbuf_mtod(bufs[j], void *));
      } /*for loop till PREFETCH_OFFSET*/

      /* 
       * check packet ether type 
       *  - IPv4: fetch src and dst
       *  - IPv6: to do
       *  - others: drop
       */

      for (j = 0; j < (nb_rx - PREFETCH_OFFSET); j++) /* Prefetch others packets */
      {
        m = bufs[j];
        rte_prefetch0(rte_pktmbuf_mtod(bufs[j + PREFETCH_OFFSET], void *));

        ethHdr = rte_pktmbuf_mtod(m, struct ether_hdr*);
        //fprintf(stdout, "\n ether type : %x\n", ethHdr->ether_type);

/*
        fprintf(stdout, "\n dst MAC: %x:%x:%x:%x:%x:%x port %u ",
            ethHdr->d_addr.addr_bytes[0], ethHdr->d_addr.addr_bytes[1],
            ethHdr->d_addr.addr_bytes[2], ethHdr->d_addr.addr_bytes[3],
            ethHdr->d_addr.addr_bytes[4], ethHdr->d_addr.addr_bytes[5],
            m->port);
*/

        if (likely(ethHdr->ether_type == 0x0008)) {
          prtPktStats.rx_ipv4[rxport] += 1;
          ipHdr = (struct ipv4_hdr *) ((char *)(ethHdr + 1));
          //fprintf(stdout, "\n IP src:%x dst: %x", ipHdr->src_addr, ipHdr->dst_addr);

          int fifoCnt = fifoWrk;
          fifoIndex = rxport;
          do {
            /*check if the FIFO is free; check for other FIFO*/
            sprintf(srbName, "SW-FIFO-%d-%d", core_socketId, fifoIndex);
            srb = rte_ring_lookup(srbName);
            if (unlikely(srb == NULL))
              rte_panic("Failed to fetch launch worker for node %d fifo\n", core_socketId, fifoIndex);

            if (rte_ring_full(srb) == 0) {
              ret = rte_ring_mp_enqueue(srb, m);
              if (likely(ret == 0)) { 
                prtPktStats.queue_rx_add[port_socketId][fifoIndex] += 1;
                prtPktStats.rxPkts[core_socketId][fifoIndex] += 1;
                prtPktStats.rxBytes[core_socketId][fifoIndex] += m->pkt_len;
                break;
              }
            }

            fifoIndex = (fifoIndex + 1)%fifoWrk;
            fifoCnt -= 1;
          } while (fifoCnt);

          if (unlikely(fifoCnt == 0)) {
            rte_pktmbuf_free(m);
            prtPktStats.queue_rx_drp[port_socketId][fifoIndex] += 1;
          } 
          else
            continue;
        }
        else {
         rte_pktmbuf_free(m); /*Free the mbuff*/
         prtPktStats.non_ip[rxport] += 1; 
        }
      }

      for (; j < nb_rx; j++)
      {
        m = bufs[j];

        ethHdr = rte_pktmbuf_mtod(m, struct ether_hdr*);
        //fprintf(stdout, "\n ether type : %x\n", ethHdr->ether_type);

/*
        fprintf(stdout, "\n dst MAC: %x:%x:%x:%x:%x:%x port %u ",
            ethHdr->d_addr.addr_bytes[0], ethHdr->d_addr.addr_bytes[1],
            ethHdr->d_addr.addr_bytes[2], ethHdr->d_addr.addr_bytes[3],
            ethHdr->d_addr.addr_bytes[4], ethHdr->d_addr.addr_bytes[5],
            m->port);
*/

        if (likely(ethHdr->ether_type == 0x0008)) {
          prtPktStats.rx_ipv4[rxport] += 1;
          ipHdr = (struct ipv4_hdr *) ((char *)(ethHdr + 1));
          //fprintf(stdout, "\n IP src:%x dst: %x", ipHdr->src_addr, ipHdr->dst_addr);

          int fifoCnt = fifoWrk;
          fifoIndex = rxport;
          do {
            /*check if the FIFO is free; check for other FIFO*/
            sprintf(srbName, "SW-FIFO-%d-%d", core_socketId, fifoIndex);
            srb = rte_ring_lookup(srbName);
            if (unlikely(srb == NULL))
              rte_panic("Failed to fetch launch worker for node %d fifo\n", core_socketId, fifoIndex);

            if (rte_ring_full(srb) == 0) {
              ret = rte_ring_mp_enqueue(srb, m);
              if (likely(ret == 0)) { 
                prtPktStats.queue_rx_add[port_socketId][fifoIndex] += 1;
                prtPktStats.rxPkts[core_socketId][fifoIndex] += 1;
                prtPktStats.rxBytes[core_socketId][fifoIndex] += m->pkt_len;
                break;
              }
            }

            fifoIndex = (fifoIndex + 1)%fifoWrk;
            fifoCnt -= 1;
          } while (fifoCnt);

          if (unlikely(fifoCnt == 0)) {
            rte_pktmbuf_free(m);
            prtPktStats.queue_rx_drp[port_socketId][fifoIndex] += 1;
          }
          else
            continue;

        }
        else {
         rte_pktmbuf_free(m); /*Free the mbuff*/
         prtPktStats.non_ip[rxport] += 1; 
        }
      }
#if 0
      /* Send burst of TX packets, to second port of pair. */
      const uint16_t nb_tx = rte_eth_tx_burst((port + 1), 0, bufs, nb_rx);

      /* Free any unsent packets. */
      if (unlikely(nb_tx < nb_rx)) {
        uint16_t buf;
        for (buf = nb_tx; buf < nb_rx; buf++)
          rte_pktmbuf_free(bufs[buf]);
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


/* display usage */
static void print_usage(const char *prgname)
{
	printf ("%s [EAL options] --"\
		"  -f FIFO: number of worker cores <2 ^ n>",
		prgname);
}

/* Parse the argument given in the command line of the application */
static int parse_args(int argc, char **argv)
{
    int opt, ret = 0;
    char **argvopt;
    int option_index;
    char *prgname = argv[0];
    char *end = NULL;
    long pm;

    static struct option lgopts[] = {
                                     {CMD_LINE_OPT_FIFO, required_argument, 0, 'f'},
                                     {NULL, 0, 0, 0}
                                    };

    argvopt = argv;

    while ((opt = getopt_long(argc, argvopt, "f",
		              lgopts, &option_index)) != EOF) {
      switch (opt) {
        case 'f':
          pm = strtol(optarg, &end, 10);
          if ((optarg[0] == '\0') || (end == NULL) || (*end != '\0') || (pm < 1)) {
                fprintf(stdout, "\n ERR: FIFO DPDK workers count Invalid!!!\n");
                print_usage(prgname);
		return -1;
          }

          fifoWrk = (uint8_t)((pm & 0xfe) == 0)?fifoWrk:(pm & 0xfe);
          break;

        default:
	  print_usage(prgname);
	  return -1;
      }
    }

    return ret;
}

/*
 * The main function, which does initialization and calls the per-lcore functions.
 */
int main(int argc, char *argv[])
{
  signed nb_ports, nb_workers, workers, wlcore, i, index;
  uint8_t portid = 0, argPort = 0;
  uint16_t tempPortMask;
  uint64_t tempCoreMask;

  /*check if dpdk version is greater than or equal 16.07 to support SQM*/
  fprintf(stdout, "\n INFO: DPDK Ver: %s\n", rte_version());

  /* Initialize the Environment Abstraction Layer (EAL). */
  int ret = rte_eal_init(argc, argv);
  if (ret < 0)
  	rte_exit(EXIT_FAILURE, "with EAL initialization\n");
  
  argc -= ret;
  argv += ret;
 
  /* Register Signal */
  signal(SIGUSR1, sigExtraStats);
  signal(SIGUSR2, sigDetails);

  ret = parse_args(argc, argv);
  if (ret < 0)
    rte_exit(EXIT_FAILURE, "Invalid parameters\n");
  printf ("\n DEBUG: FIFO:%d", fifoWrk);

  /*node info build*/
  if (populateNodeInfo(&nodeCorePortInfo) < 0)
    rte_exit(EXIT_FAILURE, "NUMA info build \n");

  displayNodeInfo(&nodeCorePortInfo);

  /*iterate ports to identify the 40G interface*/
  if ((getCount40gPorts() == 0) && (getCount10gPorts() == 0))
    rte_exit(EXIT_FAILURE, "Error: Ports for 40G or 10G are not available\n");

  /*
   * Initalizae the 40G ports under DPDK
   */
  nb_ports = rte_eth_dev_count();
  for (i = 0; i < nb_ports; i++) 
  {
    if ((1 << i) & nodeCorePortInfo.port40G_map[0]) {
      argPort = i;
      if (port40G_init(argPort, 0) != 0)
        rte_exit(EXIT_FAILURE, "Cannot init port %"PRIu8 "on node 0\n", argPort);
    }
    else if ((1 << i) & nodeCorePortInfo.port40G_map[1]) {
      argPort = i;
      if (port40G_init(argPort, 0) != 0)
        rte_exit(EXIT_FAILURE, "Cannot init port %"PRIu8 "on node 1\n", argPort);
    }
	else if ((1 << i) & nodeCorePortInfo.port10G_map[0]) {
      argPort = i;
      if (port10G_init(argPort, 0) != 0)
        rte_exit(EXIT_FAILURE, "Cannot init port %"PRIu8 "on node 0\n", argPort);
    }
    else if ((1 << i) & nodeCorePortInfo.port10G_map[1]) {
      argPort = i;
      if (port10G_init(argPort, 0) != 0)
        rte_exit(EXIT_FAILURE, "Cannot init port %"PRIu8 "on node 1\n", argPort);
    }
  }
  fprintf(stdout, "\nINFO: Initialized 40G & 10 ports\n");

  for (index = 0; index < 2; index++)
  {
    if (nodeCorePortInfo.port40G_count[index] | nodeCorePortInfo.port10G_count[index]) {
      tempPortMask = nodeCorePortInfo.port40G_map[index] | nodeCorePortInfo.port10G_count[index];
      tempCoreMask = nodeCorePortInfo.core_map[index];

      if (nodeCorePortInfo.core_count[index] >= (nodeCorePortInfo.port10G_count[index] + nodeCorePortInfo.port40G_count[index] + fifoWrk + 1)) {
        /*create the sw ring bufferes to carry data to workers*/
        for (portid = 0; portid < fifoWrk; portid++) {
          fprintf(stdout, "node %d fifoworker %d", index, portid);
          argPort = portid;
          if (fifo_init(argPort, index) != 0)
            rte_exit(EXIT_FAILURE, "Cannot init SW RING %"PRIu8 " on node %d\n", argPort, index);
        }
 
 #if 0
        ipv4_hash_params.socket_id = index;
        ipv4_hash_params.name = (index == 0)?"ipv4-hash-0":
                                (index == 1)?"ipv4-hash-1":
                                (index == 2)?"ipv4-hash-2":
                                (index == 3)?"ipv4-hash-3":NULL;

        ipv4_hash_table[index] = rte_hash_create(&ipv4_hash_params);
        if (NULL == ipv4_hash_table[index])
          rte_exit(EXIT_FAILURE, " Failed to create hash table for ipv4 %s on node %d\n", rte_strerror(rte_errno), index);
#endif

        wlcore = -1;
        /*start the workers*/
        for (int32_t j = 0; j < fifoWrk; j++)
        {
          wlcore += 1;
          while ((tempCoreMask & (1 << wlcore)) == 0) {
            wlcore += 1;
          }
          fprintf(stdout, "\n INFO: core %d for worker %d", wlcore, j);
          ret = rte_eal_remote_launch(lcore_fifoTransmit, (void *) &j, wlcore);
          if (ret)
            rte_panic("Failed to launch worker\n");
          fprintf(stdout, "\n Initialized lcore%d with worker %d", wlcore, j);
          rte_delay_ms(2000);
        }
        fprintf(stdout, "\nINFO: started FIFO worker threads\n");

        /*start the ports*/
        argPort = 0;
        fprintf(stdout, "\nINFO: node %d ports %d\n", index, (nodeCorePortInfo.port40G_count[index] + nodeCorePortInfo.port10G_count[index]));
        for (int32_t j = 0; j < (nodeCorePortInfo.port40G_count[index] + nodeCorePortInfo.port10G_count[index]); j++)
        {
          while ((tempPortMask & (1<<argPort)) == 0) {
            argPort += 1;
          }
          wlcore += 1;
          while ((tempCoreMask & (1 << wlcore)) == 0) {
            wlcore += 1;
          }

          fprintf(stdout, "\n INFO: port %d core %d", argPort, wlcore);
          ret = rte_eal_remote_launch(lcore_pktFetch, (void *) &argPort, wlcore);
          if (ret != 0)
            rte_exit(EXIT_FAILURE, "\n unable to launch on core id %d!!", wlcore);
          fprintf(stdout, "\n Initialized lcore %d with port %d", wlcore, argPort);

          argPort += 1;
        }
        rte_delay_us(3500);
      }
      else
        rte_exit(EXIT_FAILURE, "ERROR: insufficent cores on node %d\n", index);
    }
  }

#if 1
  if (ret == 0) {
    STATS_CLR_SCREEN;

    set_stats_timer();
    show_static_display();
   
    while (1)
     display();
  }
#else
  while(1);
#endif

  return 0;
}

