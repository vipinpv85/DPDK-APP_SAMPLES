#ifndef OSI_PARSER_H_
#define OSI_PARSER_H_

#ifdef __cplusplus
extern "C" {
#endif

/* INCLUDE */

/* DEFINE */

/* ENUM */

/* PROTOTYPE */
/* L2 parser */
void dp_parse_eth(struct rte_mbuf *m, dp_metadata_t *dp_metadata);
void dp_ethcnt_incr(struct rte_mbuf *m, dp_metadata_t *dp_metadata);

/* L3 Parser */
void dp_parse_vlan(struct rte_mbuf *m ,dp_metadata_t *dp_metadata);
void dp_parse_ipv4(struct rte_mbuf *m ,dp_metadata_t *dp_metadata);
void dp_parse_ipv6(struct rte_mbuf *m ,dp_metadata_t *dp_metadata);
void dp_parse_arp(struct rte_mbuf *m ,dp_metadata_t *dp_metadata);
void dp_ipv4cnt_incr(struct rte_mbuf *m, dp_metadata_t *dp_metadata);
void dp_ipv6cnt_incr(struct rte_mbuf *m, dp_metadata_t *dp_metadata);
void dp_arpcnt_incr(struct rte_mbuf *m, dp_metadata_t *dp_metadata);

/* L4 Parser */
void dp_parse_tcp(struct rte_mbuf *m ,dp_metadata_t *dp_metadata);
void dp_parse_udp(struct rte_mbuf *m ,dp_metadata_t *dp_metadata);
void dp_parse_icmp(struct rte_mbuf *m ,dp_metadata_t *dp_metadata);
void dp_parse_gre(struct rte_mbuf *m ,dp_metadata_t *dp_metadata);
void dp_parse_sctp(struct rte_mbuf *m ,dp_metadata_t *dp_metadata);
void dp_tcpcnt_incr(struct rte_mbuf *m, dp_metadata_t *dp_metadata);
void dp_udpcnt_incr(struct rte_mbuf *m, dp_metadata_t *dp_metadata);

#ifdef __cplusplus
}
#endif


#endif /* OSI_OPARSER */
