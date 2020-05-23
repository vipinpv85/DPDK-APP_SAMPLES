#include "config.h"
#include "node.h"
#include "stats.h"

/* DEFINES */
#define MAX_TX_BURST_COUNT 8

/* GLOBALS */

/* EXTERN */
extern uint8_t gtpConfigCount;
extern port_gtpConfig_t gtpConfig [GTP_PKTGEN_MAXPORTS];
extern const char gtpC[GTPC_MAXCOUNT][1500];
extern const char gtpU[GTPU_MAXCOUNT][1500];
extern numa_Info_t numaNodeInfo[GTP_MAX_NUMANODE];


static int pktgenHandler(void *arg)
{
    uint8_t port = *((uint8_t *)arg);
    struct rte_mempool *mbuf_pool_tx = NULL;
    //struct rte_mempool *mbuf_pool_rx = NULL;
    struct rte_mbuf *ptr[8], *m = NULL;
    unsigned lcore_id, socket_id,i,ret;
    struct ether_hdr * eth_hdr = NULL;

    //printf("\n\n\n\n arg %d port %d \n\n\n", *(uint8_t *)arg, port);

    lcore_id = rte_lcore_id();
    socket_id = rte_lcore_to_socket_id(lcore_id);

    /* ToDo: if mempool is per port ignore the below*/
    mbuf_pool_tx = numaNodeInfo[socket_id].tx[0];
    //mbuf_pool_tx = numaNodeInfo[socket_id].tx[port];

    while(1)
    {
        for(i = 0; i < MAX_TX_BURST_COUNT; i++ )
        {
            if (unlikely((ptr[i] = rte_pktmbuf_alloc(mbuf_pool_tx)) == NULL)) {
                printf("\n -------------- ");
                continue;
            }

            ptr[i]->data_len = ptr[i]->pkt_len = 264;
            eth_hdr = rte_pktmbuf_mtod(ptr[i], struct ether_hdr *);
            if(gtpConfig[socket_id].gtpType == GTPU)
                rte_memcpy(((char *)eth_hdr),gtpU[gtpConfig[socket_id].pktIndex],ptr[i]->pkt_len);
            else if(gtpConfig[socket_id].gtpType == GTPC)
                rte_memcpy(((char *)eth_hdr),gtpC[gtpConfig[socket_id].pktIndex],ptr[i]->pkt_len);
            else {
                printf(" Unknown Packet type: %d\n", gtpConfig[socket_id].gtpType);
                printf("\n Prt Index %d, GTP - Type %d, Ver - %d, Pkt Index %d", 
                        gtpConfig[socket_id].prtIndex,
                        gtpConfig[socket_id].gtpType,
                        gtpConfig[socket_id].gtpVersion,
                        gtpConfig[socket_id].pktIndex);
                exit(0);
            }
        }

        //rte_pktmbuf_dump (stdout, ptr[0], 64);

        ret = rte_eth_tx_burst(gtpConfig[port].prtIndex, 0, (struct rte_mbuf **)&ptr, 8);
        //ret = rte_eth_tx_burst(gtpConfig[socket_id].prtIndex, 0, (struct rte_mbuf **)&ptr, 8);
        if (unlikely(MAX_TX_BURST_COUNT > ret )) 
        {
            for (i = ret; i < MAX_TX_BURST_COUNT; i++)
            {
                m = ptr[i];
                rte_pktmbuf_free(m);
            }
        }
    }
}


int main(int argc, char **argv)
{
    int32_t ret = 0;

    argc -= ret;
    argv += ret;

    /* Load INI configuration for fetching GTP port details */
    ret = loadGtpConfig();
    if (unlikely(ret < 0))
    {
        printf("\n ERROR: failed to load config\n");
        return -1;
    }

    /* Initialize DPDK EAL */
    ret =  rte_eal_init(argc, argv);
    if (unlikely(ret < 0))
    {
        printf("\n ERROR: Cannot init EAL\n");
        return -2;
    }

    /* check Huge pages for memory buffers */
    ret = rte_eal_has_hugepages();
    if (unlikely(ret < 0))
    {
        rte_panic("\n ERROR: No Huge Page\n");
        exit(EXIT_FAILURE);
    }

    ret = populateNodeInfo();
    if (unlikely(ret < 0))
    {
        rte_panic("\n ERROR: in populating NUMA node Info\n");
        exit(EXIT_FAILURE);
    }

    /* launch functions for specified cores */
    if (interfaceSetup() < 0)
    {
        rte_panic("ERROR: interface setup Failed\n");
        exit(EXIT_FAILURE);

    }

    /*Launch thread in core 1*/
    ret = 0;
    rte_eal_remote_launch(pktgenHandler, (void *)&ret, 1);

    /*Launch thread in core 2*/
    ret = 1;
    rte_eal_remote_launch(pktgenHandler, (void *)&ret, 2);

    /*Launch thread in core 3*/
    //rte_eal_remote_launch(pktgenHandler,2, 3);
    /*Launch thread in core 0*/
    //rte_eal_remote_launch(pktgenHandler,3, 0);

    set_stats_timer();
    rte_delay_ms(5000);
    show_static_display();

    do {
        rte_delay_ms(1000);
        rte_timer_manage();
    }while(1);
}

