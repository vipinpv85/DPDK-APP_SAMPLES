#ifndef __DP_TABLES_H__
#define __DP_TABLES_H__

/* meta data rules */

#define MDPOS_MAC      0
#define MDPOS_VLAN1    1
#define MDPOS_VLAN2    2
#define MDPOS_VLAN3    4
#define MDPOS MPLS1    5
#define MDPOS_MPLS2    6
#define MDPOS_MPLS3    7
#define MDPOS_ARP      8

#define MDPOS_IP4     10
#define MDPOS_ICMP4   11
#define MDPOS_TCP4    12
#define MDPOS_UDP4    13
#define MDPOS_TLS4    14
#define MDPOS_SSL4    15
#define MDPOS_HTTP4   16
#define MDPOS_GTP4    17
#define MDPOS_GRE4    18
#define MDPOS_SMTP4   19
#define MDPOS_SCTP4   20
#define MDPOS_FTP4    21

#define MDPOS_IP6     32
#define MDPOS_ICMP6   33
#define MDPOS_TCP6    34
#define MDPOS_UDP6    35
#define MDPOS_TLS6    36
#define MDPOS_SSL6    37
#define MDPOS_HTTP6   38
#define MDPOS_GTP6    39
#define MDPOS_GRE6    40
#define MDPOS_SMTP6   41
#define MDPOS_SCTP6   42
#define MDPOS_FTP6    43

#define RULE_IS_NOIP  (0x00000000000000FF)
#define RULE_IS_IPV4  (0x00000000FFFFFF00)
#define RULE_IS_IPV6  (0xFFFFFFFF00000000)

#define FLOWDIR_S2D       0x01
#define FLOWDIR_D2S       0x02
#define FLOWDIR_ANY       0x04

/* Port Action */
#define PA_DRP_FRME    0x00
#define PA_DRP_FRAG    0x01
#define PA_ROUTE       0xF1
#define PA_BYPASS      0XFF

#define PORT_ACTION_DROPS (0x0F)

#define S_DPDK_IPV4_ACL "dp_ipv4Table"
#define S_DPDK_IPV6_ACL "dp_ipv6Table"

#define MAX_RULE_COUNT    0xFF


/* Action Order */
#define ACTIONORDER_PASS    0x01
#define ACTIONORDER_DROP    0x02
#define ACTIONORDER_RJCT    0x04
#define ACTIONORDER_ALRT    0x08

#define NUM_FIELDS_IP      6
#define DEFAULT_MAX_CATEGORIES 1

#define DPLKP_IPV4_FLWDR   0
#define DPLKP_IPV4_PROTO   1
#define DPLKP_IPV4_SPORT   2
#define DPLKP_IPV4_DPORT   3
#define DPLKP_IPV4_SRCIP   4
#define DPLKP_IPV4_DSTIP   5

#define DPLKP_IPV6_FLWDR   0
#define DPLKP_IPV6_PROTO   1
#define DPLKP_IPV6_SPORT   2
#define DPLKP_IPV6_DPORT   3
#define DPLKP_IPV6_SRCIP   4
#define DPLKP_IPV6_DSTIP   5


struct dp_ipv4Tuple
{
    uint8_t  flow;
    uint8_t  proto;
    uint16_t sPort;
    uint16_t dPort;
    uint32_t srcIp;
    uint32_t dstIp;
};

struct dp_ipv6Tuple
{
    uint8_t  flow;
    uint8_t  proto;
    uint16_t sPort;
    uint16_t dPort;
    uint32_t srcIp[4];
    uint32_t dstIp[4];
};

struct dp_resultTuple
{
    uint8_t   action;
    uint8_t   next;
    uint16_t  msgIndex;
    uint64_t  counter;
};

struct dp_ruleCount
{
    /* Ethernet: Not included in rules */
    uint8_t sll;
    uint8_t qnq;
    uint8_t ppp;
    uint8_t ppoe;
    uint8_t vlan1;
    uint8_t vlan2;
    uint8_t vlan3;

    /* IP */
    uint8_t ipv4;
    uint8_t ipv6;
    uint8_t tcp;
    uint8_t udp;
    uint8_t icmp;
    uint8_t sctp;
    uint8_t gre;
    uint8_t gtp;

    /* Application */
    uint8_t http;
    uint8_t ssl;
    uint8_t smtp;
    uint8_t ftp;
    uint8_t tls;
    uint8_t dns;
    uint8_t smb;
    uint8_t smb2;
    uint8_t dcerpc;
};


int32_t tableLookupCreate();
int32_t tableLookupDestroy(void);
int32_t tableLookupAddIpv4Rule(struct dp_ipv4Tuple *params, uint32_t userData);
int32_t tableLookup_ipv4Rules_build(void);
int32_t tableLookup_ipv4Rules_lookup(struct dp_ipv4Tuple *params);

#endif /* __DP_TABLES_H__ */
