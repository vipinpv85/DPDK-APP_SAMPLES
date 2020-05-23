#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

#include "config.h"
#include "node.h"

#include <hs.h>
#include <rte_cycles.h>
#include <rte_memcpy.h>

#define MAX_RX_BURST_COUNT 8
#define PREFETCH_OFFSET    4
#define MAX_PORT           2 //Hard coded as 2 

/* GLOABL */
hs_database_t *database = NULL;
char *pattern = NULL; 
char a[5] = {'\0','\0','\0','\0','\0'};

int32_t temp = 0;
uint64_t match = 0;
uint64_t nomatch = 0;



static int eventHandler(unsigned int id, unsigned long long from,
                        unsigned long long to, unsigned int flags, void *ctx) {
    int32_t *val = (int32_t *)ctx;

    (void)(id);
    (void)(from);
    (void)(flags);

    *val = 1;

    //printf("Match for found %d at offset %llu\n", *val, to);
    //fflush(stdout);

    match += 1;
    return 0;
}

static int hyperscan_Handler0(void *arg)
{
    uint8_t port = *((uint8_t *)arg);
    uint8_t adj_port = port ^1;
    unsigned lcore_id, socket_id;
    int32_t j, nb_rx, ret, found = 0;
    unsigned int length = 4;
    
    struct ether_addr src_mac[MAX_PORT];
    struct ether_hdr *ethHdr = NULL;
    struct ipv4_hdr  *ipHdr  = NULL;

    struct rte_mbuf *ptr[MAX_RX_BURST_COUNT], *m = NULL;
    lcore_id = rte_lcore_id();
    socket_id = rte_lcore_to_socket_id(lcore_id);
	
	/* ToDo: remove this hard coding, and populate generic array during setup */
    rte_eth_macaddr_get ( 0 , src_mac); 
    rte_eth_macaddr_get ( 1 , (src_mac + 1));
 
    /* build scratch pad for processing from lookup IP */
    hs_scratch_t *scratch = NULL;
    if (hs_alloc_scratch(database, &scratch) != HS_SUCCESS) {
        fprintf(stderr, "ERROR: Unable to allocate scratch space. Exiting.\n");
        //hs_free_database(database);
        return -1;
    }

    printf("\n arg %d port %d on socket %d \n", *(uint8_t *)arg, port, socket_id);
    fflush(stdout);
	
    while(1)
    {
        /* fetch MAX Burst RX Packets */
        nb_rx =  rte_eth_rx_burst(port, 0, ptr, MAX_RX_BURST_COUNT);

        if(likely(nb_rx)) {
            /* prefetch packets for pipeline */
            for (j = 0; ((j < PREFETCH_OFFSET) && (j < nb_rx)); j++)
            {
                rte_prefetch0(rte_pktmbuf_mtod(ptr[j], void *));
            } /* for loop till PREFETCH_OFFSET */

            for (j = 0; j < (nb_rx - PREFETCH_OFFSET); j++)
            {
                /* Prefetch other packets */
                m = ptr[j];
                rte_prefetch0(rte_pktmbuf_mtod(ptr[j + PREFETCH_OFFSET], void *));

                ethHdr = rte_pktmbuf_mtod(m, struct ether_hdr*);

                if (likely(ethHdr->ether_type == 0x0008)) {
                    ipHdr = (struct ipv4_hdr *) ((char *)(ethHdr + 1));

                    if (hs_scan(database, (const char *) ipHdr, 
                                20, 0, scratch, eventHandler,
                                (void *) &found) != HS_SUCCESS) {
                      fprintf(stderr, "ERROR: Unable to scan input buffer. Exiting.\n");
                      continue;
                    }
					
                    /*Here we are handling L2 forwarding that is pkt 
                      recieved from port 0 is fwd to port 1 and port 
                      1 is fwd to port 0 after changing mac address 
                      here available ports are 0 & 1*/

                    if (likely(found == 1)) {
                        found = 0;
                        *((unsigned int *)(ethHdr->d_addr.addr_bytes)) = *((unsigned int *)(ethHdr->s_addr.addr_bytes));
                        *((short *)(((char  *)ethHdr->d_addr.addr_bytes)+4)) = *((short *)(((char  *)ethHdr->s_addr.addr_bytes)+4)) ;
                        *((unsigned int *)(ethHdr->s_addr.addr_bytes)) = *((unsigned int *)(src_mac[adj_port].addr_bytes));
                        *((short *)(((char  *)ethHdr->s_addr.addr_bytes)+4)) = *((short *)(((char  *)src_mac[adj_port].addr_bytes)+4)) ;
                        ret =  rte_eth_tx_burst(adj_port, 0, &m, 1);
						if (liekly(ret == 1))
      						continue;
                    }
                    else{ 
                       nomatch += 1;
                       //printf("No match : %ld \n",nomatch);
                    }
                }
                rte_pktmbuf_free(m);
                continue;
            }
            for (; j < nb_rx; j++)
            {
                /* Prefetch others packets */
                m = ptr[j];
                ethHdr = rte_pktmbuf_mtod(m, struct ether_hdr*);
                if (likely(ethHdr->ether_type == 0x0008)) {
                    ipHdr = (struct ipv4_hdr *) ((char *)(ethHdr + 1));
                    if (hs_scan(database, (const char *) ipHdr, 
                                20, 0, scratch, eventHandler,
                                (void *)&found) != HS_SUCCESS) {
                      fprintf(stderr, "ERROR: Unable to scan input buffer. Exiting.\n");
                      return -1;
                    }
                    if (likely(found == 1)) {
                        found = 0;
                        *((unsigned int *)(ethHdr->d_addr.addr_bytes)) = *((unsigned int *)(ethHdr->s_addr.addr_bytes));
                        *((short *)(((char  *)ethHdr->d_addr.addr_bytes)+4)) = *((short *)(((char  *)ethHdr->s_addr.addr_bytes)+4)) ;
                        *((unsigned int *)(ethHdr->s_addr.addr_bytes)) = *((unsigned int *)(src_mac[adj_port].addr_bytes));
                        *((short *)(((char  *)ethHdr->s_addr.addr_bytes)+4)) = *((short *)(((char  *)src_mac[adj_port].addr_bytes)+4)) ;
                        ret =  rte_eth_tx_burst(adj_port, 0, &m, 1);
						if (likely(ret == 1))
						    continue;
                    }
                    else{ 
                       nomatch += 1;
                    }
                }
                rte_pktmbuf_free(m);
                continue;
            }
        }

    }
    hs_free_scratch(scratch);
}


static int hyperscan_Handler1(void *arg)
{
    uint8_t port = *((uint8_t *)arg);
    uint8_t adj_port = port ^1;
    unsigned lcore_id, socket_id;
    int32_t j, nb_rx, ret, found = 0;
    unsigned int length = 4;
    
    struct ether_addr src_mac[MAX_PORT];
    struct ether_hdr *ethHdr = NULL;
    struct ipv4_hdr  *ipHdr  = NULL;

    struct rte_mbuf *ptr[MAX_RX_BURST_COUNT], *m = NULL;
    lcore_id = rte_lcore_id();
    socket_id = rte_lcore_to_socket_id(lcore_id);
    rte_eth_macaddr_get ( 0 , src_mac); 
    rte_eth_macaddr_get ( 1 , (src_mac + 1));
	
#if 0 
    printf("%x:%x:%x:%x:%x:%x\n",src_mac[0].addr_bytes[0],src_mac[0].addr_bytes[1],
    src_mac[0].addr_bytes[2],src_mac[0].addr_bytes[3],src_mac[0].addr_bytes[4],
    src_mac[0].addr_bytes[5]);
    printf("%x:%x:%x:%x:%x:%x\n",src_mac[1].addr_bytes[0],src_mac[1].addr_bytes[1],
    src_mac[1].addr_bytes[2],src_mac[1].addr_bytes[3],src_mac[1].addr_bytes[4],
    src_mac[1].addr_bytes[5]);
#endif

    hs_scratch_t *scratch = NULL;
    if (hs_alloc_scratch(database, &scratch) != HS_SUCCESS) {
        fprintf(stderr, "ERROR: Unable to allocate scratch space. Exiting.\n");
        //hs_free_database(database);
        return -1;
    }

    printf("\n arg %d port %d on socket %d \n", *(uint8_t *)arg, port, socket_id);
    fflush(stdout);

    while(1)
    {
        /* fetch MAX Burst RX Packets */
        nb_rx =  rte_eth_rx_burst(port, 0, ptr, MAX_RX_BURST_COUNT);

        if(likely(nb_rx)) {
            //rte_pktmbuf_dump (stdout, ptr[0], 64);
            /* prefetch packets for pipeline */
            for (j = 0; ((j < PREFETCH_OFFSET) &&
                         (j < nb_rx)); j++)
            {
                rte_prefetch0(rte_pktmbuf_mtod(ptr[j], void *));
            } /* for loop till PREFETCH_OFFSET */

            for (j = 0; j < (nb_rx - PREFETCH_OFFSET); j++)
            {
                /* Prefetch others packets */
                m = ptr[j];
                rte_prefetch0(rte_pktmbuf_mtod(ptr[j + PREFETCH_OFFSET], void *));

                ethHdr = rte_pktmbuf_mtod(m, struct ether_hdr*);

                if (likely(ethHdr->ether_type == 0x0008)) {
                    ipHdr = (struct ipv4_hdr *) ((char *)(ethHdr + 1));

#if 1
                    //if (hs_scan(database, (const char *) &ipHdr->src_addr, 
                    //            length, 0, scratch, eventHandler,
                    if (hs_scan(database, (const char *) ipHdr, 
                                20, 0, scratch, eventHandler,
                                (void *) &found) != HS_SUCCESS) {
                      fprintf(stderr, "ERROR: Unable to scan input buffer. Exiting.\n");
                      continue;
                    }
#endif
                    /*Here we are handling L2 forwarding that is pkt 
                      recieved from port 0 is fwd to port 1 and port 
                      1 is fwd to port 0 after changing mac address 
                      here available ports are 0 & 1*/


                    if (likely(found == 1)) {
                        found = 0;
                        *((unsigned int *)(ethHdr->d_addr.addr_bytes)) = *((unsigned int *)(ethHdr->s_addr.addr_bytes));
                        *((short *)(((char  *)ethHdr->d_addr.addr_bytes)+4)) = *((short *)(((char  *)ethHdr->s_addr.addr_bytes)+4)) ;
                        
                        *((unsigned int *)(ethHdr->s_addr.addr_bytes)) = *((unsigned int *)(src_mac[adj_port].addr_bytes));
                        *((short *)(((char  *)ethHdr->s_addr.addr_bytes)+4)) = *((short *)(((char  *)src_mac[adj_port].addr_bytes)+4)) ;

                        //memcpy((&ethHdr->s_addr),(&src_mac[port]),6);

                       ret =  rte_eth_tx_burst(adj_port, 0, &m, 1);
#if 0
                       if (likely(ret == 1)) {
                            //printf("Send to port :%d\n",adj_port);
                            continue;
                        }
                        else{
                            printf("Transmition is failing\n");
                        }
#endif
                    }
                    else{ 
                       nomatch += 1;
                       //printf("No match : %ld \n",nomatch);
                    }
                }
                rte_pktmbuf_free(m);
                continue;
            }
            for (; j < nb_rx; j++)
            {
                /* Prefetch others packets */
                m = ptr[j];
                ethHdr = rte_pktmbuf_mtod(m, struct ether_hdr*);
                if (likely(ethHdr->ether_type == 0x0008)) {
                    ipHdr = (struct ipv4_hdr *) ((char *)(ethHdr + 1));

#if 1
                    //rte_pktmbuf_dump (stdout, ptr[0], 64);
                    //printf("\n Src ip addr: %x", ipHdr->src_addr);
                    //printf("\n Db %p scratch %p", database, scratch);
                    //if (hs_scan(database, (const char *) &ipHdr->src_addr, 
                    //            length, 0, scratch, eventHandler, 
                    if (hs_scan(database, (const char *) ipHdr, 
                                20, 0, scratch, eventHandler,
                                (void *)&found) != HS_SUCCESS) {
                      fprintf(stderr, "ERROR: Unable to scan input buffer. Exiting.\n");
                      return -1;
                    }
#endif

                    if (likely(found == 1)) {
                        found = 0;
                        *((unsigned int *)(ethHdr->d_addr.addr_bytes)) = *((unsigned int *)(ethHdr->s_addr.addr_bytes));
                        *((short *)(((char  *)ethHdr->d_addr.addr_bytes)+4)) = *((short *)(((char  *)ethHdr->s_addr.addr_bytes)+4)) ;
                        *((unsigned int *)(ethHdr->s_addr.addr_bytes)) = *((unsigned int *)(src_mac[adj_port].addr_bytes));
                        *((short *)(((char  *)ethHdr->s_addr.addr_bytes)+4)) = *((short *)(((char  *)src_mac[adj_port].addr_bytes)+4)) ;
                        //memcpy((&ethHdr->s_addr),(&src_mac[port]),6);
                        ret =  rte_eth_tx_burst(adj_port, 0, &m, 1);
#if 0
                        if (likely(ret != 1)) {
                            //printf("Send to port :%d\n",adj_port);
                            continue;
                        }
#endif
                    }
                    else{ 
                       nomatch += 1;
                       //printf("No match : %ld \n",nomatch);
                    }
                }
                rte_pktmbuf_free(m);
                continue;
            }
        }

    }
    /* Scanning is complete, any matches have been handled, so now we just
     * clean up and exit.
     */
    hs_free_scratch(scratch);
}


int main(int argc, char *argv[]) {
   const char* argument[6] = {"hyperscan","-c","f","-n","2"};
   char * pattern;
   int ret = 0;

   /* ToDo: read config, ini and csv to build DB of ip pattern for lookup */
   
   
   while ((argc > 1) && (argv[1][0] == '-'))
    {
        switch (argv[1][1])
        {
            case 'i':
                //printf("%s\n",&argv[1][2]);
                temp = inet_addr( argv[1][2]);
                a[0] = (temp >> 24) & 0xff;
                a[1] = (temp >> 16) & 0xff;
                a[2] = (temp >>  8) & 0xff;
                a[3] = (temp >>  0) & 0xff;
                break;

            case 'p':
                pattern = argv[1][2]);
                break;

            default:
                printf("Wrong Argument: %s\n", argv[1]);
                usage();
        }

        ++argv;
        --argc;
    }



#if 1
   temp = inet_addr( argv[1]);
   a[0] = (temp >> 24) & 0xff;
   a[1] = (temp >> 16) & 0xff;
   a[2] = (temp >>  8) & 0xff;
   a[3] = (temp >>  0) & 0xff;
   

#else

    pattern = argv[1];
    if (strlen(pattern) != 8) {
        fprintf(stderr, "\n pattern %s len %d\n", pattern, (int) strlen(pattern));
        return -1;
    }

   /* convert */
    temp =  (int)strtol(pattern, NULL, 16);
    printf("\n temp: %x\n", temp);

    a[0] = (temp >> 24) & 0xff;
    a[1] = (temp >> 16) & 0xff;
    a[2] = (temp >>  8) & 0xff;
    a[3] = (temp >>  0) & 0xff;
	
#endif

    ret = rte_eal_init(5, (char**) argument);
    if (ret < 0) {
        fprintf(stderr, "Usage: DPDK hyperscan app\n");
        return -1;
    }

    /* check Huge pages for memory buffers */
    ret = rte_eal_has_hugepages();
    if (unlikely(ret < 0)) {
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
    if (interfaceSetup() < 0) {    
        rte_panic("ERROR: interface setup Failed\n");
        exit(EXIT_FAILURE);

    } 

    hs_compile_error_t *compile_err;
    //if (hs_compile(pattern, HS_FLAG_DOTALL, HS_MODE_BLOCK, NULL, &database,
    if (hs_compile(a, HS_FLAG_DOTALL, HS_MODE_BLOCK, NULL, &database,
                   &compile_err) != HS_SUCCESS) {
        fprintf(stderr, "ERROR: Unable to compile pattern \"%s\": %s\n",
                pattern, compile_err->message);
        hs_free_compile_error(compile_err);
        return -1;
    }

	/*
	 * fundamentals task: Stats and OAM, IP pattern lookup, L2fwd and L3fwd
	 */
#if 1
    /*Launch thread in core 1*/
    ret = 0;
    rte_eal_remote_launch(hyperscan_Handler0, (void *)&ret, 1);

    /*Launch thread in core 2*/
    ret = 1;
    rte_eal_remote_launch(hyperscan_Handler1, (void *)&ret, 2);
    /*Launch thread in core 1*/
#endif

    while(1)
        rte_delay_ms(1000);

    /* Scanning is complete, any matches have been handled, so now we just
     * clean up and exit.
     */
    hs_free_database(database);
}

