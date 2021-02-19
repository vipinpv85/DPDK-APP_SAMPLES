for (i = 0; i < PREFETCH_OFFSET && i < nb_pkts; i++) {
        rte_prefetch0(pkts[i]);
        rte_prefetch0(rte_pktmbuf_mtod(pkts[i], char *));
}

for (i = 0; i < (nb_pkts - PREFETCH_OFFSET); i++) {
        rte_prefetch0(pkts[i + PREFETCH_OFFSET]);
        rte_prefetch0(rte_pktmbuf_mtod(pkts[i + PREFETCH_OFFSET], char *));

        struct rte_mbuf *m = pkts[i];
        mbuf_priv_data *ptr = rte_mbuf_to_priv(m);
        ptr->queue = qidx;

        if (m->hash.fdir.hi) {
                eth_hdr = rte_pktmbuf_mtod(m, struct rte_ether_hdr *);
                //rte_hexdump (stdout, "l2", (const void *)eth_hdr, 18);
                ptr->l2offset = ((m->packet_type & RTE_PTYPE_L2_ETHER_VLAN) == RTE_PTYPE_L2_ETHER_VLAN) ? 4 :
                                ((m->packet_type & RTE_PTYPE_L2_ETHER_QINQ) == RTE_PTYPE_L2_ETHER_QINQ) ? 8 : 0;
                ipv4_hdr = (RTE_ETH_IS_IPV4_HDR(m->packet_type)) ?
                        (struct rte_ipv4_hdr *)((char *)eth_hdr + sizeof(struct rte_ether_hdr) + ptr->l2offset) : NULL;
                ipv6_hdr = (RTE_ETH_IS_IPV6_HDR(m->packet_type)) ?
                        (struct rte_ipv6_hdr *)((char *)eth_hdr + sizeof(struct rte_ether_hdr) + ptr->l2offset) : NULL;
                //rte_hexdump (stdout, "l3", (const void *)ipv4_hdr, 32);
                //printf("eth_hdr %p ipv4_hdr %p ipv6_hdr %p\n", eth_hdr, ipv4_hdr, ipv6_hdr);
                ptr->l3offset = (ipv4_hdr) ? (ipv4_hdr->version_ihl & RTE_IPV4_HDR_IHL_MASK) * 4 : 40;
                if (ipv4_hdr) {
                        tcp_hdr = (struct rte_tcp_hdr *)((char *)ipv4_hdr + (ipv4_hdr->version_ihl & RTE_IPV4_HDR_IHL_MASK) * 4);
                        udp_hdr = (struct rte_udp_hdr *)((char *)ipv4_hdr + (ipv4_hdr->version_ihl & RTE_IPV4_HDR_IHL_MASK) * 4);
                }
                if (ipv6_hdr) {
                        tcp_hdr = (struct rte_tcp_hdr *)((char *)ipv6_hdr + 40);
                        udp_hdr = (struct rte_udp_hdr *)((char *)ipv6_hdr + 40);
                }
                //rte_hexdump (stdout, "l4", (const void *)tcp_hdr, 8);

                cpu_pkt_stats[lcoreid].tcp_pkts += 1;
                udp_hdr = NULL;
                continue;
        }

        replace[j++] = bufs[i];
        if ((m->packet_type & RTE_PTYPE_L4_UDP) == RTE_PTYPE_L4_UDP) {
                tcp_hdr = NULL;
                cpu_pkt_stats[lcoreid].udp_pkts += 1;
                continue;
        }
        if (!RTE_ETH_IS_IPV4_HDR(m->packet_type) && !RTE_ETH_IS_IPV6_HDR(m->packet_type)) {
                udp_hdr = tcp_hdr = ipv4_hdr = ipv6_hdr = NULL;
                cpu_pkt_stats[lcoreid].nonip_pkts += 1;
                continue;
        }
        cpu_pkt_stats[lcoreid].otherip_pkts += 1;
}

for (; i < (nb_pkts); i++) {
        struct rte_mbuf *m = pkts[i];
        mbuf_priv_data *ptr = rte_mbuf_to_priv(m);
        ptr->queue = qidx;

        if (m->hash.fdir.hi) {
                eth_hdr = rte_pktmbuf_mtod(m, struct rte_ether_hdr *);
                //rte_hexdump (stdout, "l2", (const void *)eth_hdr, 18);
                ptr->l2offset = ((m->packet_type & RTE_PTYPE_L2_ETHER_VLAN) == RTE_PTYPE_L2_ETHER_VLAN) ? 4 :
                                ((m->packet_type & RTE_PTYPE_L2_ETHER_QINQ) == RTE_PTYPE_L2_ETHER_QINQ) ? 8 : 0;
                ipv4_hdr = (RTE_ETH_IS_IPV4_HDR(m->packet_type)) ?
                        (struct rte_ipv4_hdr *)((char *)eth_hdr + sizeof(struct rte_ether_hdr) + ptr->l2offset) : NULL;
                ipv6_hdr = (RTE_ETH_IS_IPV6_HDR(m->packet_type)) ?
                        (struct rte_ipv6_hdr *)((char *)eth_hdr + sizeof(struct rte_ether_hdr) + ptr->l2offset) : NULL;
                //rte_hexdump (stdout, "l3", (const void *)ipv4_hdr, 32);
                //printf("eth_hdr %p ipv4_hdr %p ipv6_hdr %p\n", eth_hdr, ipv4_hdr, ipv6_hdr);
                ptr->l3offset = (ipv4_hdr) ? (ipv4_hdr->version_ihl & RTE_IPV4_HDR_IHL_MASK) * 4 : 40;
                if (ipv4_hdr) {
                        tcp_hdr = (struct rte_tcp_hdr *)((char *)ipv4_hdr + (ipv4_hdr->version_ihl & RTE_IPV4_HDR_IHL_MASK) * 4);
                        udp_hdr = (struct rte_udp_hdr *)((char *)ipv4_hdr + (ipv4_hdr->version_ihl & RTE_IPV4_HDR_IHL_MASK) * 4);
                }
                if (ipv6_hdr) {
                        tcp_hdr = (struct rte_tcp_hdr *)((char *)ipv6_hdr + 40);
                        udp_hdr = (struct rte_udp_hdr *)((char *)ipv6_hdr + 40);
                }
                //rte_hexdump (stdout, "l4", (const void *)tcp_hdr, 8);
                cpu_pkt_stats[lcoreid].tcp_pkts += 1;
                udp_hdr = NULL;
                continue;
        }

        replace[j++] = bufs[i];
        if ((m->packet_type & RTE_PTYPE_L4_UDP) == RTE_PTYPE_L4_UDP) {
                tcp_hdr = NULL;
                cpu_pkt_stats[lcoreid].udp_pkts += 1;
                continue;
        }
        if (!RTE_ETH_IS_IPV4_HDR(m->packet_type) && !RTE_ETH_IS_IPV6_HDR(m->packet_type)) {
                udp_hdr = tcp_hdr = ipv4_hdr = ipv6_hdr = NULL;
                cpu_pkt_stats[lcoreid].nonip_pkts += 1;
                continue;
        }
        cpu_pkt_stats[lcoreid].otherip_pkts += 1;
}

const uint16_t nb_tx = rte_eth_tx_burst(port ^ 1, qidx, replace, j);
/* Free any unsent packets. */
if (unlikely(nb_tx < j)) {
        uint16_t buf;
        for (buf = j; buf < j; buf++)
                rte_pktmbuf_free(bufs[buf]);
}

return (nb_pkts - j);
