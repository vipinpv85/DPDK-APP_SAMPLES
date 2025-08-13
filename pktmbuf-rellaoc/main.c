#include <stdint.h>
#include <stdlib.h>
#include <inttypes.h>
#include <rte_eal.h>
#include <rte_ethdev.h>
#include <rte_cycles.h>
#include <rte_lcore.h>
#include <rte_mbuf.h>
#include <signal.h>

#define RX_RING_SIZE 1024
#define TX_RING_SIZE 1024

#define NUM_MBUFS 8191
#define MBUF_CACHE_SIZE 250
#define BURST_SIZE 32

static bool reset_buffer=false;
static bool flush_mbuf=false;

static struct rte_mempool *mbuf_pool;

static void create_new_mbuf(void);

static inline int
port_init(uint16_t port, struct rte_mempool *mbuf_pool)
{
        struct rte_eth_conf port_conf;
        const uint16_t rx_rings = 1, tx_rings = 1;
        uint16_t nb_rxd = RX_RING_SIZE;
        uint16_t nb_txd = TX_RING_SIZE;
        int retval;
        uint16_t q;
        struct rte_eth_dev_info dev_info;
        struct rte_eth_txconf txconf;

        if (!rte_eth_dev_is_valid_port(port))
                return -1;

        memset(&port_conf, 0, sizeof(struct rte_eth_conf));

        retval = rte_eth_dev_info_get(port, &dev_info);
        if (retval != 0) {
                printf("Error during getting device (port %u) info: %s\n",
                                port, strerror(-retval));
                return retval;
        }


        if (dev_info.tx_offload_capa & RTE_ETH_TX_OFFLOAD_MBUF_FAST_FREE)
                port_conf.txmode.offloads |=
                        RTE_ETH_TX_OFFLOAD_MBUF_FAST_FREE;

        /* Configure the Ethernet device. */
        retval = rte_eth_dev_configure(port, rx_rings, tx_rings, &port_conf);
        if (retval != 0)
                return retval;

        retval = rte_eth_dev_adjust_nb_rx_tx_desc(port, &nb_rxd, &nb_txd);
        if (retval != 0)
                return retval;

        /* Allocate and set up 1 RX queue per Ethernet port. */
        for (q = 0; q < rx_rings; q++) {
                retval = rte_eth_rx_queue_setup(port, q, nb_rxd,
                                rte_eth_dev_socket_id(port), NULL, mbuf_pool);
                if (retval < 0)
                        return retval;
        }

        txconf = dev_info.default_txconf;
        txconf.offloads = port_conf.txmode.offloads;
        /* Allocate and set up 1 TX queue per Ethernet port. */
        for (q = 0; q < tx_rings; q++) {
                retval = rte_eth_tx_queue_setup(port, q, nb_txd,
                                rte_eth_dev_socket_id(port), &txconf);
                if (retval < 0)
                        return retval;
        }

        /* Starting Ethernet port. 8< */
        retval = rte_eth_dev_start(port);
        /* >8 End of starting of ethernet port. */
        if (retval < 0)
                return retval;

        /* Display the port MAC address. */
        struct rte_ether_addr addr;
        retval = rte_eth_macaddr_get(port, &addr);
        if (retval != 0)
                return retval;

        printf("Port %u MAC: %02" PRIx8 " %02" PRIx8 " %02" PRIx8
                           " %02" PRIx8 " %02" PRIx8 " %02" PRIx8 "\n",
                        port, RTE_ETHER_ADDR_BYTES(&addr));

        /* Enable RX in promiscuous mode for the Ethernet device. */
        retval = rte_eth_promiscuous_enable(port);
        /* End of setting RX port in promiscuous mode. */
        if (retval != 0)
                return retval;

        return 0;
}
static __rte_noreturn void
lcore_main(void)
{
        uint16_t port;
        uint64_t count = 0;

        /*
         * Check that the port is on the same NUMA node as the polling thread
         * for best performance.
         */
        RTE_ETH_FOREACH_DEV(port)
                if (rte_eth_dev_socket_id(port) >= 0 &&
                                rte_eth_dev_socket_id(port) !=
                                                (int)rte_socket_id())
                        printf("WARNING, port %u is on remote NUMA node to "
                                        "polling thread.\n\tPerformance will "
                                        "not be optimal.\n", port);

        printf("\nCore %u forwarding packets. [Ctrl+C to quit]\n",
                        rte_lcore_id());

        /* Main work of application loop. 8< */
        for (;;) {
                /*
                 * Receive packets on a port and forward them on the paired
                 * port. The mapping is 0 -> 1, 1 -> 0, 2 -> 3, 3 -> 2, etc.
                 */
                RTE_ETH_FOREACH_DEV(port) {

                        if (reset_buffer==false) {
                                /* Get burst of RX packets, from first port of pair. */
                                struct rte_mbuf *bufs[BURST_SIZE];
                                const uint16_t nb_rx = rte_eth_rx_burst(port, 0,
                                                bufs, BURST_SIZE);

                                if (unlikely(nb_rx == 0))
                                        continue;

                                /* Send burst of TX packets, to second port of pair. */
                                const uint16_t nb_tx = rte_eth_tx_burst(port, 0, bufs, nb_rx);

                                count += nb_tx;
                                if ((count % 10000000) == 0)
                                        printf("rx %u tx %u \n", nb_rx, nb_tx);
                                /* Free any unsent packets. */
                                if (unlikely(nb_tx < nb_rx)) {
                                        uint16_t buf;
                                        for (buf = nb_tx; buf < nb_rx; buf++)
                                                rte_pktmbuf_free(bufs[buf]);
                                }
                        }
                        else {
                                create_new_mbuf();
                                reset_buffer=false;
                        }
                }
        }
        /* >8 End of loop. */
}

static void create_new_mbuf(void)
{
        int ret = 0;
        {
                ret = rte_eth_dev_rx_queue_stop(0, 0);
                if (ret != 0)
                        rte_exit(EXIT_FAILURE, "unable to stop rx queue\n");

                ret = rte_eth_dev_tx_queue_stop(0, 0);
                if (ret != 0)
                        rte_exit(EXIT_FAILURE, "unable to stop tx queue\n");

                printf("stopped all queues!\n");

                rte_mempool_dump (stdout, mbuf_pool);
                rte_mempool_free (mbuf_pool);
                printf("dump pool after free\n");

                //mbuf_pool = rte_pktmbuf_pool_create("MBUF_POOL_new", NUM_MBUFS * 0.5,
              //mbuf_pool = rte_pktmbuf_pool_create("MBUF_POOL_new", NUM_MBUFS * 1,
                mbuf_pool = rte_pktmbuf_pool_create("MBUF_POOL_new", NUM_MBUFS * 2,
                        MBUF_CACHE_SIZE, 0, RTE_MBUF_DEFAULT_BUF_SIZE, rte_eth_dev_socket_id(0));
                if (mbuf_pool == NULL)
                        rte_exit(EXIT_FAILURE, "Cannot create new mbuf pool\n");
                printf("create new pool done!\n");
                rte_mempool_dump (stdout, mbuf_pool);

                if (rte_eth_dev_stop(0) != 0)
                        rte_exit(EXIT_FAILURE, "failed to stop port 0\n");

                ret = rte_eth_rx_queue_setup(0, 0, 1024, rte_eth_dev_socket_id(0), NULL, mbuf_pool);
                if (ret < 0)
                        rte_exit(EXIT_FAILURE, "Cannot rx queue setup %s:%d\n", rte_strerror(rte_errno), rte_errno);
                printf("rx queue setup!\n");

#if 0
                ret = rte_eth_dev_tx_queue_start(0, 0);
                if (ret != 0)
                        rte_exit(EXIT_FAILURE, "unable to stop tx queue\n");

                ret = rte_eth_dev_rx_queue_start(0, 0);
                if (ret != 0)
                        rte_exit(EXIT_FAILURE, "unable to stop rx queue\n");
#else
                if (rte_eth_dev_start(0) != 0)
                        rte_exit(EXIT_FAILURE, "failed to start port 0\n");
#endif

                printf("start all queues!\n");
        }
}

static void
signal_handler(int signum)
{
        int ret = 0;
        if (signum == SIGUSR1) {
                printf("\n\nSignal %d received, preparing to exit...\n",
                                signum);
                reset_buffer=true;
        }
}

int
main(int argc, char *argv[])
{
        unsigned nb_ports;
        uint16_t portid;

        signal(SIGUSR1, signal_handler);

        /* Initializion the Environment Abstraction Layer (EAL). 8< */
        int ret = rte_eal_init(argc, argv);
        if (ret < 0)
                rte_exit(EXIT_FAILURE, "Error with EAL initialization\n");
        /* >8 End of initialization the Environment Abstraction Layer (EAL). */

        argc -= ret;
        argv += ret;

        /* Check that there is an even number of ports to send/receive on. */
        nb_ports = rte_eth_dev_count_avail();
        if (nb_ports != 1)
                rte_exit(EXIT_FAILURE, "Error: number of ports must be 1\n");

        /* Creates a new mempool in memory to hold the mbufs. */

       /* Allocates mempool to hold the mbufs. 8< */
        mbuf_pool = rte_pktmbuf_pool_create("MBUF_POOL", NUM_MBUFS * nb_ports,
                MBUF_CACHE_SIZE, 0, RTE_MBUF_DEFAULT_BUF_SIZE, rte_socket_id());
        if (mbuf_pool == NULL)
                rte_exit(EXIT_FAILURE, "Cannot create mbuf pool\n");

        /* Initializing all ports. 8< */
        RTE_ETH_FOREACH_DEV(portid)
                if (port_init(portid, mbuf_pool) != 0)
                        rte_exit(EXIT_FAILURE, "Cannot init port %"PRIu16 "\n",
                                        portid);
        /* >8 End of initializing all ports. */

        if (rte_lcore_count() > 1)
                printf("\nWARNING: Too many lcores enabled. Only 1 used.\n");

        /* Call lcore_main on the main core only. Called on single lcore. 8< */
        lcore_main();
        /* >8 End of called on single lcore. */

        /* clean up the EAL */
        rte_eal_cleanup();

        return 0;
}
