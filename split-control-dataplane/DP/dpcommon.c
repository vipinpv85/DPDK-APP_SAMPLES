
#include <rte_ethdev.h>

#include <signal.h>
#include "dpcommon.h"
#include "osiparser.h"

/* GLOBAL */
uint8_t nbPorts;
uint8_t pmap [S_DPDK_MAX_ETHPORTS] = {PORTMAP_DROP, PORTMAP_DROP,
                                      PORTMAP_DROP, PORTMAP_DROP};
uint8_t pact [S_DPDK_MAX_ETHPORTS] = {PORTACT_DROP, PORTACT_ACL,
                                      PORTACT_ACL, PORTACT_ACL};
struct rte_mempool * dp_pktmbuf_pool = NULL, *dp_metaDataPool = NULL;
uint8_t config = 0xFF;


dp_metadata_t        dp_metadata[S_DPDK_MAX_ETHPORTS];
dp_parser_func       dp_l2parser[0xFFFF] = {[0x0000 ... 0xFFFE]=dp_parse_pkt_dummy};
dp_parser_func       dp_l3parser[0xFF] = {[0x00 ... 0xFE]=dp_parse_pkt_dummy};
dp_parser_func       dp_l4parser[0xFFFF] = {[0x0000 ... 0xFFFE]=dp_parse_pkt_dummy};
dp_counter_incr_func dp_cnt_incr[10] = {[0 ... 9] = dp_cnt_dummy};
rxfp_stat_t          rxfpCount;

/* STATIC */
static const struct rte_eth_conf portConf = {
    .rxmode = {
              .split_hdr_size = 0,   
              .header_split   = 0, /**< Header Split disabled */
              .hw_ip_checksum = 0, /**< IP checksum offload disabled */
              .hw_vlan_filter = 0, /**< VLAN filtering disabled */
              .jumbo_frame    = 0, /**< Jumbo Frame Support disabled */
              .hw_strip_crc   = 0, /**< CRC stripped by hardware */
              },
    .txmode = {
              .mq_mode = ETH_MQ_TX_NONE,
              },
};

static struct   ether_addr dp_ports_eth_addr [S_DPDK_MAX_ETHPORTS];

void sigHandler(int32_t signo)
{
    if (signo == SIGUSR1)
    {
        MSG("DEBUG: Got Signal %d!!\n", signo);
       // printPktStats();
    }
    else if (signo == SIGUSR2)
    {
        MSG("DEBUG: Got Signal %d!!\n", signo);
        //printErrorStats();
    }
    else if (signo == SIGINT)
    {
        /* Destroy ACL Tables */
        tableLookupDestroy();
        /* Kill Slave Threads */
        /* Kill Master Threads */
        /* Destory Ring Buf */
        /* Flush rte_mbuf  */
        /* Flush meta_data mempool */
        /* Bring down interfaces*/

        exit (signo);
    }
    return;
}


static uint8_t get_dev_count(void)
{
    uint8_t nbPorts = (uint8_t) rte_eth_dev_count();
    if (nbPorts == 0)
        return 0;

    return nbPorts;
}


int32_t checkPortLinkStatus(uint32_t port_mask)
{
    struct rte_eth_link link;
    uint8_t portId, portNum;
    int ret;

    MSG("INFO: Checking links status.....");
    fflush(stdout);

    portNum = get_dev_count();
    if (unlikely(portNum == 0))
    {
         MSG("ERROR: No Intf found!!!");
         return -(INTERFACE);
    }

    MSG("\n\n Total Ports %u\n",portNum);
    for (portId = 0; portId < portNum; portId++) 
    {
        MSG("\n\n@@@@@@@@@@@@@@@@@@Conf:: portId %u\n",portId);
        ret = rte_eth_dev_configure(portId, 1, 1, &portConf);
        if (ret < 0)
        {
            MSG(" ERROR: Eth Config");
            break;
        }

        if ((port_mask & (1 << portId)) == 0)
        {
            continue;
        }

        memset(&link, 0, sizeof(link));
        rte_eth_link_get_nowait(portId, &link);
        if (link.link_status)
        {
            MSG("Port %d Link Up - speed %u Mbps - %s\n", 
               (uint8_t)portId,
               (unsigned)link.link_speed, 
               (link.link_duplex == ETH_LINK_FULL_DUPLEX)?
               ("full-duplex") : ("half-duplex\n"));
        }
        else
        {
            MSG("Port %d Link Down\n", (uint8_t)portId);
        }

        continue;
    }
    return 0;
}

int32_t ethDevSetup(void)
{
    uint8_t portId;
    struct rte_eth_dev_info dev_info;
    int32_t ret = 0;

    nbPorts = rte_eth_dev_count();
    if (nbPorts == 0)
    {
        MSG("ERROR: No Ethernet ports - bye\n");
        return -1;
    }

    if (nbPorts > S_DPDK_MAX_ETHPORTS)
        nbPorts = S_DPDK_MAX_ETHPORTS;

    dp_pktmbuf_pool =
             rte_mempool_create("mbuf_pool", (NB_MBUF - 1),
                        MBUF_SIZE, 32,
                        sizeof(struct rte_pktmbuf_pool_private),
                        rte_pktmbuf_pool_init, NULL,
                        rte_pktmbuf_init, NULL,
                        SOCKET_ID_ANY, MEMPOOL_F_SP_PUT);

    if (unlikely(NULL == dp_pktmbuf_pool))
    {
        MSG("ERROR: mbuf_pool alloc");
        return -1;
    }

    /* Create meta-data pool for packet */
    dp_metaDataPool =
             rte_mempool_create("meta_data", (NB_MBUF - 1),
                        MBUF_SIZE, 32,
                        0,
                        NULL, NULL,
                        NULL, NULL,
                        SOCKET_ID_ANY, 0);

    if (unlikely(NULL == dp_metaDataPool))
    {
        MSG("ERROR: meta_dataBuf alloc");
        return -1;
    }

    for (portId = 0; portId < nbPorts; portId++)
    {
        rte_eth_dev_info_get(portId, &dev_info);  /* TODO: print device info details */
        /* init port */
        printf("Initializing port %u... ", (unsigned) portId);
        fflush(stdout);
        ret = rte_eth_dev_configure(portId, 1, 1, &portConf);
        if (ret < 0)
        {
            /* TODO: free mempool */
            MSG("ERROR: Cannot configure device: err=%d, port=%u\n",
                  ret, (unsigned) portId);
            return -1;
        }

        rte_eth_macaddr_get(portId,&dp_ports_eth_addr[portId]);

        /* init one RX queue */
        fflush(stdout);
        ret = rte_eth_rx_queue_setup(portId, 0, RTE_TEST_RX_DESC_DEFAULT,
                                     SOCKET_ID_ANY,
                                     NULL,
                                     dp_pktmbuf_pool);
        if (ret < 0)
        {
            /* TODO: free mempool */
            MSG("ERROR: rte_eth_rx_queue_setup:err=%d, port=%u\n",
                  ret, (unsigned) portId);
            return -1;
        }
        
        /* init one TX queue on each port */
        fflush(stdout);
        ret = rte_eth_tx_queue_setup(portId, 0, RTE_TEST_TX_DESC_DEFAULT,
                                     SOCKET_ID_ANY,
                                     NULL);
        if (ret < 0)
        {
            MSG("ERROR: rte_eth_tx_queue_setup:err=%d, port=%u\n",
                ret, (unsigned) portId);
            return -1;
        }
    }
    return 0;
}

int32_t ethDevStart(void)
{
    uint8_t portId;
    int32_t ret = 0;

    for (portId = 0; portId < nbPorts; portId++)
    {
        /* Start device */
        ret = rte_eth_dev_start(portId);
        if (ret < 0)
        {
            MSG("ERROR: rte_eth_dev_start:err=%d, port=%u\n",
                  ret, (unsigned) portId);
            return -1;
        }

        MSG("DEBUG: dev start done for Port : %u\n",portId);

        rte_eth_promiscuous_enable(portId);

        MSG("Port %u, MAC address: %02X:%02X:%02X:%02X:%02X:%02X\n\n",
                 (unsigned) portId,
                 dp_ports_eth_addr[portId].addr_bytes[0],
                 dp_ports_eth_addr[portId].addr_bytes[1],
                 dp_ports_eth_addr[portId].addr_bytes[2],
                 dp_ports_eth_addr[portId].addr_bytes[3],
                 dp_ports_eth_addr[portId].addr_bytes[4],
                 dp_ports_eth_addr[portId].addr_bytes[5]);
    }
    return 0;
}


void dp_cnt_dummy(struct rte_mbuf *m, dp_metadata_t *dp_metadata)
{
    //dp_send_packet(m,dp_metadata->portid);
}

void dp_parse_pkt_dummy(struct rte_mbuf *m ,dp_metadata_t *dp_metadata)
{
    dp_cnt_incr[dp_metadata->prev_proto](m, dp_metadata);
    dp_metadata->prev_proto = DEFAULT;
    //dp_send_packet(m,dp_metadata->portid);
}

void initcounterFunctions(void)
{
    dp_cnt_incr[ETH]     = dp_ethcnt_incr;
    dp_cnt_incr[IPv4]    = dp_ipv4cnt_incr;
    dp_cnt_incr[IPv6]    = dp_ipv6cnt_incr;
    dp_cnt_incr[ARP]     = dp_arpcnt_incr;
    dp_cnt_incr[TCP]     = dp_tcpcnt_incr;
    dp_cnt_incr[UDP]     = dp_udpcnt_incr;
    dp_cnt_incr[DEFAULT] = dp_cnt_dummy;
}

void initl2Functions(void)
{
    dp_l2parser[PROTO_IPV4] = dp_parse_ipv4;
    dp_l2parser[PROTO_VLAN] = dp_parse_vlan;
    dp_l2parser[PROTO_IPV6] = dp_parse_ipv6;
    dp_l2parser[PROTO_ARP]  = dp_parse_arp;

    return;
}

void initl3Functions(void)
{
    dp_l3parser[PROTO_ICMP] = dp_parse_icmp;
    dp_l3parser[PROTO_TCP]  = dp_parse_tcp;
    dp_l3parser[PROTO_UDP]  = dp_parse_udp;
    //dp_l4parser[PROTO_SCTP] = dp_parse_sctp;
    //dp_l4parser[PROTO_GRE]  = dp_parse_gre;

    return;
}

#if 0
void initl4Functions(void)
{
    dp_l4parser[0x5000]=dp_parse_http;
    dp_l4parser[0x1600]=dp_parse_ssh;
    dp_l4parser[0x022a]=dp_parse_rtsp;
    dp_l4parser[0x04D2]=dp_parse_rtp;
    dp_l4parser[0x0D3A]=dp_parse_gtp;
    dp_l4parser[0x12B5]=dp_parse_vxlan;
    return;
}
#endif

void printPktStats(void)
{
    uint8_t port = 0;
    MSG("nbPorts: %u", nbPorts);
    #if 0
    for(port = 0; port <nbPorts ; port++)
    {
        printf("\n, For Port%d\n",port);
        printf("\tEth  Count = %"PRIu64"\n", dp_metadata[port].dp_cnt.eth_cnt);
    }
    return;
    #endif
    for(port = 0; port <nbPorts ; port++)
    {
        printf("\n, For Port%d\n",port);
        printf("\tEth  Count = %"PRIu64"\n", dp_metadata[port].dp_cnt.eth_cnt);
        printf("\tIPv4 Count = %"PRIu64"\n", dp_metadata[port].dp_cnt.ipv4_cnt);
        printf("\tIPv6 Count = %"PRIu64"\n", dp_metadata[port].dp_cnt.ipv6_cnt);
        printf("\t\tTCP  Count = %"PRIu64"\n", dp_metadata[port].dp_cnt.tcp_cnt);
        printf("\t\tUDP  Count = %"PRIu64"\n", dp_metadata[port].dp_cnt.udp_cnt);
        printf("\t\t\tHTTP Count = %"PRIu64"\n", dp_metadata[port].dp_cnt.http_cnt);
        printf("\tARP  Count = %"PRIu64"\n", dp_metadata[port].dp_cnt.arp_cnt);
        printf("\t\tICMP  Count = %"PRIu64"\n", dp_metadata[port].dp_cnt.icmp_cnt);
        printf("\tVLAN  Count = %"PRIu64"\n", dp_metadata[port].dp_cnt.vlan_cnt);
    }
    return;
}

void printErrorStats(void)
{
    printf("No errors for now\n"); /*Change this print afterwards */
    return;
}

