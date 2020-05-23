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

#include <stdint.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>

#include <rte_config.h>
#include <rte_branch_prediction.h>
#include <rte_debug.h>
#include <rte_eal.h>
#include <rte_common.h>
#include <rte_cfgfile.h>
#include <rte_mempool.h>
#include <rte_malloc.h>
#include <rte_mbuf.h>
#include <rte_memcpy.h>
#include <rte_lcore.h>
#include <rte_ethdev.h>
#include <rte_errno.h>
#include <rte_ip.h>
#include <rte_udp.h>
#include <rte_ethdev.h>
#include <rte_version.h>
#include <rte_hash.h>
#include <rte_arp.h>
#include <rte_hash_crc.h>


int main(int argc, char *argv[])
{
  uint8_t state = 0xff;
  struct rte_eth_dev_info devInfo;
  struct rte_eth_link link;

  /*check if dpdk version is greater than or equal 16.07 to support SQM*/
  fprintf(stdout, "\n INFO: DPDK Ver: %s\n", rte_version());

  /* Initialize the Environment Abstraction Layer (EAL). */
  int ret = rte_eal_init(argc, argv);
  if (ret < 0)
	rte_exit(EXIT_FAILURE, "with EAL initialization\n");
  
  argc -= ret;
  argv += ret;
  
  if (rte_eth_dev_count() == 0)
	rte_exit(EXIT_FAILURE, "there are no interfaces\n");
	
  if (rte_eal_process_type() == RTE_PROC_PRIMARY) {
	rte_eth_dev_info_get(0, &devInfo);
	rte_eth_link_get    (0, &link);
 
	fprintf(stdout,"\n DEBUG: Intf 0 Speed %d link %d", link.link_speed, link.link_status);
	fprintf(stdout,"\n - driver: %s", devInfo.driver_name);
	fprintf(stdout,"\n - if_index: %d", devInfo.if_index);
	if (devInfo.pci_dev) {
	  fprintf(stdout,"\n - PCI INFO ");
	  fprintf(stdout,"\n -- ADDR - domain:bus:devid:function %04x:%04x:%04x.%x",
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
  
	fflush(stdout);
  }
  
  state = link.link_status;
  
  while(1) {
	//rte_eth_link_get        (0, &link);
	rte_eth_link_get_nowait (0, &link);
	
	if (state != link.link_status) {
	  state = link.link_status;
	  fprintf(stdout,"\n DEBUG: Intf 0 Speed %d link %d process %s", link.link_speed, link.link_status, (rte_eal_process_type() == RTE_PROC_PRIMARY)?"primary":"secondary");
	  fflush(stdout);
	}

  }
 
  return 0;
}