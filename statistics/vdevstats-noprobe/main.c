/*
 *   BSD LICENSE
 *
 *   Copyright(c) 2010-2016 Intel Corporation. All rights reserved.
 *   All rights reserved.
 *
 *   Redistribution and use in source and binary forms, with or without
 *   modification, are permitted provided that the following conditions
 *   are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in
 *       the documentation and/or other materials provided with the
 *       distribution.
 *     * Neither the name of Intel Corporation nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *   OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <stdarg.h>
#include <inttypes.h>
#include <sys/queue.h>
#include <stdlib.h>
#include <getopt.h>

#include <net/if.h>
#include <pcap.h>
#include <sys/stat.h>

#include <rte_eal.h>
#include <rte_common.h>
#include <rte_debug.h>
#include <rte_ethdev.h>
#include <rte_malloc.h>
#include <rte_memory.h>
#include <rte_memzone.h>
#include <rte_launch.h>
#include <rte_tailq.h>
#include <rte_per_lcore.h>
#include <rte_lcore.h>
#include <rte_debug.h>
#include <rte_log.h>
#include <rte_atomic.h>
#include <rte_branch_prediction.h>
#include <rte_string_fns.h>
#include <rte_cycles.h>

struct virtnet_rx {
        struct virtqueue *vq;
        /* dummy mbuf, for wraparound when processing RX ring. */
        struct rte_mbuf fake_mbuf;
        uint64_t mbuf_initializer; /**< value to init mbufs. */
        struct rte_mempool *mpool; /**< mempool for mbuf allocation */

        uint16_t queue_id;   /**< DPDK queue index. */
        uint8_t port_id;     /**< Device port identifier. */

        /* Statistics */
        //struct virtnet_stats stats;
        uint64_t stats[5];

        const struct rte_memzone *mz; /**< mem zone to populate RX ring. */
};

struct virtnet_tx {
        struct virtqueue *vq;
        /**< memzone to populate hdr. */
        const struct rte_memzone *virtio_net_hdr_mz;
        phys_addr_t virtio_net_hdr_mem;  /**< hdr for each xmit packet */

        uint16_t    queue_id;            /**< DPDK queue index. */
        uint8_t     port_id;             /**< Device port identifier. */

        /* Statistics */
        //struct virtnet_stats stats;
        uint64_t stats[5];

        const struct rte_memzone *mz;    /**< mem zone to populate TX ring. */
};


static void nic_stats_display(pid_t pid)
{
	static const char *MZ_RTE_ETH_DEV_DATA = "rte_eth_dev_data";
	static const char *pcapCmp = "rte_eth_dev_data";
	const struct rte_memzone *region = NULL;
	const struct rte_eth_dev_data *ports = NULL;

	uint8_t i = 0, displayAll = 0;
	uint16_t j = 0;

	struct stat sts;

	char *dev, errbuf[PCAP_ERRBUF_SIZE];
	pcap_if_t *interfaces,*temp;
	int ret;
	char cmd[50], *name= "mydevice_";

	char array[1000] = {0};
	sprintf(cmd, "kill -0 %d", pid);
	FILE *f = popen(cmd, "r");
	fgets(array, sizeof(array), f);
	pclose(f);

	if (strlen(array) == 0){
		sprintf(cmd, "strings  /proc/%d/cmdline | grep eth_pcap", pid);
		FILE *f = popen(cmd, "r");
		fgets(array, sizeof(array), f);
		fclose(f);
		
		if (strstr("No such file", array) != NULL) {
			printf("\n array: %s", array);
			displayAll = 1;
		}
	}

	static const char *nic_stats_border = "########################";

	region = rte_memzone_lookup(MZ_RTE_ETH_DEV_DATA);
	if (region == NULL) {
		printf("\n failed to lookup MZ_RTE_ETH_DEV_DATA: %s", MZ_RTE_ETH_DEV_DATA);
		return;
	}
	ports = region->addr;
	
	while (i < 16) {
		if (strlen(ports[i].name)) {
			printf("\n %s NIC statistics for port %-2d %s", nic_stats_border, i, nic_stats_border);
			printf("\n - name: %s", ports[i].name);
			printf("\n - DPDK Port id: %d", ports[i].port_id);

			if (ports[i].dev_started == 0) {
				if (displayAll == 0) {
					/*check if eth_pcap*/
					if (strstr(ports[i].name, "eth_pcap") == NULL) {
						printf("\n Interface %s not expected; dumping all pcap!!", ports[i].name);
						displayAll = 1;
					}
					else {
						sprintf(cmd, "strings  /proc/%d/cmdline | grep eth_pcap%d", pid, ports[i].port_id);
						f = popen(cmd, "r");
						fgets(array, sizeof(array), f);
						pclose(f);

						char *device = strstr(array, name);
						if (device) {
							fflush(stdout);
							sprintf(cmd, "ip -s link show %s", device);
							ret = system(cmd);
						}
					}
				}

				i++;
				continue;
			}

			printf("\n - numa node: %d mtu: %d dev_started: %d promiscuous: %d", 
				ports[i].numa_node, ports[i].mtu, ports[i].dev_started, ports[i].promiscuous);
			printf("\n - dev_link: speed: %d duplex: %d autoneg: %d status: %d", 
					ports[i].dev_link.link_speed, ports[i].dev_link.link_duplex,
					ports[i].dev_link.link_autoneg, ports[i].dev_link.link_status);
			//printf("\n - drv_name: %s", ports[i].drv_name);
			printf("\n - kdrv: %d", ports[i].kdrv);
			if (ports[i].mac_addrs) 
				printf("\n - mac_addrs: %02x:%02x:%02x:%02x:%02x:%02x ", 
					ports[i].mac_addrs->addr_bytes[0],
					ports[i].mac_addrs->addr_bytes[1],
					ports[i].mac_addrs->addr_bytes[2],
					ports[i].mac_addrs->addr_bytes[3],
					ports[i].mac_addrs->addr_bytes[4],
					ports[i].mac_addrs->addr_bytes[5]);
			printf("\n - min_rx_buf_size: %d", ports[i].min_rx_buf_size);
			printf("\n - all_multicast: %d dev_flags: %d", ports[i].all_multicast, ports[i].dev_flags);
			printf("\n - nb_rx_queues: %d nb_tx_queues: %d", ports[i].nb_rx_queues, ports[i].nb_tx_queues);

			if (strncmp(pcapCmp, ports[i].name,7) != 0) 
			{
				if (ports[i].rx_queues) {
					printf("\n - rx nombuf: %"PRIu64, ports[i].rx_mbuf_alloc_failed);
					if (ports[i].rx_queues == NULL) 
						continue;
					for (j = 0; j < ports[i].nb_rx_queues; j++) {
						printf("\n -- port: %d --", ((struct virtnet_rx *)ports[i].rx_queues[j])->port_id);
						if (ports[i].rx_queues[j] == NULL) 
							continue;
						uint64_t *stats = (uint64_t *)&((struct virtnet_rx *)ports[i].rx_queues[j])->stats;
						printf("\n - RX%d pkts: %"PRIu64" bytes: %"PRIu64" err: %"PRIu64" broad: %"PRIu64" multi: %"PRIu64, 
							j, 
							stats[0], stats[1], stats[2], stats[3], stats[4]); 
					}
				}

				if (ports[i].tx_queues) {
					if (ports[i].tx_queues == NULL) 
						continue;
					for (j = 0; j < ports[i].nb_tx_queues; j++) {
						printf("\n -- port: %d --", ((struct virtnet_rx *)ports[i].tx_queues[j])->port_id);
						if (ports[i].tx_queues[j] == NULL) 
							continue;
						uint64_t *stats = (uint64_t *)&((struct virtnet_tx *)ports[i].tx_queues[j])->stats;
						printf("\n - TX%d  pkts: %"PRIu64" bytes: %"PRIu64" err: %"PRIu64" broad: %"PRIu64" multi: %"PRIu64, 
							j, 
							stats[0], stats[1], stats[2], stats[3], stats[4]); 
					}
				}
			}
			else {
				if ((ports[i].kdrv != RTE_KDRV_NONE) & (ports[i].dev_flags != RTE_ETH_DEV_DETACHABLE))
				{
					printf("\n unknown port: %s", ports[i].name);
					continue;
				}

			}
		}
		i++;
	}
	printf("\n %s %s \n", nic_stats_border, nic_stats_border);

	if (displayAll) {
		printf("\n %s PCAP ALL %s \n", nic_stats_border, nic_stats_border);

		//First get the list of available devices
		if(pcap_findalldevs(&interfaces,errbuf)==-1)
		{
			printf("\nerror in pcap findall devs");
			return;
		}

		for(temp=interfaces;temp;temp=temp->next)
		{
			if (strncmp(temp->name, name, 11) == 0) {
				printf("\n -- %s dedsc: %s \n", temp->name, temp->description);
				sprintf(cmd, "ip -s link show %s", temp->name);
				ret = system(cmd);
			}
		}

		printf("\n %s %s \n", nic_stats_border, nic_stats_border);
	}	
}


int
main(int argc, char **argv)
{
	int ret, i;
	char c_flag[] = "-c1";
	char n_flag[] = "-n4";
	char mp_flag[] = "--proc-type=secondary";
	char nopci_flag[] = "--no-pci";
	char nohpet_flag[] = "--no-hpet"; 
	char *argp[argc + 5];

	pid_t pid = 0;

	argp[0] = argv[0];
	argp[1] = c_flag;
	argp[2] = n_flag;
	argp[3] = mp_flag;
	argp[4] = nopci_flag;
	argp[5] = nohpet_flag;

	argc = (argc == 2)?2:0;

	for (i = 1; i < argc; i++)
		argp[i + 5] = argv[i];

	argc += 5;

	ret = rte_eal_init(argc, argp);
	if (ret < 0)
		rte_panic("Cannot init EAL\n");

	if (!rte_eal_primary_proc_alive(NULL))
		rte_exit(EXIT_FAILURE, "No primary DPDK process is running.\n");

	if (argc) {
		//printf("\n user argument: %s", argp[6]);
		pid = atoi(argp[6]);
	}

	nic_stats_display(pid);

	return 0;
}
