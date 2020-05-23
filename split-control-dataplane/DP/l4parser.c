#include "dpcommon.h"

/* G L O B A L S */
struct tcp_hdr *tcp_hdr;
struct udp_hdr *udp_hdr;
struct icmp_hdr *icmp_hdr;
struct sctp_hdr *sctp_hdr;

/* E X T E R N*/
extern dp_parser_func dp_l4parser[0xFFFF];

void dp_parse_tcp(struct rte_mbuf *m ,dp_metadata_t *dp_metadata)
{
    tcp_hdr = (struct tcp_hdr *)(((char *)m->buf_addr) + dp_metadata->l4_offset);
    dp_metadata->tcp        = 1;
    dp_metadata->l4_len     = (tcp_hdr->data_off & 0xf0) >> 2;
    dp_metadata->prev_proto = TCP;
    dp_l4parser[tcp_hdr->dst_port](m,dp_metadata);
}

void dp_parse_udp(struct rte_mbuf *m ,dp_metadata_t *dp_metadata)
{
   // MSG("\n\nGOT UDP");
    udp_hdr = (struct udp_hdr *)(((char *)m->buf_addr) + dp_metadata->l4_offset);
    dp_metadata->udp        = 1;
    dp_metadata->l4_len     = UDP_LEN;
    dp_metadata->prev_proto = UDP;
    dp_l4parser[udp_hdr->dst_port](m,dp_metadata);
}

void dp_parse_icmp(struct rte_mbuf *m ,dp_metadata_t *dp_metadata)
{
    icmp_hdr = (struct icmp_hdr *)(((char *)m->buf_addr) + dp_metadata->l4_offset);
    dp_metadata->icmp =1;
    dp_metadata->dp_cnt.icmp_cnt++;
}

void dp_tcpcnt_incr(struct rte_mbuf *m, dp_metadata_t *dp_metadata)
{
    dp_metadata->dp_cnt.tcp_cnt++;
    dp_metadata->prev_proto = DEFAULT;
    dp_cnt_dummy(m, dp_metadata);
}

void dp_udpcnt_incr(struct rte_mbuf *m, dp_metadata_t *dp_metadata)
{
    dp_metadata->dp_cnt.udp_cnt++;
    dp_metadata->prev_proto = DEFAULT;
    dp_cnt_dummy(m, dp_metadata);
}

