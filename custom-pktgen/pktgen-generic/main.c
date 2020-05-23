#include "config.h"
#include "node.h"
#include "stats.h"

/* DEFINES */
#define MAX_TX_BURST_COUNT 8

/* GLOBALS */
uint8_t quitThreads = 0;

/* EXTERN */
extern uint8_t  sigusr2Flag;
extern uint8_t doStatsDisplay;
extern const char   arp[ARP_MAXCOUNT][600];
extern const char icmp4[ICMP4_MAXCOUNT][1000];
extern const char icmp6[ICMP6_MAXCOUNT][1000];
extern const char  ipv4[IPV4_MAXCOUNT][1500];
extern const char  ipv6[IPV6_MAXCOUNT][1500];
extern const char  tcp4[TCP4_MAXCOUNT][1500];
extern const char  tcp6[TCP6_MAXCOUNT][1500];
extern const char  udp4[UDP4_MAXCOUNT][1500];
extern const char  udp6[UDP6_MAXCOUNT][1500];

extern const uint16_t   arpSize[ARP_MAXCOUNT];
extern const uint16_t icmp4Size[ICMP4_MAXCOUNT];
extern const uint16_t icmp6Size[ICMP6_MAXCOUNT];
extern const uint16_t  ipv4Size[IPV4_MAXCOUNT];
extern const uint16_t  ipv6Size[IPV6_MAXCOUNT];
extern const uint16_t  tcp4Size[TCP4_MAXCOUNT];
extern const uint16_t  tcp6Size[TCP6_MAXCOUNT];
extern const uint16_t  udp4Size[UDP4_MAXCOUNT];
extern const uint16_t  udp6Size[UDP6_MAXCOUNT];

extern port_config_t config [PKTGEN_MAXPORTS];
extern numa_Info_t numaNodeInfo[MAX_NUMANODE];
extern pkt_stats_t prtPktStats [PKTGEN_MAXPORTS];


void sigExtraStats(__attribute__((unused)) int signo)
{
    doStatsDisplay = 0;

    /* clear screen */
    STATS_CLR_SCREEN;

    show_static_display();

    doStatsDisplay = 1;
    return;
}

void sigConfig(__attribute__((unused)) int signo)
{
    int8_t port = 0, ports = rte_eth_dev_count();
    int32_t ret;

    doStatsDisplay = 0;

    /* clear screen */
    STATS_CLR_SCREEN;
    printf("\033[11;2H" RED "Reading CONFIG File!!!" RESET);

    fflush(stdout);

    /* stop lcore pkt gen work threads */
    quitThreads =  1;

    /*wait for 1 sec across threads to update */
    rte_delay_ms(4000);

    /* call config read */
    ret = loadConfig();
    if (unlikely(ret < 0))
    {
        printf("\n ERROR: failed to load config\n");
    }

    /* prepare worker threads data */
    for (port = 0; port < ports; port++) 
    {
      /* validate as per traffic selection */
      if((config[port].type == ARP) ) {
          if (unlikely(config[port].pktIndex >= 
                     (int)(sizeof(arp)/sizeof(arp[0]))) ) {
              printf("\n ERROR: Max Pkt for U %d %d\n",
                      (int)(sizeof(arp)/sizeof(arp[0])), 
                      config[port].pktIndex);
              return;
           }
           config[port].pktSize = arpSize[config[port].pktIndex];
      }
      else if((config[port].type == ICMP4)){
          if (unlikely(config[port].pktIndex >=
                     (int)(sizeof(icmp4)/sizeof(icmp4[0]))) ) {
              printf("\n Max Pkt for  C %d %d\n",
                      (int)(sizeof(icmp4)/sizeof(icmp4[0])),
                      config[port].pktIndex);
              return;
           }
           config[port].pktSize = icmp4Size[config[port].pktIndex];
      }
      else if((config[port].type == ICMP6)){
          if (unlikely(config[port].pktIndex >=
                     (int)(sizeof(icmp6)/sizeof(icmp6[0]))) ) {
              printf("\n Max Pkt for  C %d %d\n",
                      (int)(sizeof(icmp6)/sizeof(icmp6[0])),
                      config[port].pktIndex);
              return;
           }
          config[port].pktSize = icmp6Size[config[port].pktIndex];
      }
      else if((config[port].type == IPV4)){
          if (unlikely(config[port].pktIndex >=
                     (int)(sizeof(ipv4)/sizeof(ipv4[0]))) ) {
              printf("\n Max Pkt for  C %d %d\n",
                      (int)(sizeof(ipv4)/sizeof(ipv4[0])),
                      config[port].pktIndex);
              return;
           }
          config[port].pktSize = ipv4Size[config[port].pktIndex];
      }
      else if((config[port].type == IPV6)){
          if (unlikely(config[port].pktIndex >=
                     (int)(sizeof(ipv6)/sizeof(ipv6[0]))) ) {
              printf("\n Max Pkt for  C %d %d\n",
                      (int)(sizeof(ipv6)/sizeof(ipv6[0])),
                      config[port].pktIndex);
              return;
           }
          config[port].pktSize = ipv6Size[config[port].pktIndex];
      }
      else if((config[port].type == TCP4)){
          if (unlikely(config[port].pktIndex >=
                     (int)(sizeof(tcp4)/sizeof(tcp4[0]))) ) {
              printf("\n Max Pkt for  C %d %d\n",
                      (int)(sizeof(tcp4)/sizeof(tcp4[0])),
                      config[port].pktIndex);
              return;
           }
          config[port].pktSize = tcp4Size[config[port].pktIndex];
      }
      else if((config[port].type == TCP6)){
          if (unlikely(config[port].pktIndex >=
                     (int)(sizeof(tcp6)/sizeof(tcp6[0]))) ) {
              printf("\n Max Pkt for  C %d %d\n",
                      (int)(sizeof(tcp6)/sizeof(tcp6[0])),
                      config[port].pktIndex);
              return;
           }
          config[port].pktSize = tcp6Size[config[port].pktIndex];
      }
      else if((config[port].type == UDP4)){
          if (unlikely(config[port].pktIndex >=
                     (int)(sizeof(udp4)/sizeof(udp4[0]))) ) {
              printf("\n Max Pkt for  C %d %d\n",
                      (int)(sizeof(udp4)/sizeof(udp4[0])),
                      config[port].pktIndex);
              return;
           }
          config[port].pktSize = udp4Size[config[port].pktIndex];
      }
      else if((config[port].type == UDP6)){
          if (unlikely(config[port].pktIndex >=
                     (int)(sizeof(udp6)/sizeof(udp6[0]))) ) {
              printf("\n Max Pkt for  C %d %d\n",
                      (int)(sizeof(udp6)/sizeof(udp6[0])),
                      config[port].pktIndex);
              return;
           }
          config[port].pktSize = udp6Size[config[port].pktIndex];
      }
      else {
          printf("\n Prt %d,  \n - Type %d \n , \n Pkt %d", 
                  port,
                  config[port].type,
                  config[port].pktIndex);
          return;
      }
    }

    quitThreads =  0;
    doStatsDisplay = 1;
    show_static_display();

    fflush(stdout);

    return;
}

/*
   Generate traffic 
 */
static int pktgenHandler(void *arg)
{
    uint8_t port = *((uint8_t *)arg);
    struct rte_mempool *mbuf_pool_tx = NULL;
    struct rte_mbuf *ptr[8], *m = NULL;
    unsigned lcore_id, socket_id;
    int32_t i,ret;
    struct ether_hdr * eth_hdr = NULL;

    lcore_id = rte_lcore_id();
    socket_id = rte_lcore_to_socket_id(lcore_id);

    printf("\n Port = %d \n lcore_id = %d \n spcket_id = %d\n", 
               port,lcore_id,socket_id);

    /* ToDo: if mempool is per port ignore the below*/
    mbuf_pool_tx = numaNodeInfo[socket_id].tx[0];
    //mbuf_pool_tx = numaNodeInfo[socket_id].tx[port];

    /* validate as per traffic selection */
    if((config[port].type == ARP) ) {
        if (unlikely(config[port].pktIndex >= 
                   (int)(sizeof(arp)/sizeof(arp[0]))) ) {
            printf("\n ERROR: Max Pkt for U %d %d\n",
                    (int)(sizeof(arp)/sizeof(arp[0])), 
                    config[port].pktIndex);
            return 0;
         }
         config[port].pktSize = arpSize[config[port].pktIndex];
    }
    else if((config[port].type == ICMP4)){
        if (unlikely(config[port].pktIndex >=
                   (int)(sizeof(icmp4)/sizeof(icmp4[0]))) ) {
            printf("\n Max Pkt for  C %d %d\n",
                    (int)(sizeof(icmp4)/sizeof(icmp4[0])),
                    config[port].pktIndex);
            return 0;
         }
         config[port].pktSize = icmp4Size[config[port].pktIndex];
    }
    else if((config[port].type == ICMP6)){
        if (unlikely(config[port].pktIndex >=
                   (int)(sizeof(icmp6)/sizeof(icmp6[0]))) ) {
            printf("\n Max Pkt for  C %d %d\n",
                    (int)(sizeof(icmp6)/sizeof(icmp6[0])),
                    config[port].pktIndex);
            return 0;
         }
        config[port].pktSize = icmp6Size[config[port].pktIndex];
    }
    else if((config[port].type == IPV4)){
        if (unlikely(config[port].pktIndex >=
                   (int)(sizeof(ipv4)/sizeof(ipv4[0]))) ) {
            printf("\n Max Pkt for  C %d %d\n",
                    (int)(sizeof(ipv4)/sizeof(ipv4[0])),
                    config[port].pktIndex);
            return 0;
         }
        config[port].pktSize = ipv4Size[config[port].pktIndex];
    }
    else if((config[port].type == IPV6)){
        if (unlikely(config[port].pktIndex >=
                   (int)(sizeof(ipv6)/sizeof(ipv6[0]))) ) {
            printf("\n Max Pkt for  C %d %d\n",
                    (int)(sizeof(ipv6)/sizeof(ipv6[0])),
                    config[port].pktIndex);
            return 0;
         }
        config[port].pktSize = ipv6Size[config[port].pktIndex];
    }
    else if((config[port].type == TCP4)){
        if (unlikely(config[port].pktIndex >=
                   (int)(sizeof(tcp4)/sizeof(tcp4[0]))) ) {
            printf("\n Max Pkt for  C %d %d\n",
                    (int)(sizeof(tcp4)/sizeof(tcp4[0])),
                    config[port].pktIndex);
            return 0;
         }
        config[port].pktSize = tcp4Size[config[port].pktIndex];
    }
    else if((config[port].type == TCP6)){
        if (unlikely(config[port].pktIndex >=
                   (int)(sizeof(tcp6)/sizeof(tcp6[0]))) ) {
            printf("\n Max Pkt for  C %d %d\n",
                    (int)(sizeof(tcp6)/sizeof(tcp6[0])),
                    config[port].pktIndex);
            return 0;
         }
        config[port].pktSize = tcp6Size[config[port].pktIndex];
    }
    else if((config[port].type == UDP4)){
        if (unlikely(config[port].pktIndex >=
                   (int)(sizeof(udp4)/sizeof(udp4[0]))) ) {
            printf("\n Max Pkt for  C %d %d\n",
                    (int)(sizeof(udp4)/sizeof(udp4[0])),
                    config[port].pktIndex);
            return 0;
         }
        config[port].pktSize = udp4Size[config[port].pktIndex];
    }
    else if((config[port].type == UDP6)){
        if (unlikely(config[port].pktIndex >=
                   (int)(sizeof(udp6)/sizeof(udp6[0]))) ) {
            printf("\n Max Pkt for  C %d %d\n",
                    (int)(sizeof(udp6)/sizeof(udp6[0])),
                    config[port].pktIndex);
            return 0;
         }
        config[port].pktSize = udp6Size[config[port].pktIndex];
    }
    else {
        printf("\n Prt %d,  \n - Type %d \n , \n Pkt %d", 
                port,
                config[port].type,
                config[port].pktIndex);
        return 0;
    }

    while(1)
    {
        ret = 0;

        if( sigusr2Flag & 0x01 )
        {
            if( !(sigusr2Flag & port) )
            {
                sigusr2Flag = sigusr2Flag | port;
                return 0;
            }
        }

        ret = 0;
        for(i = 0; i < MAX_TX_BURST_COUNT; i++ )
        {
            ptr[i] = rte_pktmbuf_alloc(mbuf_pool_tx);
            if (unlikely(ptr[i] == NULL)) {

                /* free the current buffers and retry */
                for (i = i - 1; i >= 0; i--)
                {
                    m = ptr[i];
                    rte_pktmbuf_free(m);
                }

                ret = 0xffffff;
                break; 
            }

            ptr[i]->data_len = config[port].pktSize;
            ptr[i]->pkt_len = config[port].pktSize;

            eth_hdr = rte_pktmbuf_mtod(ptr[i], struct ether_hdr *);
            if(config[port].type == ARP) {
                rte_memcpy(((char *)eth_hdr),arp[config[port].pktIndex],config[port].pktSize);
            }
            else if(config[port].type == ICMP4){
                rte_memcpy(((char *)eth_hdr),icmp4[config[port].pktIndex],ptr[i]->data_len);
            }
            else if(config[port].type == ICMP6){
                rte_memcpy(((char *)eth_hdr),icmp6[config[port].pktIndex],ptr[i]->data_len);
            }
            else if(config[port].type == IPV4){
                rte_memcpy(((char *)eth_hdr),ipv4[config[port].pktIndex],ptr[i]->data_len);
            }
            else if(config[port].type == IPV6){
                rte_memcpy(((char *)eth_hdr),ipv6[config[port].pktIndex],ptr[i]->data_len);
            }
            else if(config[port].type == TCP4){
                rte_memcpy(((char *)eth_hdr),tcp4[config[port].pktIndex],ptr[i]->data_len);
            }
            else if(config[port].type == TCP6){
                rte_memcpy(((char *)eth_hdr),tcp6[config[port].pktIndex],ptr[i]->data_len);
            }
            else if(config[port].type == UDP4){
                rte_memcpy(((char *)eth_hdr),udp4[config[port].pktIndex],ptr[i]->data_len);
            }
            else if(config[port].type == UDP6){
                rte_memcpy(((char *)eth_hdr),udp6[config[port].pktIndex],ptr[i]->data_len);
            }

            ptr[i]->port = port;
            ptr[i]->next = NULL;

       }

        if (unlikely(ret == 0xffffff))
            continue;
        //rte_pktmbuf_dump (stdout, ptr[0], ptr[0]->data_len);

        ret = rte_eth_tx_burst(config[port].prtIndex, 0, (struct rte_mbuf **)&ptr, 8);

        if(config[port].type == ARP  ) {
            prtPktStats[port].tx_arp += ret;
        }
        else if(config[port].type == ICMP4){
            prtPktStats[port].tx_icmp4 += ret;
        }
        else if(config[port].type == ICMP6){
            prtPktStats[port].tx_icmp6 += ret;
        }
        else if(config[port].type == IPV4){
            prtPktStats[port].tx_ipv4 += ret;
        }
        else if(config[port].type == IPV6){
            prtPktStats[port].tx_ipv6 += ret;
        }
        else if(config[port].type == TCP4){
            prtPktStats[port].tx_tcp4 += ret;
        }
        else if(config[port].type == TCP6){
            prtPktStats[port].tx_tcp6 += ret;
        }
        else if(config[port].type == UDP4){
            prtPktStats[port].tx_udp4 += ret;
        }
        else if(config[port].type == UDP6){
            prtPktStats[port].tx_udp6 += ret;
        }
        

        if (unlikely(MAX_TX_BURST_COUNT > ret )) 
        {
            for (i = ret; i < MAX_TX_BURST_COUNT; i++)
            {
                rte_pktmbuf_free(ptr[i]);
            }
        }
    }
}

/*
  Launh per Socket per NUMA aware cores
*/
void launchNumaThread(void)
{
    uint32_t portIndex,coreIndex;
    int32_t i = 0, ret;
    unsigned masterCore = rte_get_master_lcore();

    for (i = 0; i < MAX_NUMANODE; i++)
    {
        /* move out master core from core mask for the socket */
        uint32_t coreMask = numaNodeInfo[i].lcoreAvail ^ (1 << masterCore);
        uint32_t portMask = numaNodeInfo[i].intfAvail;

        /* check to launch on current lcore with interfaces */
        if (likely(numaNodeInfo[i].lcoreTotal && numaNodeInfo[i].intfTotal)) {
            /* let master core on socket 0 run management logic always */
            if (i == 0) {
                //if(unlikely(numaNodeInfo[i].lcoreTotal <= numaNodeInfo[i].intfTotal)) {
                if (0) {
                    printf("\n ERROR: insufficent lcore!!\n");
                    exit(EXIT_FAILURE);
                }
            }

            /* iter intf of each numa */
            portIndex = 0; /* interface Index */
            coreIndex = 0; /* core Index */

            do {
                /* get lcore and port for pktGen */
                portIndex = __builtin_ffs(portMask) - 1;
                coreIndex = __builtin_ffs(coreMask) - 1;

                if ((coreIndex == 0xff) || (portIndex == 0xff)) {
                    break;
                }

                /* clear from mask */
                coreMask = coreMask ^ (1 << (coreIndex));
                portMask = portMask ^ (1 << (portIndex));

                printf("\n INFO: Socket %d core %d port %d", 
                           i, coreIndex, portIndex);
                ret = rte_eal_remote_launch(pktgenHandler,&portIndex,coreIndex);
                if (ret != 0) {
                    printf("\n ERROR: failed to launch on %d for port %d\n",
                            coreMask, portMask);
                    exit(EXIT_FAILURE);
                }

                numaNodeInfo[i].lcoreUsed |= (1 << coreIndex);
                numaNodeInfo[i].intfUsed |= (1 << portIndex);

                numaNodeInfo[i].lcoreInUse += 1;
                numaNodeInfo[i].intfInUse += 1;

                portIndex = coreIndex = 0xff;
            } while ((portMask) && (coreMask));
            /* launch tx thread on each lcore; expcet master */

        }
        else if (numaNodeInfo[i].intfTotal) {
            printf("\n ERROR: Socket %d for Interfaces %d no lcore found!!\n",
                   i, numaNodeInfo[i].intfTotal);
            exit(EXIT_FAILURE);
        }
    } /* end of loop: for NUMA socket iter */
}




int main(int argc, char **argv)
{
    int32_t ret = 0;
    //int32_t j = 0;

    /* if user arguments are present */
    argc -= ret;
    argv += ret;

    /* Load INI configuration for fetching  port details */
    ret = loadConfig();
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

    /* if user arguments are present */
    argc -= ret;
    argv += ret;

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

    /* ToDo: launch per numa socket cores */
    unsigned masterCore = rte_get_master_lcore();
    printf("\n INFO: master Lcore: %u", masterCore);

    launchNumaThread();

    /* Register Signal */
    signal(SIGUSR1, sigExtraStats);
    signal(SIGUSR2, sigConfig);

    set_stats_timer();
    rte_delay_ms(5000);
    show_static_display();

    do {
        rte_delay_ms(1000);
        /* ToDo: check stats display is toggled  */
        rte_timer_manage();
    }while(1);
}

