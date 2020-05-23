#include <stdint.h>
#include <inttypes.h>
#include <signal.h>
#include <rte_eal.h>
#include <rte_ethdev.h>
#include <rte_cycles.h>
#include <rte_lcore.h>
#include <rte_mbuf.h>
#include <rte_version.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/if.h>
#include <linux/if_tun.h>
#include <getopt.h>

#define RX_RING_SIZE 1024
#define TX_RING_SIZE 1024

#define NUM_MBUFS 8191
#define MBUF_CACHE_SIZE 250
#define BURST_SIZE 32

uint8_t dpdk_old_version = 0;
uint8_t port_map_tap = 0;
uint16_t tap_count = 0;
uint16_t tap_ports[32];
int tap_map_port[32][4]; 
struct rte_mempool *mbuf_pool;

static const struct rte_eth_conf port_conf_default = {
	.rxmode = {
		.max_rx_pkt_len = ETHER_MAX_LEN,
		.mq_mode = ETH_MQ_RX_RSS,
		.split_hdr_size = 0,
	},
	.rx_adv_conf = {
		.rss_conf = {
			.rss_key = NULL,
			.rss_hf = ETH_RSS_IP,
		},
	},
};

static const struct rte_eth_conf tap_conf_default = {
        .rxmode = {
                .max_rx_pkt_len = ETHER_MAX_LEN,
        },
};

int tun_alloc(char *dev, int flags);

int tun_alloc(char *dev, int flags) {

  struct ifreq ifr;
  int fd, err;
  const char *clonedev = "/dev/net/tun";

  /* Arguments taken by the function:
   *
   * char *dev: the name of an interface (or '\0'). MUST have enough
   *   space to hold the interface name if '\0' is passed
   * int flags: interface flags (eg, IFF_TUN etc.)
   */

   /* open the clone device */
   if( (fd = open(clonedev, O_RDWR)) < 0 ) {
     return fd;
   }

   /* preparation of the struct ifr, of type "struct ifreq" */
   memset(&ifr, 0, sizeof(ifr));

   ifr.ifr_flags = flags;   /* IFF_TUN or IFF_TAP, plus maybe IFF_NO_PI */

   if (*dev) {
     /* if a device name was specified, put it in the structure; otherwise,
      * the kernel will try to allocate the "next" device of the
      * specified type */
     strncpy(ifr.ifr_name, dev, IFNAMSIZ);
   }

   /* try to create the device */
   if( (err = ioctl(fd, TUNSETIFF, (void *) &ifr)) < 0 ) {
     close(fd);
     return err;
   }

  /* if the operation was successful, write back the name of the
   * interface to the variable "dev", so the caller can know
   * it. Note that the caller MUST reserve space in *dev (see calling
   * code below) */
  strcpy(dev, ifr.ifr_name);

  /* this is the special file descriptor that the caller will use to talk
   * with the virtual interface */
  return fd;
}
static void
signal_handler(int signum)
{
        if (signum == SIGINT || signum == SIGTERM) {
		printf("\n\n");

		int i = 0, ret = 0;

		for (i = 0; i< rte_eth_dev_count(); i++)
		{
			uint16_t mtu = 0;
			struct rte_eth_link link = {0};
			struct rte_eth_dev_info dev_info = {0};
			struct rte_eth_rxq_info qinfo = {0};
			struct rte_eth_stats stats = {0};
			struct rte_eth_rss_conf rss_conf = {0};

			printf("  NIC Port %d\n", i);
			printf("+++++++++++++++++++++++++++++++++\n");

			printf("  :::Generic Config:::\n");
			printf("  - Socket %d\n", rte_eth_dev_socket_id(i));
			rte_eth_link_get(i, &link);
			printf("  - link speed %d duplex %d,"
				" auto neg %d status %d\n",
				link.link_speed,
				link.link_duplex,
				link.link_autoneg,
				link.link_status);
			printf("  - promiscous %d\n", rte_eth_promiscuous_get(i));
			ret = rte_eth_dev_get_mtu(i, &mtu);
			if (ret == 0)
				printf("  - mtu %d\n", mtu);

			printf("+++++++++++++++++++++++++++++++++\n");
			printf("  ::: queue stats :::\n");
			
			rte_eth_dev_info_get(i, &dev_info);

			for (int j = 0; j < dev_info.nb_rx_queues; j++) {
				ret = rte_eth_rx_queue_info_get(i, j, &qinfo);
				if (ret == 0) {
					printf("  queue %d\n", j);
					printf("  - rx scatter %d"
						" descriptors %d "
						"  mempool socket %d\n",
						qinfo.scattered_rx,
						qinfo.nb_desc,
						qinfo.mp->socket_id);
				}

				ret = rte_eth_stats_get(i, &stats);
				if (ret == 0) {
					printf("  - packet input %"PRIu64""
						" output %"PRIu64""
						" error %"PRIu64"\n",
						stats.q_ipackets[j],
						stats.q_opackets[j],
						stats.q_errors[j]);
				}
			}

			ret = rte_eth_dev_rss_hash_conf_get(i, &rss_conf);
			if ((ret == 0) && (rss_conf.rss_key != NULL)) {
				printf("  - RSS len %u key (hex): ",
					rss_conf.rss_key_len);
				for (int k = 0; k < rss_conf.rss_key_len; k++)
					printf(" %x", rss_conf.rss_key[k]);
				printf("\n  - hf 0x%"PRIx64"\n",
					rss_conf.rss_hf);
			}

			printf("  ::: stats :::\n");
			
			ret = rte_eth_stats_get(i, &stats);
			if (ret == 0) {
				printf("  - packet input %"PRIu64","
					" output %"PRIu64"\n",
					stats.ipackets,
					stats.opackets);
				printf("  - packet error input %"PRIu64"."
					" output %"PRIu64"\n",
					stats.ierrors,
					stats.oerrors);
				printf("  - RX no mbuf %"PRIu64"\n", stats.rx_nombuf);
			}
			printf("===================================\n");
			
		}

		ret = system("cat /proc/net/dev | grep tap");

		for (i = 0; i < tap_count; i++)
		{
			close(tap_map_port[i][0]);
			if (port_map_tap == 0) {
				close(tap_map_port[i][1]);
				close(tap_map_port[i][2]);
				close(tap_map_port[i][3]);
			}
		}

                rte_exit(EXIT_SUCCESS, "\nSignal %d received, preparing to exit...\n",
                                signum);

        }
}

/*
 * Initializes a given port using global settings and with the RX buffers
 * coming from the mbuf_pool passed as a parameter.
 */
static inline int
port_init(uint16_t port, struct rte_mempool *mbuf_pool)
{
	struct rte_eth_conf port_conf = port_conf_default;
	uint16_t rx_rings = 4, tx_rings = 1;
	uint16_t nb_rxd = RX_RING_SIZE;
	uint16_t nb_txd = TX_RING_SIZE;
	int retval;
	uint16_t q;
	struct rte_eth_dev_info dev_info;
	struct rte_eth_txconf txconf;

	if (!rte_eth_dev_is_valid_port(port))
		return -1;

	rte_eth_dev_info_get(port, &dev_info);

	if ((strncmp("eth_tap", dev_info.driver_name, 7) == 0) ||
	(strncmp("net_tap", dev_info.driver_name, 7) == 0)) {
		printf(" tap interface (%u)\n", port);
		rx_rings = 1;
		port_conf = tap_conf_default;
	}

	/* Configure the Ethernet device. */
	retval = rte_eth_dev_configure(port, rx_rings, tx_rings, &port_conf);
	if (retval != 0)
		return retval;

	/* Allocate and set up RX queue per Ethernet port. */
	for (q = 0; q < rx_rings; q++) {
		retval = rte_eth_rx_queue_setup(port, q, nb_rxd,
				rte_eth_dev_socket_id(port), NULL, mbuf_pool);
		if (retval < 0)
			return retval;
	}

	txconf = dev_info.default_txconf;
	/* Allocate and set up 1 TX queue per Ethernet port. */
	for (q = 0; q < tx_rings; q++) {
		retval = rte_eth_tx_queue_setup(port, q, nb_txd,
				rte_eth_dev_socket_id(port), &txconf);
		if (retval < 0)
			return retval;
	}

	/* Start the Ethernet port. */
	retval = rte_eth_dev_start(port);
	if (retval < 0)
		return retval;

	/* Display the port MAC address. */
	struct ether_addr addr;
	rte_eth_macaddr_get(port, &addr);
	printf("Port %u MAC: %02" PRIx8 " %02" PRIx8 " %02" PRIx8
			   " %02" PRIx8 " %02" PRIx8 " %02" PRIx8 "\n",
			port,
			addr.addr_bytes[0], addr.addr_bytes[1],
			addr.addr_bytes[2], addr.addr_bytes[3],
			addr.addr_bytes[4], addr.addr_bytes[5]);

	/* Enable RX in promiscuous mode for the Ethernet device. */
	rte_eth_promiscuous_enable(port);

	return 0;
}

/*
 * The lcore main. This is the main thread that does the work, reading from
 * an input port and writing to an output port.
 */
static __attribute__((noreturn)) void
lcore_main(void)
{
	uint16_t port;

	/*
	 * Check that the port is on the same NUMA node as the polling thread
	 * for best performance.
	 */
	for (port = 0; port < rte_eth_dev_count(); port++)
		if (rte_eth_dev_socket_id(port) > 0 &&
				rte_eth_dev_socket_id(port) !=
						(int)rte_socket_id())
			printf("WARNING, port %u is on remote NUMA node to "
					"polling thread.\n\tPerformance will "
					"not be optimal.\n", port);

	printf("\nCore %u forwarding packets. [Ctrl+C to quit]\n",
			rte_lcore_id());

	/* Run until the application is quit or killed. */
	for (;;) {
		/*
		 * Receive packets on a port and forward them on the paired
		 * port. The mapping is 0 -> 1, 1 -> 0, 2 -> 3, 3 -> 2, etc.
		 */
		for (port = 0; port < rte_eth_dev_count(); port++)
		{
			uint16_t tx_port = port;

			struct rte_eth_dev_info dev_info = {0};
			rte_eth_dev_info_get(port, &dev_info);

			if (port_map_tap == 0) {
				for (int j = 0; j < dev_info.nb_rx_queues; j++)
				{
					/* Get burst of RX packets, from first port of pair. */
					struct rte_mbuf *bufs[BURST_SIZE];
					const uint16_t nb_rx = rte_eth_rx_burst(port, j,
							bufs, BURST_SIZE);

					if (likely(nb_rx)) {
						for(int k = 0; k < nb_rx; k++)
							if (write(tap_map_port[port][j],
								rte_pktmbuf_mtod(bufs[k], struct ether_hdr *),
								bufs[k]->pkt_len) <= 0)
								printf(" faled to write\n");
					}

					int pkt_size = 0;
					struct rte_mbuf *m = rte_pktmbuf_alloc(mbuf_pool);
					if (m == NULL) {
						printf(" faled to alloc mbuf\n");
						continue;
					}

					tx_port = port;
					pkt_size = read(tap_map_port[port][j],
							rte_pktmbuf_mtod(m, void *), 1500);
					if (pkt_size == 0) {
						printf(" unable to read packet from tap!\n");
						rte_pktmbuf_free(m);
						continue;
					}

					m->nb_segs = 1;
	                		m->next = NULL;
        	        		m->pkt_len = (uint16_t)pkt_size;
                			m->data_len = (uint16_t)pkt_size;

					const uint16_t nb_tx = rte_eth_tx_burst(tx_port, 0, &m, 1);
					if (nb_tx < 1) {
						rte_pktmbuf_free(m);
						printf(" failed to send packet via tx_burst!\n");
						continue;
					}
				}
			}
			else {
				for (int j = 0; j < dev_info.nb_rx_queues; j++)
				{
				/* Get burst of RX packets, from first port of pair. */
				struct rte_mbuf *bufs[BURST_SIZE];
				const uint16_t nb_rx = rte_eth_rx_burst(port, j,
						bufs, BURST_SIZE);

				if (likely(nb_rx)) {
					for(int k = 0; k < nb_rx; k++)
						if (write(tap_map_port[port][0],
							rte_pktmbuf_mtod(bufs[k], struct ether_hdr *),
							bufs[k]->pkt_len) <= 0)
							printf(" faled to write\n");
				}
				}

				int pkt_size = 0;
				struct rte_mbuf *m = rte_pktmbuf_alloc(mbuf_pool);
				if (m == NULL) {
					printf(" faled to alloc mbuf\n");
					continue;
				}

				tx_port = port;
				pkt_size = read(tap_map_port[port][0],
						rte_pktmbuf_mtod(m, void *), 1500);
				if (pkt_size == 0) {
					printf(" unable to read packet from tap!\n");
					rte_pktmbuf_free(m);
					continue;
				}

				m->nb_segs = 1;
	                	m->next = NULL;
        	        	m->pkt_len = (uint16_t)pkt_size;
                		m->data_len = (uint16_t)pkt_size;

				const uint16_t nb_tx = rte_eth_tx_burst(tx_port, 0, &m, 1);
				if (nb_tx < 1) {
					rte_pktmbuf_free(m);
					printf(" failed to send packet via tx_burst!\n");
					continue;
				}

			}
		}
	}
}

/* display usage */
static void
usage(const char *prgname)
{
        printf("%s [EAL options] -- -p PORT to TAP 1:1 \n",
               prgname);
}


static int
parse_args(int argc, char **argv)
{
        int opt;
        char **argvopt;
        char *prgname = argv[0];

        argvopt = argv;

        while ((opt = getopt(argc, argvopt, ":ph")) != EOF) {
		switch (opt)
		{
			case 'p':
				port_map_tap = 1;
				break;

			case 'h':
				usage(prgname);
				return 1;

			case '?':
				printf("\n Unrecognized option!!!\n");
				return -2;

			default:
				usage(prgname);
				return -1;
		}
	}

        if (optind >= 0)
                argv[optind-1] = prgname;

        int ret = optind-1;
        optind = 0; /* reset getopt lib */
        return 0;
}

/*
 * The main function, which does initialization and calls the per-lcore
 * functions.
 */
int
main(int argc, char *argv[])
{
	unsigned nb_ports;
	uint16_t portid;

	/* Initialize the Environment Abstraction Layer (EAL). */
	int ret = rte_eal_init(argc, argv);
	if (ret < 0)
		rte_exit(EXIT_FAILURE, "Error with EAL initialization\n");

	argc -= ret;
	argv += ret;

	/* parse user args if any */
	ret = parse_args(argc, argv);
	if (ret < 0)
		rte_exit(EXIT_FAILURE, "Invalid applciation options\n");
	else if (ret == 1)
		rte_exit(EXIT_SUCCESS, "\n");

	/* Check that there is an even number of ports to send/receive on. */
	nb_ports = rte_eth_dev_count();
	if (nb_ports == 0)
		rte_exit(EXIT_FAILURE, "Error: least 1 port is required.\n");

	/* Creates a new mempool in memory to hold the mbufs. */
	mbuf_pool = rte_pktmbuf_pool_create("MBUF_POOL", NUM_MBUFS * nb_ports,
		MBUF_CACHE_SIZE, 0, RTE_MBUF_DEFAULT_BUF_SIZE, rte_socket_id());

	if (mbuf_pool == NULL)
		rte_exit(EXIT_FAILURE, "Cannot create mbuf pool\n");

	signal(SIGINT, signal_handler);
	signal(SIGTERM, signal_handler);

	/* check DPDK version is > 17.08  */
	printf(" DPDK version: %s\n", rte_version());
	if ((RTE_VER_YEAR < 17) && (RTE_VER_MONTH < 8)) {
		printf(" Current version of DPDK is < 17.08\n");
		dpdk_old_version = 1;
	}

	/* Initialize all ports. */
	for (portid = 0; portid < rte_eth_dev_count(); portid++)
		if (port_init(portid, mbuf_pool) != 0)
			rte_exit(EXIT_FAILURE, "Cannot init port %"PRIu16 "\n",
					portid);

	/* create TAP ports */
	for (portid = 0; portid < nb_ports; portid++)
	{
		struct rte_eth_dev_info dev_info = {0};
		char tapname[25];

		rte_eth_dev_info_get(portid, &dev_info);

		printf(" creating tap port DPDK port (%u) with rx queues (%u)\n",
			portid, dev_info.nb_rx_queues);

		if (port_map_tap == 0) {
			for (int i = 0; i < dev_info.nb_rx_queues; i++) {
				snprintf(tapname, 25, "tap%d-%d", portid, i);

				if (dpdk_old_version == 1) {
					printf("create tap (%s)\n", tapname);
					int tun_fd = tun_alloc(tapname, IFF_TAP | IFF_NO_PI);

					if (tun_fd < 0){
						rte_exit(EXIT_FAILURE, "failed to create tap %s", tapname);
					}

					tap_map_port[portid][i] = tun_fd;
				}
				else
					rte_exit(EXIT_SUCCESS, "Use main.c_1708 for build\n");

				printf(" created tap port (%s)\n", tapname);
				tap_count += 1;
			}
		}
		else {
			snprintf(tapname, 25, "tap%d", portid);
			if (dpdk_old_version == 1) {
				printf("create tap (%s)\n", tapname);
				int tun_fd = tun_alloc(tapname, IFF_TAP | IFF_NO_PI);

				if (tun_fd < 0){
					rte_exit(EXIT_FAILURE, "failed to create tap %s", tapname);
				}

				tap_map_port[portid][0] = tun_fd;
			}
			else
				rte_exit(EXIT_SUCCESS, "Use main.c_1708 for build\n");

			printf(" created tap port (%s)\n", tapname);
			tap_count += 1;
		}
	}
	

	if (rte_lcore_count() > 1)
		printf("\nWARNING: Too many lcores enabled. Only 1 used.\n");

	/* Call lcore_main on the master core only. */
	lcore_main();

	return 0;
}
