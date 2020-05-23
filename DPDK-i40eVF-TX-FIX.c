static uint16_t
precheck_pkts(uint8_t port __rte_unused, uint16_t qidx __rte_unused,
                struct rte_mbuf **pkts, uint16_t nb_pkts, void *_ __rte_unused)
{
        if (unlikely(nb_pkts == 0))
                return nb_pkts;

        int i, j, k;
        struct rte_mbuf *ptr;
        struct rte_mbuf *replace[nb_pkts];

        for (i = 0, j = 0, k = 0; i < nb_pkts; i++)
        {
                ptr = pkts[i];

                if (ptr->pkt_len < 17) {
                        replace[k++] = ptr;
                } else if (!(ptr->ol_flags & PKT_TX_TCP_SEG)) {
                        if (ptr->nb_segs > 8 ||
                                        ptr->pkt_len > 9728)
                                replace[k++] = ptr;
                        else
                                pkts[j++] = ptr;
                } else if (ptr->nb_segs > 8 ||
                                ptr->tso_segsz < 256 ||
                                ptr->tso_segsz > 9674 ||
                                ptr->pkt_len > 262144) {
                        replace[k++] = ptr;
                } else {
                        pkts[j++] = ptr;
                }
        }

        for (i = 0; i < k; i++)
                rte_pktmbuf_free(replace[i]);

        //printf(" Port (%u) Queue (%u) nb_pkts - old (%u) new (%u) drop (%u)\n", port, qidx, nb_pkts, j, k);
        return j;
}

/*
# DPDK-I40E-VF-FIX
code snippet is tx callback handler which checks for special cases which causes PF to go for hang states

# hOW
update as last code point port_init
`
        if (rte_eth_add_tx_callback(port, 0, precheck_pkts, NULL) == NULL)
                rte_exit(EXIT_FAILURE, "failed to set rte_eth_add_tx_callback");
 */