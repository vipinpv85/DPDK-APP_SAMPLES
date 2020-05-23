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

#include "node.h"

numaInfo_t *nodeInfo = NULL;

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


uint8_t getCount1gPorts(numaInfo_t *nodeCorePortInfo)
{
  return (nodeCorePortInfo->port1G_count[0] + 
          nodeCorePortInfo->port1G_count[1] +
          nodeCorePortInfo->port1G_count[2] +
          nodeCorePortInfo->port1G_count[3]);
}

uint8_t getCount10gPorts(numaInfo_t *nodeCorePortInfo)
{
  return (nodeCorePortInfo->port10G_count[0] + 
          nodeCorePortInfo->port10G_count[1] +
          nodeCorePortInfo->port10G_count[2] +
          nodeCorePortInfo->port10G_count[3]);
}

uint8_t getCount40gPorts(numaInfo_t *nodeCorePortInfo)
{
  return (nodeCorePortInfo->port40G_count[0] + 
          nodeCorePortInfo->port40G_count[1] +
          nodeCorePortInfo->port40G_count[2] +
          nodeCorePortInfo->port40G_count[3]);
}

int port40G_init(uint8_t port, uint8_t numaNode)
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

int port10G_init(uint8_t port, uint8_t numaNode)
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

int port1G_init(uint8_t port, uint8_t numaNode)
{
    char mempoolName[64] = {0};
    struct rte_mempool *mbuff_pool = NULL;

    struct rte_eth_conf port_conf = port_conf_default;
    port_conf.link_speeds = ETH_LINK_SPEED_1G;

    int retval, i;

    struct rte_eth_dev_info devInfo;

    rte_eth_dev_stop(port);
    fprintf(stdout, "\n\n INFO: setting up port %d on node %d\n", port, numaNode);

    if (port >= rte_eth_dev_count())
      return -1;

    rte_eth_dev_info_get(port, &devInfo);

    /* allocate mempool TX and RX seperate */
    sprintf(mempoolName, "1G-%d-%d-%d-rx", devInfo.pci_dev->addr.function, port, numaNode);

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


void displayNodeInfo(void)
{
  if (nodeInfo == NULL)
	return;

  if (nodeInfo->port1G_count[0] | 
      nodeInfo->port10G_count[0] | 
      nodeInfo->port40G_count[0]) {
    fprintf(stdout,"\n NODE: 0 -- PORT --");
    fprintf(stdout,"\n ^^  1G ports: %x count: %u",nodeInfo->port1G_map[0], nodeInfo->port1G_count[0]);
    fprintf(stdout,"\n ^^ 10G ports: %x count: %u",nodeInfo->port10G_map[0],nodeInfo->port10G_count[0]);
    fprintf(stdout,"\n ^^ 40G ports: %x count: %u",nodeInfo->port40G_map[0],nodeInfo->port40G_count[0]);
  }
  if (nodeInfo->core_count[0]) {
    fprintf(stdout,"\n NODE: 0 -- CORE --");
    fprintf(stdout,"\n **   map: %"PRIx64,nodeInfo->core_map[0]);
    fprintf(stdout,"\n ** count: %u",nodeInfo->core_count[0]);
  }

  if (nodeInfo->port1G_count[1] | 
      nodeInfo->port10G_count[1] | 
      nodeInfo->port40G_count[1]) {
    fprintf(stdout,"\n NODE: 1 -- PORT --");
    fprintf(stdout,"\n ^^  1G ports: %x count: %u",nodeInfo->port1G_map[1], nodeInfo->port1G_count[1]);
    fprintf(stdout,"\n ^^ 10G ports: %x count: %u",nodeInfo->port10G_map[1],nodeInfo->port10G_count[1]);
    fprintf(stdout,"\n ^^ 40G ports: %x count: %u",nodeInfo->port40G_map[1],nodeInfo->port40G_count[1]);
  }
  if (nodeInfo->core_count[1]) {
    fprintf(stdout,"\n NODE: 1 -- CORE --");
    fprintf(stdout,"\n **   map: %"PRIx64,nodeInfo->core_map[1]);
    fprintf(stdout,"\n ** count: %u",nodeInfo->core_count[1]);
  }

  if (nodeInfo->port1G_count[2] | 
      nodeInfo->port10G_count[2] | 
      nodeInfo->port40G_count[2]) {
    fprintf(stdout,"\n NODE: 2 -- PORT --");
    fprintf(stdout,"\n ^^  1G ports: %x count: %u",nodeInfo->port1G_map[2], nodeInfo->port1G_count[2]);
    fprintf(stdout,"\n ^^ 10G ports: %x count: %u",nodeInfo->port10G_map[2],nodeInfo->port10G_count[2]);
    fprintf(stdout,"\n ^^ 40G ports: %x count: %u",nodeInfo->port40G_map[2],nodeInfo->port40G_count[2]);
  }
  if (nodeInfo->core_count[2]) {
    fprintf(stdout,"\n NODE: 2 -- CORE --");
    fprintf(stdout,"\n **   map: %"PRIx64,nodeInfo->core_map[2]);
    fprintf(stdout,"\n ** count: %u",nodeInfo->core_count[2]);
  }

  if (nodeInfo->port1G_count[3] | 
      nodeInfo->port10G_count[3] | 
      nodeInfo->port40G_count[3]) {
    fprintf(stdout,"\n DEBUG: NODE: 3 -- PORT --");
    fprintf(stdout,"\n ^^  1G ports: %x count: %u",nodeInfo->port1G_map[3], nodeInfo->port1G_count[3]);
    fprintf(stdout,"\n ^^ 10G ports: %x count: %u",nodeInfo->port10G_map[3],nodeInfo->port10G_count[3]);
    fprintf(stdout,"\n ^^ 40G ports: %x count: %u",nodeInfo->port40G_map[3],nodeInfo->port40G_count[3]);
  }
  if (nodeInfo->core_count[3]) {
    fprintf(stdout,"\n NODE: 3 -- CORE --");
    fprintf(stdout,"\n **   map: %"PRIx64,nodeInfo->core_map[3]);
    fprintf(stdout,"\n ** count: %u",nodeInfo->core_count[3]);
  }

  return;
}

int32_t populateNodeInfo(numaInfo_t *nodeCorePortInfo)
{
  int32_t i = 0, socketId = -1, lcoreIndex = 0, enable = 0;
  uint8_t portCount;

  nodeInfo = nodeCorePortInfo;

  struct rte_eth_dev_info devInfo;
  struct rte_eth_link link;

  /* fetch total lcore count under DPDK */
  i = rte_get_next_lcore(-1, 1, 0);
  while (i != RTE_MAX_LCORE) 
  {
     socketId = rte_lcore_to_socket_id(i);
     enable = rte_lcore_is_enabled(i);

     //fprintf(stdout,"\n lcore %d socket %d enable %d", i, socketId, enable);

     if (likely(enable)) {
       /* classify the lcore info per NUMA node */
       nodeCorePortInfo->core_map[socketId] |= (1 << i);
       nodeCorePortInfo->core_count[socketId] += 1;
     }
     else {
       fprintf(stdout,"ERROR: Lcore %d Socket %d not enabled\n", lcoreIndex, socketId);
     }

     i = rte_get_next_lcore(i, 1, 0);
  }

  portCount = rte_eth_dev_count();
  for (i =0; i < portCount; i++)
  {
    rte_eth_dev_info_get(i, &devInfo);
    rte_eth_link_get(i, &link);

    socketId = (devInfo.pci_dev->device.numa_node == -1)?0:devInfo.pci_dev->device.numa_node;

    /*if link status is down check speed option for 40G*/
    if (link.link_speed == ETH_SPEED_NUM_40G) {
      nodeCorePortInfo->port40G_map[socketId]   |= (1 << i);
      nodeCorePortInfo->port40G_count[socketId] += 1;
    }
    else if (link.link_speed == ETH_SPEED_NUM_10G) {
      nodeCorePortInfo->port10G_map[socketId]   |= (1 << i);
      nodeCorePortInfo->port10G_count[socketId] += 1;
    }
    else if (link.link_speed == ETH_SPEED_NUM_1G) {
      nodeCorePortInfo->port1G_map[socketId]   |= (1 << i);
      nodeCorePortInfo->port1G_count[socketId] += 1;
    }
    else if (link.link_speed == ETH_SPEED_NUM_NONE) {
#if 0
      fprintf(stdout,"\n INFO: Skipping intf %d speed %d duples %d autoneg %d status %d", i, link.link_speed, link.link_duplex, link.link_autoneg, link.link_status);
      /*If speed is not set check PCI vendor and subvendor values to assume speed*/
#endif
      fprintf(stdout,"\n DEBUG: Skipping == PCI ID - vendor:device:sub-vendor:sub-device %04x:%04x:%04x:%04x",
              devInfo.pci_dev->id.vendor_id,
              devInfo.pci_dev->id.device_id,
              devInfo.pci_dev->id.subsystem_vendor_id,
              devInfo.pci_dev->id.subsystem_device_id);
      fprintf(stdout,"\n Interface %d, 40G:%d 10G:%d 1G:%d", i, 
                 ((devInfo.speed_capa & ETH_LINK_SPEED_40G) != 0),
                 ((devInfo.speed_capa & ETH_LINK_SPEED_10G) != 0),
                 ((devInfo.speed_capa & ETH_LINK_SPEED_1G)  != 0));
    }
    else { 
      fprintf(stdout,"\n DEBUG: Unknwn - Intf %d Speed %d Socket %d", i, link.link_speed, socketId);
      fprintf(stdout,"\n - driver: %s", devInfo.driver_name);
      fprintf(stdout,"\n - if_index: %d", devInfo.if_index);
      if (devInfo.pci_dev) {
        fprintf(stdout,"\n - PCI INFO ");
        fprintf(stdout,"\n -- ADDR - domain:bus:devid:function %04x:%04x:%02x.%x",
              devInfo.pci_dev->addr.domain,
              devInfo.pci_dev->addr.bus,
              devInfo.pci_dev->addr.devid,
              devInfo.pci_dev->addr.function);
        fprintf(stdout,"\n == PCI ID - vendor:device:sub-vendor:sub-device %04x:%04x:%04x:%04x",
              devInfo.pci_dev->id.vendor_id,
              devInfo.pci_dev->id.device_id,
              devInfo.pci_dev->id.subsystem_vendor_id,
              devInfo.pci_dev->id.subsystem_device_id);
        fprintf(stdout,"\n -- numa node: %d",devInfo.pci_dev->device.numa_node);
      }
    }
  }
  return 0;
}
