#include <unistd.h>
#include <rte_power.h>

#include "dpcommon.h"

/* EXTERN */
extern struct rte_ring *srb [16];
extern uint8_t config;
extern struct rte_mempool *dp_metaDataPool;

struct rte_mbuf *r1[1024];
struct rte_mbuf *r2[1024];

int32_t userThread(__attribute__((unused)) void *arg)
{
    uint8_t iterCount1 = 0, iterCount2 = 1;
    int32_t deqNum = 0, index = 0;
    uint32_t core = rte_lcore_id(), coreBuff = (core == 2)?0:1;

    /* TODO replace coreBuff with mapping */

    while (config)
    {
       //sleep(1);
       /*MSG("%s core %d!!", __func__, core);*/

       if (core == 2)
       {
           deqNum = rte_ring_dequeue_burst(srb [coreBuff], (void *)&r1, 1024);
           if (likely(deqNum))
           {
               if (unlikely(iterCount1))
               {
                   iterCount1 = 0;
                   /* TODO scale max frequency */
                   rte_power_freq_max(core);
               }

               //MSG("RINGBUF0 has %d elements", deqNum);
               for (index = 0; index < deqNum; index++)
               {
                   //MSG("core %d index %d", core, index);

                   /* Invoke customer function */
                   if (NULL != r1 [index]->userdata)
                   {
                       /*give back the mempool obj */
                       rte_mempool_put(dp_metaDataPool, (void *) r1 [index]->userdata);
                       r1 [index]->userdata = NULL;
                   }

                   /* free the mbuff */
                   rte_pktmbuf_free(r1 [index]);
                   /* TOODO add counters */
               }
           }
           else
           {
               iterCount1 = 1;
             
               /* TODO scale min frequency */
               rte_power_freq_min(core);
           }
       }
       else if (core == 3)
       {
           deqNum = rte_ring_dequeue_burst(srb [coreBuff], (void *)&r2, 1024);
           if (likely(deqNum))
           {
               if (unlikely(iterCount2))
               {
                   iterCount2 = 0;
                   /* TODO scale max frequency */
                   rte_power_freq_max(core);
               }

               //MSG("RINGBUF1 has %d elements", deqNum);
               for (index = 0; index < deqNum; index++)
               {
                   //MSG("core %d index %d ", core, index);

                   /* Invoke customer function */
                   if (NULL != r2 [index]->userdata)
                   {
                       /*give back the mempool obj */
                       rte_mempool_put(dp_metaDataPool, (void *) r2 [index]->userdata);
                       r2 [index]->userdata = NULL;
                   }

                   /* free the mbuff */
                   rte_pktmbuf_free(r2 [index]);
                   /* TOODO add counters */
               }
           }
           else
           {
               iterCount2 = 1;
               /* TODO scale min frequency */
               rte_power_freq_min(core);
           }
       }
       else
       {
           MSG("ALERT: @!@ unknown core %d", core);
           break;
       }
    }

    return 0;
}

int32_t userFrameProcess(uint32_t userData, uint8_t coreId, struct rte_mbuf *m)
{
    int32_t ret = 0;
    uint8_t enq, rb = (coreId == 2)?0:1;
    uint64_t count [2] = {0, 0};

    /* Fetch Object from Mempool */
    /* If failure return -1 */
    ret = rte_mempool_get(dp_metaDataPool, (void *)&userData);
    if (ret != 0)
    {
        MSG("ERROR: Mempool get ret %d coreid %u count %"PRIu64, ret, coreId, count[rb]);
        return -1;
    }

    /* attach obj to mbuf->userdata */
    m->userdata = &userData;

    /* set udata as 0xBABABABA */
    //m->udata64   = 0xBABABABA;

    /* put mbuf to ring buf */
    enq = rte_ring_enqueue_burst(srb [rb], (void *)m, 1);
    if (enq != 1)
    {
        MSG("ERROR: Ring enqueue %d core :%u count %"PRIu64, enq, coreId, count [rb]);
        rte_pktmbuf_free(m);
        return -2;
    }

    count [rb]++;

    /* TODO add counters */
    return 0;
}


