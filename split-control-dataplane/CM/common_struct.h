// common_struct.h 
     

#include <stdio.h>


//Generic Rule Information Strucuture
typedef struct
{
    uint8_t next;
    uint8_t msgIndex;
    uint8_t counter;
    char    action[6];
}result_ipv4_t;
typedef struct
{
    uint8_t next;
    uint8_t msgIndex;
    uint8_t counter;
    char    action[6];
}result_ipv6_t;
typedef struct 
{
 //   char action[6];
//    char version[5];
    char flow[5];
    uint8_t proto;
    uint16_t sport;
    uint16_t dport;
    uint32_t sipaddr;
    uint32_t dipaddr;
//    uint32_t miss_cnt;
//    uint32_t success_cnt;
}any_ipv4_rule_t;


typedef struct
{
//    char action[6];
//    char version[5];
    char  flow[5];
    uint8_t proto;
    uint16_t sport;
    uint16_t dport;
    uint64_t sipaddr;
    uint64_t dipaddr;
//    uint64_t miss_cnt;
//    uint64_t success_cnt;
}any_ipv6_rule_t;

enum
{
    rTCP = 1,
    rUDP,
    rSSL,
    rICMP,
    rHTTP,
    rTLS,
    rGRE,
    rGTP,
    rFTP,
    rSCTP,
    rSMTP,
    rDNS,
    rSMB,
    rSMB2,
    rDCERPC,
};

any_ipv4_rule_t tcp_ipv4_rule[10];
any_ipv4_rule_t udp_ipv4_rule[10];
any_ipv4_rule_t tls_ipv4_rule[10];
any_ipv4_rule_t ssl_ipv4_rule[10];
any_ipv4_rule_t icmp_ipv4_rule[10];
any_ipv4_rule_t http_ipv4_rule[10];
any_ipv4_rule_t ftp_ipv4_rule[10];
any_ipv4_rule_t gtp_ipv4_rule[10];
any_ipv4_rule_t gre_ipv4_rule[10];
any_ipv4_rule_t sctp_ipv4_rule[10];
any_ipv4_rule_t smtp_ipv4_rule[10];
any_ipv4_rule_t dcerpc_ipv4_rule[10];
any_ipv4_rule_t dns_ipv4_rule[10];
any_ipv4_rule_t smb_ipv4_rule[10];
any_ipv4_rule_t smb2_ipv4_rule[10];

any_ipv6_rule_t tcp_ipv6_rule[10];
any_ipv6_rule_t udp_ipv6_rule[10];
any_ipv6_rule_t tls_ipv6_rule[10];
any_ipv6_rule_t ssl_ipv6_rule[10];
any_ipv6_rule_t icmp_ipv6_rule[10];
any_ipv6_rule_t http_ipv6_rule[10];
any_ipv6_rule_t ftp_ipv6_rule[10];
any_ipv6_rule_t gtp_ipv6_rule[10];
any_ipv6_rule_t gre_ipv6_rule[10];
any_ipv6_rule_t sctp_ipv6_rule[10];
any_ipv6_rule_t smtp_ipv6_rule[10];
any_ipv6_rule_t dcerpc_ipv6_rule[10];
any_ipv6_rule_t dns_ipv6_rule[10];
any_ipv6_rule_t smb_ipv6_rule[10];
any_ipv6_rule_t smb2_ipv6_rule[10];

result_ipv4_t tcp_ipv4_result[10];
result_ipv4_t udp_ipv4_result[10];
result_ipv4_t tls_ipv4_result[10];
result_ipv4_t ssl_ipv4_result[10];
result_ipv4_t icmp_ipv4_result[10];
result_ipv4_t http_ipv4_result[10];
result_ipv4_t ftp_ipv4_result[10];
result_ipv4_t gtp_ipv4_result[10];
result_ipv4_t gre_ipv4_result[10];
result_ipv4_t sctp_ipv4_result[10];
result_ipv4_t smtp_ipv4_result[10];
result_ipv4_t dcerpc_ipv4_result[10];
result_ipv4_t dns_ipv4_result[10];
result_ipv4_t smb_ipv4_result[10];
result_ipv4_t smb2_ipv4_result[10];

result_ipv6_t tcp_ipv6_result[10];
result_ipv6_t udp_ipv6_result[10];
result_ipv6_t tls_ipv6_result[10];
result_ipv6_t ssl_ipv6_result[10];
result_ipv6_t icmp_ipv6_result[10];
result_ipv6_t http_ipv6_result[10];
result_ipv6_t ftp_ipv6_result[10];
result_ipv6_t gtp_ipv6_result[10];
result_ipv6_t gre_ipv6_result[10];
result_ipv6_t sctp_ipv6_result[10];
result_ipv6_t smtp_ipv6_result[10];
result_ipv6_t dcerpc_ipv6_result[10];
result_ipv6_t dns_ipv6_result[10];
result_ipv6_t smb_ipv6_result[10];
result_ipv6_t smb2_ipv6_result[10];
// Flow stats Information Structure
typedef struct
{
    uint64_t flow_mgr_closed_pruned;
    uint64_t flow_mgr_new_pruned;
    uint64_t flow_mgr_est_pruned;
    uint64_t flow_memuse;
    uint64_t flow_spare;
    uint64_t flow_emerg_mode_entered;
    uint64_t flow_emerg_mode_over;
}flow_stat_t;

flow_stat_t flow_stat;

// Decoder stats Information Structure
typedef struct 
{
    uint64_t decoder_pkts;
    uint64_t decoder_bytes;
    uint64_t decoder_ipv4;
    uint64_t decoder_ipv6;
    uint64_t decoder_ethernet;
    uint64_t decoder_raw;
    uint64_t decoder_sll;
    uint64_t decoder_tcp;
    uint64_t decoder_udp;
    uint64_t decoder_sctp;
    uint64_t decoder_icmpv4;
    uint64_t decoder_icmpv6;
    uint64_t decoder_ppp;
    uint64_t decoder_pppoe;
    uint64_t decoder_gre;
    uint64_t decoder_vlan;
    uint64_t decoder_avg_pkt_size;
    uint64_t decoder_max_pkt_size;
}decoder_stat_t;

decoder_stat_t decoder_stat;
// Defrag stats Information Structure
typedef struct 
{
    uint64_t defrag_ipv4_fragments;
    uint64_t defrag_ipv4_reassembled;
    uint64_t defrag_ipv4_timeouts;
    uint64_t defrag_ipv6_fragments;
    uint64_t defrag_ipv6_reassembled;
    uint64_t defrag_ipv6_timeouts;
}defrag_stat_t;

defrag_stat_t defrag_stat;
// Tcp stats Information Structure
typedef struct 
{
    uint64_t tcp_sessions;
    uint64_t tcp_ssn_memcap_drop;
    uint64_t tcp_pseudo;
    uint64_t tcp_invalid_checksum;
    uint64_t tcp_no_flow;
    uint64_t tcp_reused_ssn;
    uint64_t tcp_memuse;
    uint64_t tcp_syn;
    uint64_t tcp_synack;
    uint64_t tcp_rst;
    uint64_t tcp_segment_memcap_drop;
    uint64_t tcp_stream_depth_reached;
    uint64_t tcp_reassembly_memuse;
    uint64_t tcp_reassembly_gap;
}tcp_stat_t;

tcp_stat_t tcp_stat;
//Detect stats Information Structure
typedef struct 
{
    uint64_t detect_alert;
}detect_stat_t;

detect_stat_t detect_stat;
//Frame stats Information Structure
typedef struct 
{
    uint64_t ipv4_frag;
    uint64_t ipv6_frag;
    uint64_t rx_pkt;
    uint64_t tx_pkt;
    uint64_t rx_byte;
    uint64_t tx_byte;
    uint64_t rx_err;
    uint64_t tx_err;
    uint64_t eth_cnt;
    uint64_t ipv4_cnt;
    uint64_t ipv6_cnt;
    uint64_t tcp_cnt;
    uint64_t http_cnt;
    uint64_t http_data;
    uint64_t udp_cnt;
    uint64_t icmp_cnt;
    uint64_t arp_cnt;
    uint64_t ftp_cnt;
    uint64_t checksum_Err;
}frame_stat_t;

frame_stat_t stat[5];
//frame_stat_t eth1;
//frame_stat_t eth2;
//frame_stat_t eth3;
typedef struct 
{ 
    char port_name[5];
    char link_state[6];
    char duplex[5];
    char pci[9];
    char jumbo[8];
    uint64_t capacity;
    uint64_t speed;
}intf_details_t;

intf_details_t intf[5];

typedef struct 
{
     uint64_t  rx_pkt;
     uint64_t  rx_byte;
     uint64_t  ipv4;
     uint64_t  ipv6;
     uint64_t  tcp;
     uint64_t  udp;
     uint64_t  icmp;
     uint64_t  ssl;
     uint64_t  tls;
     uint64_t  gtp;
     uint64_t  smp;
     uint64_t  ftp;
     uint64_t  smtp;
     uint64_t  sctp;
     uint64_t  dns;
     uint64_t  http;
     uint64_t  gre;
     uint64_t  dcerpc;
     uint64_t  tx_pkt;
     uint64_t  tx_byte;
     uint64_t  pkt_drop;
     uint64_t  failed;
}rxfp_stats_t;
;
rxfp_stats_t rxfp;

typedef struct 
{
    uint64_t  rx_pkt;
    uint64_t  rx_processed;
    uint64_t  acl_lookup;
    uint64_t  acl_match;
    uint64_t  acl_nomatch;
    uint64_t  failed;
    uint64_t  tx_pkts;
} st_stats_t;

st_stats_t st1;
st_stats_t st2;

typedef struct 
{
    uint64_t rx_pkt;
    uint64_t tx_pkt;
    uint64_t tx_byte;
    uint64_t failed;
    uint64_t Err;
}txfw_stats_t;

txfw_stats_t txfw;

