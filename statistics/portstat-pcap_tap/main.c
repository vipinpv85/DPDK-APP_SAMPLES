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

#include <rte_config.h>

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
#include <rte_pci.h>
#include <rte_dev.h>
#include <rte_devargs.h>

#define PCAP_TAP_REGEX "test"
/* static */
//#define PROCSTAT_ARGS1 "./dpdk-procinfo -c 1 --no-hpet "
/*shared*/
#define PROCSTAT_ARGS1 "./dpdk-procinfo -c 1 --no-hpet -d <PATH to DPDK lib>/lib/<add libraries for physical nic>"
#define PROCSTAT_ARGS2 " -- --xstats"

int main(int argc, char **argv)
{
	int ret, i;
    uint16_t portCount = 0;

    char errbuf[PCAP_ERRBUF_SIZE] = {0};
    pcap_if_t *interfaces = NULL, *temp = NULL;

    char array[200] = {0}, cmd[1000] = {0};

    struct rte_eth_dev_info devInfo = {0};


    ret = rte_eal_init(argc, argv);
    if (ret < 0)
        rte_panic("Cannot init EAL\n"); 

	if (!rte_eal_primary_proc_alive(NULL))
		rte_exit(EXIT_FAILURE, "No primary DPDK process is running.\n");

    portCount = rte_eth_dev_count ();
    fprintf (stdout, " Interfaces in Primary: %u\n", portCount);

    for (i = 0; i < portCount; i++) {
        rte_eth_dev_info_get (i, &devInfo);

        fprintf(stdout, " - driver %s\n", 
                        devInfo.driver_name);
        fprintf(stdout, " - if_index %d\n", 
                        devInfo.if_index);
#if 0
        fprintf(stdout, " - nb_queues: rx %u tx %u\n", 
                        devInfo.nb_rx_queues, devInfo.nb_tx_queues);
        fprintf(stdout, " - CAPA: speed 0x%x rx_offload 0x%x tx_offlad 0x%x\n", 
                        devInfo.speed_capa, devInfo.rx_offload_capa, devInfo.tx_offload_capa);
        fprintf(stdout, " - RX: min_bufsize %u max_pktlen %u\n", 
                        devInfo.min_rx_bufsize, devInfo.max_rx_pktlen);
        fprintf(stdout, " - max_queues RX %u; TX %u\n", 
                        devInfo.max_rx_queues, devInfo.max_tx_queues);
        fprintf(stdout, " - MAX: mac_addrs %u hash_mac_addrs %u vfs %u\n", 
                        devInfo.max_mac_addrs, devInfo.max_hash_mac_addrs, devInfo.max_vfs);
        fprintf(stdout, " - vmdq: max_pools %u queue_base %u queue_num %u pool_base %u\n", 
                        devInfo.max_vmdq_pools, devInfo.vmdq_queue_base, devInfo.vmdq_queue_num, devInfo.vmdq_pool_base);
        fprintf(stdout, " - device redirection table size %u\n", 
                        devInfo.reta_size);
        fprintf(stdout, " - hash key size %u\n", 
                        devInfo.hash_key_size);
        fprintf(stdout, " - Bit mask of RSS offloads 0x%x\n",
                        devInfo.flow_type_rss_offloads);
#endif

        if (devInfo.pci_dev) {
#if 0
            fprintf(stdout," - PCI INFO \n");
            fprintf(stdout," -- ADDR - domain:bus:devid:function %04x:%04x:%04x.%x\n",
                            devInfo.pci_dev->addr.domain,
                            devInfo.pci_dev->addr.bus,
                            devInfo.pci_dev->addr.devid,
                            devInfo.pci_dev->addr.function);
            fprintf(stdout," == PCI ID - vendor:device:sub-vendor:sub-device %04x:%04x:%04x:%04x\n",
                            devInfo.pci_dev->id.vendor_id,
                            devInfo.pci_dev->id.device_id,
                            devInfo.pci_dev->id.subsystem_vendor_id,
                            devInfo.pci_dev->id.subsystem_device_id);
            fprintf(stdout, " max_vfs %d\n", devInfo.pci_dev->max_vfs);
            fprintf(stdout, " Pass through Kenrel dirver %d\n", devInfo.pci_dev->kdrv);
            fprintf(stdout, " PCI location %s\n", devInfo.pci_dev->name);

            fprintf(stdout, " device: name %s numa %d \n",
                            devInfo.pci_dev->device.name, devInfo.pci_dev->device.numa_node); 
            if (devInfo.pci_dev->device.devargs)
                fprintf(stdout, " devargs - type %d name %s args %s",
                                devInfo.pci_dev->device.devargs->type, devInfo.pci_dev->device.devargs->name, 
                                devInfo.pci_dev->device.devargs->args);
            fprintf(stdout, " device:driver -  name %s alias %s\n", 
                            devInfo.pci_dev->device.driver->name, devInfo.pci_dev->device.driver->alias);
            fprintf(stdout, " driver: drv_flags 0x%x\n", 
                            devInfo.pci_dev->driver->drv_flags);
#else
            sprintf(array, "%s -w %04x:%02x:%02x.%x", 
                            array,
                            devInfo.pci_dev->addr.domain,
                            devInfo.pci_dev->addr.bus,
                            devInfo.pci_dev->addr.devid,
                            devInfo.pci_dev->addr.function);
#endif
        }
    }

    fprintf(stdout, " Primary array-%s\n", array);

    /*invoke the proc stat DPDK application */
    sprintf(cmd, "%s %s %s | grep -v ': 0'", PROCSTAT_ARGS1, array, PROCSTAT_ARGS2);
    fprintf(stdout, " CMD: %s\n", cmd);
    ret = system(cmd);

    /*get the list of available devices*/
    if(pcap_findalldevs(&interfaces,errbuf)==-1)
    {
        printf("error in pcap findall devs \n");
        return -1;
    }

    for(temp=interfaces;temp;temp=temp->next)
    {
        if (strncmp(temp->name, PCAP_TAP_REGEX, 11) == 0) {
            printf("========= BEGIN %s =========\n", temp->name);
            printf("\n -- %s dedsc: %s \n", temp->name, temp->description);
            sprintf(cmd, "ip -s link show %s", temp->name);
            ret = system(cmd);
            printf("========= END %s =========\n", temp->name);
        }
    }
    
	return 0;
}
