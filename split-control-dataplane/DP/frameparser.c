#include <unistd.h>

#include "dpcommon.h"
#include "dptables.h"

/* G L O B A L S */
struct dp_ipv4Tuple ipv4Tuple;

/* E X T E R N */
extern struct rte_ring *srb [16];
extern uint8_t       pmap [S_DPDK_MAX_ETHPORTS];
extern uint8_t       pact [S_DPDK_MAX_ETHPORTS];
extern struct        dp_resultTuple aclIpv4Result [1024];
extern struct        dp_resultTuple aclIpv6Result [1024];
extern uint64_t      rulesSet;

extern dp_metadata_t dp_metadata [S_DPDK_MAX_ETHPORTS];
extern rxfp_stat_t   rxfpCount;

/* P R O T O T Y P E */
static void frameDecode(struct rte_mbuf *m, unsigned portid);

/* P R O T O T Y P E */
int32_t frameParseSetup(__attribute__((unused)) void *arg)
{
    int32_t  ret = 0;
    uint32_t i, j, nb_rx, egressPort = 0xFF;
    struct rte_mbuf *pkts_burst[MAX_PKT_BURST];
    struct rte_mbuf *m = NULL;

    MSG("%s core %d!!", __func__, rte_lcore_id());

    while(1)
    {
        /* Read packet from RX queues */
        for (i = 0; i < S_DPDK_MAX_ETHPORTS; i++) 
        {
            /* 4 ports are mapped single lcore 
             * improve this by mapping to a port list */
            egressPort = pmap [i];
            if (unlikely(egressPort ^ PORTMAP_ISDRP))
            {
                MSG("DEBUG: Default PortMap drop : %u", i);
                rxfpCount.pmap_drop++;
                continue;
            }

            /* TODO add the egress port in meta data */

            nb_rx = rte_eth_rx_burst((uint8_t) i, 0,
                         pkts_burst, MAX_PKT_BURST);
       //     MSG("no:of Packets : %u Port::%d \n\n",nb_rx,i);

            for (j = 0; j < nb_rx; j++) 
            {
                m = pkts_burst[j];
                rte_prefetch0(rte_pktmbuf_mtod(m, void *));

                /* Call routine to packet metadata */
                frameDecode(m, i);

                /* if non-ip check port-action to drop or fwd */
//                if (dp_metadata [i].prev_proto )

                /* check meta header if its IP and fragmented*/
                if (unlikely(!(dp_metadata [i].ipv4 | dp_metadata [i].ipv6)))
                {
                    MSG("Non-IP Packets");
                    /* check port action is to drp or fwd */
                    if (unlikely(PORTACT_DROP & pact [i]))
                    {
                        MSG("INFO: non IP frame port action drop");
                        rte_pktmbuf_free(m);
                        rxfpCount.pact_drop++;
                        continue;
                    }
                    else /* fwd port map egress port */
                    {
                        /* TODO tx the frame to egressPort */
                        rte_pktmbuf_free(m);
                        rxfpCount.pfwd++;
                        continue;
                    }
                }

                /* ip fragment then check default port-action */
                if (unlikely(dp_metadata [i].ipv4_fragments)) /* TODO: | dp_metadata [i].ipv6_fragments)) */
                {
                    if (unlikely(PORTACT_FDROP & pact [i]))
                    {
                        MSG("INFO: Fragment IP frame port action drop");
                        rte_pktmbuf_free(m);
                        rxfpCount.f4drp++;
                        continue;
                    }
                    else /* fwd port map egress port */
                    {
                        /* TODO tx the frame to egressPort */
                        rte_pktmbuf_free(m);
                        rxfpCount.pfwd++;
                        continue;
                    }
                }

                /* if IP, check port-action for ACL */
                if (likely(PORTACT_ACL & pact [i]))
                {
                    //MSG("rulesSet: %"PRIu64" IPV4: %u", rulesSet, dp_metadata [i].ipv4);
                    /* If port-action ACL, check protocl set in ruleSet */
                    if ((dp_metadata [i].ipv4) && (rulesSet & RULE_IS_IPV4))
                    {
                        rxfpCount.acl_Lkp++;

                        struct ipv4_hdr *ipv4_hdr = (struct ipv4_hdr *)(((char *)m->buf_addr) + dp_metadata [i].l3_offset);
                        uint32_t info  = ntohl(*((uint32_t *) (((char *)m->buf_addr) + dp_metadata [i].l4_offset)) & 0xffffffff);

                        uint16_t dPort = 0;
                        uint16_t sPort = 0;

                        /* if protocl is tcp or UDO then process */
                        if ((ipv4_hdr->next_proto_id == PROTO_TCP) || 
                            (ipv4_hdr->next_proto_id == PROTO_UDP))
                        {
                            dPort = (info & 0x0000FFFF) >>  0;
                            sPort = (info & 0xFFFF0000) >> 16;
                        }

                       // MSG("sPort: %u dPort : %u", sPort & 0xffff, dPort & 0xffff);

                        /* preapre the ACL rule for lookup */
                        ipv4Tuple.flow  = FLOWDIR_S2D; /* TODO add logic for lfow direction */
                        ipv4Tuple.proto = ipv4_hdr->next_proto_id;
                        ipv4Tuple.sPort = sPort;
                        ipv4Tuple.dPort = dPort;
                        ipv4Tuple.srcIp = ipv4_hdr->src_addr;
                        ipv4Tuple.dstIp = ipv4_hdr->dst_addr;

                        ret = tableLookup_ipv4Rules_lookup(&ipv4Tuple);
                        if (ret > 0)
                        {
#if 0
                            MSG("DEBUG: IPV4 Lookup Port: %u Proto: %u result %d",i, ipv4Tuple.proto, ret);
                            MSG("INFO: action %x next %x msgIndex %x counter %"PRIu64,
                                aclIpv4Result [ret -1].action, 
                                aclIpv4Result [ret -1].next,
                                aclIpv4Result [ret -1].msgIndex,
                                aclIpv4Result [ret -1].counter);
#endif

                           /* update counter for each match */
                           rxfpCount.acl_hit++;
                           aclIpv4Result [ret -1].counter++;

                           /* if lookup success & action is drop|reject*/
                           if ((ACTIONORDER_DROP == aclIpv4Result [ret -1].action) ||
                               (ACTIONORDER_RJCT == aclIpv4Result [ret -1].action))
                           {
                               /* TODO tx the ICMP err frame to egressPort for reject */
                               rte_pktmbuf_free(m);
                               continue;
                           }
                           else if ((ACTIONORDER_PASS == aclIpv4Result [ret -1].action) &&
                                    (0x00 == aclIpv4Result [ret -1].next))
                           {
                               /* TODO tx the frame to egressPort */
                               rte_pktmbuf_free(m);
                               rxfpCount.pfwd++;
                               continue;
                           }

                           //MSG("Before user Frame Process !!!");
                           /* if alert or pass with next action send ST */
                           ret = userFrameProcess((uint32_t) (aclIpv4Result [ret -1].counter), (i&0x01)?3:2, m);
                           rxfpCount.st_fwd++;
                           continue;
                        }
                        else
                        {
                           MSG("DEBUG: IPV4 Lookup Fail Port: %u Proto: %u",i, ipv4Tuple.proto);
                           rxfpCount.acl_miss++;
                        }
                    }
                    else
                    if ((dp_metadata [i].ipv6) && (rulesSet & RULE_IS_IPV6))
                    {
                        /* TODO add suppport for ipv6 */
                    }

                }

                //MSG("Checking for PORTACT_DROP");
                /* if no ACL set check default port-action */
                if (unlikely(PORTACT_DROP & pact [i]))
                {
                    //MSG("INFO: IP frame port action drop");
                    rte_pktmbuf_free(m);
                    rxfpCount.pact_drop++;
                    continue;
                }
                else /* fwd port map egress port */
                {
                    /* TODO tx the frame to egressPort */
                    rte_pktmbuf_free(m);
                    rxfpCount.pfwd++;
                    continue;
                }

                //MSG("Freeing Packet");
                rte_pktmbuf_free(m);
                rxfpCount.pdrop++;
            }
        }
    }

#if 0
       enq = rte_ring_enqueue_burst(srb[0], (void *)r1, 4);
       if (enq != 4)
       {
           MSG("ERROR: why this petti is like this??? %d", enq);
       }

       enq = rte_ring_enqueue_burst(srb[1], (void *)r2, 3);
       if (enq != 3)
       {
           MSG("ERROR: why this dakini is like this??? %d", enq);
       }
#endif

    return 0;
}

static void frameDecode(struct rte_mbuf *m, unsigned portid)
{
    struct ether_hdr *eth;
    void *tmp;
    unsigned dst_port= -1;

    eth = rte_pktmbuf_mtod(m, struct ether_hdr *);

    dp_metadata[(*m).port].portid  = (m)->port;
    dp_metadata_t *dp_metadata_ptr = &dp_metadata[(*m).port];
    dp_metadata_ptr->l2_offset     = (m)->data_off;
    dp_metadata_ptr->l3_offset     = (m)->data_off + ETH_HEADER_LEN;
    dp_metadata_ptr->prev_proto    = ETH;

   // MSG("\n\n ingress port: %u", (m)->port);

    dp_parse_eth(m, dp_metadata_ptr);
#if 0
    dp_send_packet(m, (uint8_t) dst_port);
#endif

    return;
}


