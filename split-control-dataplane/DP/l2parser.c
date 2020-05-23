#include "dpcommon.h"
#include <unistd.h>

/* EXTERN */
extern struct rte_ring *srb [16];
extern dp_parser_func dp_l2parser[0xFFFF];

char r1[1024][15];
char r2[1024][15];

int32_t etherParse(__attribute__((unused)) void *arg)
{
    int32_t count = 500, core = rte_lcore_id(), deq1, deq2, index = 0;

    MSG(".");
    while ((count--) > 0)
    {
       sleep(1);
       /*MSG("%s core %d!!", __func__, core);*/

       if (core == 2)
       {
           deq1 = rte_ring_dequeue_burst(srb[0], (void *)&r1, 1024);
           /*MSG("RINGBUF0 has %d elements", deq1);*/
           if (deq1)
               for (index = 0; index < deq1; index++)
                   MSG("core %d run %d index %d value %s", core, count, index, 
                                   r1[index]);
       }
       else if (core == 3)
       {
           deq2 = rte_ring_dequeue_burst(srb[1], (void *)&r2, 1024);
           /*MSG("RINGBUF1 has %d elements", deq2)*/;
           if (deq2)
               for (index = 0; index < deq2; index++)
                   MSG("core %d run %d index %d value %s", core, count, index, 
                                   r2[index]);
       }
       else
           MSG("@!@ unknown core %d", core);
    }

    return 0;
}

void dp_parse_eth(struct rte_mbuf *m, dp_metadata_t *dp_metadata)
{
    struct ether_hdr *eth = ((struct ether_hdr *)((char *)m->buf_addr + m->data_off));
    uint16_t ethertype = ntohs(eth->ether_type);
 //   MSG("################ethertype: %u", ethertype);
    dp_l2parser[ethertype](m,dp_metadata);

    return;
}

void dp_ethcnt_incr(struct rte_mbuf *m, dp_metadata_t *dp_metadata)
{
    MSG("################");
    dp_metadata->dp_cnt.eth_cnt++;
    dp_metadata->prev_proto = DEFAULT;
    dp_cnt_dummy(m, dp_metadata);
}

