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

    socketId = (devInfo.pci_dev->numa_node == -1)?0:devInfo.pci_dev->numa_node;

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
        fprintf(stdout,"\n -- numa node: %d", devInfo.pci_dev->numa_node);
      }
    }
  }
  return 0;
}
