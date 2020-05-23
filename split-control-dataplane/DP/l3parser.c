/* I N C L U D E */
#include "dpcommon.h"

/* G L O B A L S */
struct vlan_hdr *vlan_hdr;
struct ipv4_hdr *ipv4_hdr;
struct ipv6_hdr *ipv6_hdr;
struct arp_hdr  *arp_hdr;

uint8_t l4_proto;

/* E X T E R N */
extern dp_metadata_t dp_metadata[S_DPDK_MAX_ETHPORTS];
extern dp_parser_func dp_l3parser[0xFF];

void dp_parse_vlan(struct rte_mbuf *m ,dp_metadata_t *dp_metadata)
{
    vlan_hdr = (struct vlan_hdr *)(((char *)m->buf_addr) + dp_metadata->l3_offset);
    dp_metadata->vlan=1;
    dp_metadata->dp_cnt.vlan_cnt++;
}

void dp_parse_ipv4(struct rte_mbuf *m ,dp_metadata_t *dp_metadata)
{

    ipv4_hdr = (struct ipv4_hdr *)(((char *)m->buf_addr) + dp_metadata->l3_offset);
    l4_proto = ipv4_hdr->next_proto_id;
    if(ipv4_hdr->fragment_offset ==0x20 || ipv4_hdr->fragment_offset == 0x60 )
    {
        printf("Packet Contains Fragments , so not parsing\n");
    }
    else
    {
        dp_metadata->l3_len     = IPV4_LEN;//+(ipv4_hdr->version_ihl * 4);
        dp_metadata->ipv4       = 1;
        dp_metadata->l4_offset  = dp_metadata->l3_offset + IPV4_LEN; //+(ipv4_hdr->version_ihl * 4);
        dp_metadata->prev_proto = IPv4;
        dp_l3parser[l4_proto](m,dp_metadata);
    }
}

void dp_parse_ipv6(struct rte_mbuf *m ,dp_metadata_t *dp_metadata)
{
    ipv6_hdr = (struct ipv6_hdr *)(((char *)m->buf_addr) + dp_metadata->l3_offset);
    dp_metadata->l3_len = IPV6_LEN;
    dp_l3parser[l4_proto](m,dp_metadata);
    dp_metadata->prev_proto = IPv6;
    dp_metadata->ipv6 = 1;
}

void dp_parse_arp(struct rte_mbuf *m ,dp_metadata_t *dp_metadata)
{
    arp_hdr = (struct arp_hdr *)(((char *)m->buf_addr) + dp_metadata->l3_offset);
    dp_metadata->arp = 1;
    dp_metadata->prev_proto = ARP;

}

void dp_ipv4cnt_incr(struct rte_mbuf *m, dp_metadata_t *dp_metadata)
{
    dp_metadata->dp_cnt.ipv4_cnt++;
    dp_metadata->prev_proto = DEFAULT;
    dp_cnt_dummy(m, dp_metadata);
}

void dp_ipv6cnt_incr(struct rte_mbuf *m, dp_metadata_t *dp_metadata)
{
    dp_metadata->dp_cnt.ipv6_cnt++;
    dp_metadata->prev_proto = DEFAULT;
    dp_cnt_dummy(m, dp_metadata);
}

void dp_arpcnt_incr(struct rte_mbuf *m, dp_metadata_t *dp_metadata)
{
    dp_metadata->dp_cnt.arp_cnt++;
    dp_metadata->prev_proto = DEFAULT;
    dp_cnt_dummy(m, dp_metadata);
}

