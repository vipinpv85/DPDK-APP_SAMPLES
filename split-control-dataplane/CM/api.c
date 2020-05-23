// api.c

/*I N C L U D E S */
#include<stdio.h>
#include <string.h>

#include "config.h"
#include "common_struct.h"
#include "api.h"
#include <inttypes.h>

#include <rte_ethdev.h>
#include <rte_acl.h>

#include "dpcommon.h"

/* G L O B A L  V A R I A B L E S */
uint32_t ip;  

char opmode[4] = {0};
char default_dir[8] = {0};
uint8_t pmap_p0 = 6;
uint8_t pmap_p1 = 6;
uint8_t pmap_p2 = 6;
uint8_t pmap_p3 = 6;

uint8_t acl_p4 = 0;
uint8_t acl_p1 = 0;
uint8_t acl_p2 = 0;
uint8_t acl_p3 = 0;
uint8_t fdrp_p4 = 0;
uint8_t fdrp_p1 = 0;
uint8_t fdrp_p2 = 0;
uint8_t fdrp_p3 = 0;
uint8_t drp_p4 = 0;
uint8_t drp_p1 = 0;
uint8_t drp_p2 = 0;
uint8_t drp_p3 = 0;
uint8_t st_p4 = 0;
uint8_t st_p1 = 0;
uint8_t st_p2 = 0;
uint8_t st_p3 = 0;


uint32_t http_ipv4_rule_cnt = 0;
uint32_t ftp_ipv4_rule_cnt = 0;
uint32_t dns_ipv4_rule_cnt = 0;
uint32_t tcp_ipv4_rule_cnt = 0;
uint32_t udp_ipv4_rule_cnt = 0;
uint32_t ssl_ipv4_rule_cnt = 0;
uint32_t tls_ipv4_rule_cnt = 0;
uint32_t gre_ipv4_rule_cnt = 0;
uint32_t gtp_ipv4_rule_cnt = 0;
uint32_t sctp_ipv4_rule_cnt = 0;
uint32_t smtp_ipv4_rule_cnt = 0;
uint32_t smb_ipv4_rule_cnt = 0;
uint32_t smb2_ipv4_rule_cnt = 0;
uint32_t dcerpc_ipv4_rule_cnt = 0;
uint32_t icmp_ipv4_rule_cnt = 0;

uint32_t http_ipv6_rule_cnt = 0;
uint32_t ftp_ipv6_rule_cnt = 0;
uint32_t dns_ipv6_rule_cnt = 0;
uint32_t tcp_ipv6_rule_cnt = 0;
uint32_t udp_ipv6_rule_cnt = 0;
uint32_t ssl_ipv6_rule_cnt = 0;
uint32_t tls_ipv6_rule_cnt = 0;
uint32_t gre_ipv6_rule_cnt = 0;
uint32_t gtp_ipv6_rule_cnt = 0;
uint32_t sctp_ipv6_rule_cnt = 0;
uint32_t smtp_ipv6_rule_cnt = 0;
uint32_t smb_ipv6_rule_cnt = 0;
uint32_t smb2_ipv6_rule_cnt = 0;
uint32_t dcerpc_ipv6_rule_cnt = 0;
uint32_t icmp_ipv6_rule_cnt = 0;

/* G L O B A L  F U N C T I O N   P R O T O T Y P  E  */

extern uint16_t aclIpv4Count;
extern dp_metadata_t dp_metadata [S_DPDK_MAX_ETHPORTS];
extern rxfp_stat_t   rxfpCount;
extern struct   dp_ruleCount  ruleCnt;
extern struct   dp_resultTuple aclIpv4Result [1024];

// API to check rules 
uint32_t ipv4_rule_lookup(any_ipv4_rule_t *any_rule, result_ipv4_t *result, char *action,char *version,
                     uint32_t sip, uint32_t sprt, char *direction,
                     uint32_t dip, uint32_t dprt)
{

    if (!strcmp (result->action, action) && !strcmp (any_rule->flow, direction) && 
       (any_rule->sipaddr == sip) && (any_rule->dipaddr == dip) && 
       (any_rule->sport == sprt) && (any_rule->dport == dprt))
    {
        return 1;
    }

    return 0;
}

uint32_t ipv6_rule_lookup(any_ipv6_rule_t *any_rule, result_ipv6_t *result, char *action,char *version,
                     uint64_t sip, uint32_t sprt, char *direction,
                     uint64_t dip, uint32_t dprt)
{

    if (!strcmp (result->action, action) && !strcmp (any_rule->flow, direction) && 
       (any_rule->sipaddr == sip) && (any_rule->dipaddr == dip) && 
       (any_rule->sport == sprt) && (any_rule->dport == dprt))
    {
        return 1;
    }

    return 0;
}

void show_ipv4_rule(any_ipv4_rule_t *any_rule, result_ipv4_t *result)
{
    uint32_t ip, index = 0;
    static uint32_t bytes1[4];
    static uint32_t bytes2[4];
    
    ip = any_rule->sipaddr;
    bytes1[0] = ip & 0xFF;
    bytes1[1] = (ip >> 8) & 0xFF;
    bytes1[2] = (ip >> 16) & 0xFF;
    bytes1[3] = (ip >> 24) & 0xFF;
 
    ip = any_rule->dipaddr;
      
    bytes2[0] = ip & 0xFF;
    bytes2[1] = (ip >> 8) & 0xFF;
    bytes2[2] = (ip >> 16) & 0xFF;
    bytes2[3] = (ip >> 24) & 0xFF;

    printf("%s, ", result->action);
    printf("%d.%d.%d.%d, %d, ", bytes1[3], bytes1[2], bytes1[1], bytes1[0], any_rule->sport);
    printf("%s, ", any_rule->flow);
    printf("%d.%d.%d.%d, %d\n", bytes2[3], bytes2[2], bytes2[1], bytes2[0], any_rule->dport);

    printf("\n\t\t --- IPv4 ACL ---");
    rte_acl_list_dump();

    printf("\n\t\t --- IPv4 result ---");
    for (index = 0; index < aclIpv4Count; index++)
    {
        printf("\n %10s: %u, %s:%u, %s:%"PRIu64, 
                           "Index", index, 
                           "Action", aclIpv4Result [index].action, 
                           "Counter", aclIpv4Result [index].counter);
    }
}

void show_ipv6_rule(any_ipv6_rule_t *any_rule, result_ipv6_t *result)
{
    uint32_t ip;
    static uint64_t bytes1[4];
    static uint64_t bytes2[4];
    
    ip = any_rule->sipaddr;
    bytes1[0] = ip & 0xFF;
    bytes1[1] = (ip >> 8) & 0xFF;
    bytes1[2] = (ip >> 16) & 0xFF;
    bytes1[3] = (ip >> 24) & 0xFF;
 
    ip = any_rule->dipaddr;
      
    bytes2[0] = ip & 0xFF;
    bytes2[1] = (ip >> 8) & 0xFF;
    bytes2[2] = (ip >> 16) & 0xFF;
    bytes2[3] = (ip >> 24) & 0xFF;

    printf("%s, ", result->action);
    printf("%"PRIu64".%"PRIu64".%"PRIu64".%"PRIu64", %d, ", bytes1[3], bytes1[2], bytes1[1], bytes1[0], any_rule->sport);
    printf("%s, ", any_rule->flow);
    printf("%"PRIu64".%"PRIu64".%"PRIu64".%"PRIu64",", bytes2[3], bytes2[2], bytes2[1], bytes2[0]);
    printf("%d\n", any_rule->dport);
}




//api to check and add tcp rule
uint32_t add_tcp_ipv4_rule(char *action, char *version, uint32_t sip, 
                      uint32_t sprt, char *direction, uint32_t dip, uint32_t dprt)
{
    struct dp_ipv4Tuple ipv4Tuple;
    struct dp_resultTuple rsltTuple;

    ipv4Tuple.flow  = FLOWDIR_ANY;
    ipv4Tuple.proto = PROTO_TCP;
    ipv4Tuple.sPort = (uint16_t)sprt;
    ipv4Tuple.dPort = (uint16_t)dprt;
    ipv4Tuple.srcIp = sip;
    ipv4Tuple.dstIp = dip;

    rsltTuple.action  = ACTIONORDER_PASS;
    rsltTuple.next    = 0xFF;
    rsltTuple.counter = 0x00;

    if (!strcmp("alert", action) == 0)
    {
        rsltTuple.action = ACTIONORDER_ALRT;
    }
    else if (!strcmp("drop", action) == 0)
    {
        rsltTuple.action = ACTIONORDER_DROP;
    }
    else if (!strcmp("reject", action) == 0)
    {
        rsltTuple.action = ACTIONORDER_RJCT;
    }


    updateAcl_ipv4Tcp(&ipv4Tuple, &rsltTuple);

    static uint32_t i;
    uint32_t temp;
//  printf("inside add_tcp_ipv4_rule\n");
    for (temp = 0 ; temp <= i; temp++)
    {
        if (ipv4_rule_lookup(&tcp_ipv4_rule[temp], &tcp_ipv4_result[temp], action, version, sip, sprt, direction, dip, dprt))
        {
            return 0;
        }
    }


//    strcpy(tcp_ipv4_rule[i].version, version);
    strcpy(tcp_ipv4_result[i].action, action);
    strcpy(tcp_ipv4_rule[i].flow, direction);
    tcp_ipv4_rule[i].sipaddr = sip;
    tcp_ipv4_rule[i].sport = sprt;
    tcp_ipv4_rule[i].dipaddr = dip;
    tcp_ipv4_rule[i].dport = dprt;
    tcp_ipv4_rule[i].proto = rTCP;
    

 /* ip = tcp_ipv4_rule[i].sipaddr;
    bytes1[] = ip & 0xFF;
    bytes1[1] = (ip >> 8) & xFF;
    bytes1[2] = (ip >> 16) & xFF;
    bytes1[3] = (ip >> 24) & xFF;
 
    ip = tcp_ipv4_rule[i].dipaddr;
      
    bytes2[] = ip & 0xFF;
    bytes2[1] = (ip >> 8) & xFF;
    bytes2[2] = (ip >> 16) & xFF;
    bytes2[3] = (ip >> 24) & xFF;


    printf("\n%s %s ", tcp_ipv4_rule[i].action, tcp_ipv4_rule[i].version);
    printf("%d.%d.%d.%d %d -> ", bytes1[3], bytes1[2], bytes1[1], bytes1[], tcp_ipv4_rule[i].sport);
    printf("%d.%d.%d.%d %d\n", bytes2[3], bytes2[2], bytes2[1], bytes2[], tcp_ipv4_rule[i].dport);*/
    i++;
    tcp_ipv4_rule_cnt = i;
    return 1;
}


 //api to check rule and udp add 
uint32_t add_udp_ipv4_rule(char *action, char *version, uint32_t sip, 
                      uint32_t sprt, char *direction, uint32_t dip, uint32_t  dprt)
{

    static uint32_t i;
    uint32_t temp;
//  printf("inside add_udp_ipv4_rule\n");
    for (temp = 0 ; temp <= i; temp++)
    {
        if (ipv4_rule_lookup(&udp_ipv4_rule[temp], &udp_ipv4_result[temp], action, version, sip, sprt, direction, dip, dprt))
        {
            return 0;
        }
    }
    strcpy(udp_ipv4_result[i].action, action);
    strcpy(udp_ipv4_rule[i].flow, direction);
    udp_ipv4_rule[i].sipaddr = sip;
    udp_ipv4_rule[i].sport = sprt;
    udp_ipv4_rule[i].dipaddr = dip;
    udp_ipv4_rule[i].dport = dprt;
    udp_ipv4_rule[i].proto = rUDP;

    i++;
    udp_ipv4_rule_cnt = i;
    return 1;
} 


/* API to check rule and gre rules  */
uint32_t add_gre_ipv4_rule(char *action, char *version, uint32_t  sip, 
                 uint32_t sprt, char *direction, uint32_t dip, uint32_t dprt)
{
    static uint32_t i;
    uint32_t temp;
//  printf("inside add_udp_ipv4_rule\n");
    for (temp = 0 ; temp <= i; temp++)
    {
        if (ipv4_rule_lookup(&gre_ipv4_rule[temp], &gre_ipv4_result[temp], action, version, sip, sprt, direction, dip, dprt))
        {
            return 0;
        }
    }
    strcpy(gre_ipv4_result[i].action, action);
    strcpy(gre_ipv4_rule[i].flow, direction);
    gre_ipv4_rule[i].sipaddr = sip;
    gre_ipv4_rule[i].sport = sprt;
    gre_ipv4_rule[i].dipaddr = dip;
    gre_ipv4_rule[i].dport = dprt;
    gre_ipv4_rule[i].proto = rGRE;




    i++;
    gre_ipv4_rule_cnt = i;
    return 1;
} 

/* api to check and gtpc rule */

uint32_t add_gtp_ipv4_rule( char *action, char *version, uint32_t sip, 
                        uint32_t sprt, char *direction, uint32_t dip, uint32_t  dprt)
{
    static uint32_t i;
    uint32_t temp;
//  printf("inside add_gtpc_rule\n");
    for (temp = 0; temp <= i; temp++)
    {
        if (ipv4_rule_lookup(&gtp_ipv4_rule[temp], &gtp_ipv4_result[temp], action, version, sip, sprt, direction, dip, dprt))
        {
            return 0;
        }
    }
    strcpy(gtp_ipv4_result[i].action, action);
    strcpy(gtp_ipv4_rule[i].flow, direction);
    gtp_ipv4_rule[i].sipaddr = sip;
    gtp_ipv4_rule[i].sport = sprt;
    gtp_ipv4_rule[i].dipaddr = dip;
    gtp_ipv4_rule[i].dport = dprt;
    gtp_ipv4_rule[i].proto = rGTP;


    i++;
    gtp_ipv4_rule_cnt = i;
    return 1;
} 



/* API to check and gtpu rules */
uint32_t add_ftp_ipv4_rule(char *action, char *version, uint32_t sip, 
                       uint32_t sprt, char *direction, uint32_t dip, uint32_t dprt)
{
    static uint32_t i;
    uint32_t temp;
//  printf("inside add_gtpu_rule\n");
    for (temp = 0; temp <= i; temp++)
    {
        if (ipv4_rule_lookup(&ftp_ipv4_rule[temp], &ftp_ipv4_result[temp], action, version, sip, sprt, direction, dip, dprt))
        {
            return 0;
        }
    }
    strcpy(ftp_ipv4_result[i].action, action);
    strcpy(ftp_ipv4_rule[i].flow, direction);
    ftp_ipv4_rule[i].sipaddr = sip;
    ftp_ipv4_rule[i].sport = sprt;
    ftp_ipv4_rule[i].dipaddr = dip;
    ftp_ipv4_rule[i].dport = dprt;
    ftp_ipv4_rule[i].proto = rFTP;


    i++;
    ftp_ipv4_rule_cnt = i;
    return 1;
} 


/* API to check and add nvgre rules */
uint32_t add_sctp_ipv4_rule(char *action, char *version, uint32_t sip, 
                        uint32_t sprt, char *direction, uint32_t dip, uint32_t dprt)
{
    static uint32_t i;
    uint32_t temp;
//  printf("inside add_nvgre_ipv4_rule\n");
    for (temp = 0; temp <= i; temp++)
    {
        if (ipv4_rule_lookup(&sctp_ipv4_rule[temp], &sctp_ipv4_result[temp], action, version, sip, sprt, direction, dip, dprt))
        {
            return 0;
        }
    }
    strcpy(sctp_ipv4_result[i].action, action);
    strcpy(sctp_ipv4_rule[i].flow, direction);
    sctp_ipv4_rule[i].sipaddr = sip;
    sctp_ipv4_rule[i].sport = sprt;
    sctp_ipv4_rule[i].dipaddr = dip;
    sctp_ipv4_rule[i].dport = dprt;
    sctp_ipv4_rule[i].proto = rSCTP;


    i++;
    sctp_ipv4_rule_cnt = i;
    return 1;
} 

uint32_t add_smtp_ipv4_rule(char *action, char *version, uint32_t sip, 
                        uint32_t sprt, char *direction, uint32_t dip, uint32_t dprt)
{
    static uint32_t i;
    uint32_t temp;
//  printf("inside add_nvgre_ipv4_rule\n");
    for (temp = 0; temp <= i; temp++)
    {
        if (ipv4_rule_lookup(&smtp_ipv4_rule[temp], &smtp_ipv4_result[temp], action, version, sip, sprt, direction, dip, dprt))
        {
            return 0;
        }
    }
    strcpy(smtp_ipv4_result[i].action, action);
    strcpy(smtp_ipv4_rule[i].flow, direction);
    smtp_ipv4_rule[i].sipaddr = sip;
    smtp_ipv4_rule[i].sport = sprt;
    smtp_ipv4_rule[i].dipaddr = dip;
    smtp_ipv4_rule[i].dport = dprt;
    smtp_ipv4_rule[i].proto = rSMTP;


    i++;
    smtp_ipv4_rule_cnt = i;
    return 1;
} 
uint32_t add_dns_ipv4_rule(char *action, char *version, uint32_t sip, 
                        uint32_t sprt, char *direction, uint32_t dip, uint32_t dprt)
{
    static uint32_t i;
    uint32_t temp;
//  printf("inside add_nvgre_ipv4_rule\n");
    for (temp = 0; temp <= i; temp++)
    {
        if (ipv4_rule_lookup(&dns_ipv4_rule[temp], &dns_ipv4_result[temp], action, version, sip, sprt, direction, dip, dprt))
        {
            return 0;
        }
    }
    strcpy(dns_ipv4_result[i].action, action);
    strcpy(dns_ipv4_rule[i].flow, direction);
    dns_ipv4_rule[i].sipaddr = sip;
    dns_ipv4_rule[i].sport = sprt;
    dns_ipv4_rule[i].dipaddr = dip;
    dns_ipv4_rule[i].dport = dprt;
    dns_ipv4_rule[i].proto = rDNS;


    i++;
    dns_ipv4_rule_cnt = i;
    return 1;
} 


/* API to check and tls rules */
uint32_t add_tls_ipv4_rule(char *action, char *version, uint32_t sip, 
                      uint32_t sprt, char *direction, uint32_t dip, uint32_t dprt)
{
      static uint32_t i;
   uint32_t temp;
// printf("inside add_tls_ipv4_rule\n");
   for (temp = 0; temp <= i; temp++)
   {
       if (ipv4_rule_lookup(&tls_ipv4_rule[temp], &tls_ipv4_result[temp], action, version, sip, sprt, direction, dip, dprt))
       {
           return 0;
       }
   }
    strcpy(tls_ipv4_result[i].action, action);
    strcpy(tls_ipv4_rule[i].flow, direction);
    tls_ipv4_rule[i].sipaddr = sip;
    tls_ipv4_rule[i].sport = sprt;
    tls_ipv4_rule[i].dipaddr = dip;
    tls_ipv4_rule[i].dport = dprt;
    tls_ipv4_rule[i].proto = rTLS;


   i++;
   tls_ipv4_rule_cnt = i;
   return 1;
  //api to check rule and add 
}



/* API to check and add ssl rules */ 
uint32_t add_ssl_ipv4_rule(char *action, char *version, uint32_t sip, 
                      uint32_t sprt, char *direction, uint32_t dip, uint32_t dprt)
{
   static uint32_t i;
   uint32_t temp;
// printf("inside add_ssl_ipv4_rule\n");
   for (temp = 0; temp <= i; temp++)
   {
       if (ipv4_rule_lookup(&ssl_ipv4_rule[temp], &ssl_ipv4_result[temp], action, version, sip, sprt, direction, dip, dprt))
       {
           return 0;
       }
   }
    strcpy(ssl_ipv4_result[i].action, action);
    strcpy(ssl_ipv4_rule[i].flow, direction);
    ssl_ipv4_rule[i].sipaddr = sip;
    ssl_ipv4_rule[i].sport = sprt;
    ssl_ipv4_rule[i].dipaddr = dip;
    ssl_ipv4_rule[i].dport = dprt;
    ssl_ipv4_rule[i].proto = rSSL;


   i++;
   ssl_ipv4_rule_cnt = i;
   return 1;
}



/* API to check and vlan1 rules */
uint32_t add_smb_ipv4_rule(char *action, char *version, uint32_t sip, 
                        uint32_t sprt, char *direction, uint32_t dip, uint32_t dprt)
{
     static uint32_t i;
   uint32_t temp;
// printf("inside add_vlan1_rule\n");
   for (temp = 0; temp <= i; temp++)
   {
       if (ipv4_rule_lookup(&smb_ipv4_rule[temp], &smb_ipv4_result[temp], action, version, sip, sprt, direction, dip, dprt))
       {
           return 0;
       }
   }
    strcpy(smb_ipv4_result[i].action, action);
    strcpy(smb_ipv4_rule[i].flow, direction);
    smb_ipv4_rule[i].sipaddr = sip;
    smb_ipv4_rule[i].sport = sprt;
    smb_ipv4_rule[i].dipaddr = dip;
    smb_ipv4_rule[i].dport = dprt;
    smb_ipv4_rule[i].proto = rSMB;


   i++;
   smb_ipv4_rule_cnt = i;
   return 1;
}

uint32_t add_smb2_ipv4_rule(char *action, char *version, uint32_t sip, 
                        uint32_t sprt, char *direction, uint32_t dip, uint32_t dprt)
{
     static uint32_t i;
   uint32_t temp;
// printf("inside add_vlan1_rule\n");
   for (temp = 0; temp <= i; temp++)
   {
       if (ipv4_rule_lookup(&smb2_ipv4_rule[temp], &smb2_ipv4_result[temp], action, version, sip, sprt, direction, dip, dprt))
       {
           return 0;
       }
   }
    strcpy(smb2_ipv4_result[i].action, action);
    strcpy(smb2_ipv4_rule[i].flow, direction);
    smb2_ipv4_rule[i].sipaddr = sip;
    smb2_ipv4_rule[i].sport = sprt;
    smb2_ipv4_rule[i].dipaddr = dip;
    smb2_ipv4_rule[i].dport = dprt;
    smb2_ipv4_rule[i].proto = rSMB2;


   i++;
   smb2_ipv4_rule_cnt = i;
   return 1;

}
/* API to check and add vlan2 rule */
uint32_t add_dcerpc_ipv4_rule(char *action, char *version, uint32_t sip, 
                        uint32_t sprt, char *direction, uint32_t dip, uint32_t dprt)
{
     static uint32_t i;
   uint32_t temp;
// printf("inside add_vlan2_rule\n");
   for (temp = 0; temp <= i; temp++)
   {
       if (ipv4_rule_lookup(&dcerpc_ipv4_rule[temp], &dcerpc_ipv4_result[temp], action, version, sip, sprt, direction, dip, dprt))
       {
           return 0;
       }
   }
    strcpy(dcerpc_ipv4_result[i].action, action);
    strcpy(dcerpc_ipv4_rule[i].flow, direction);
    dcerpc_ipv4_rule[i].sipaddr = sip;
    dcerpc_ipv4_rule[i].sport = sprt;
    dcerpc_ipv4_rule[i].dipaddr = dip;
    dcerpc_ipv4_rule[i].dport = dprt;
    dcerpc_ipv4_rule[i].proto = rDCERPC;


   i++;
   dcerpc_ipv4_rule_cnt = i;
   return 1;
}


/* API  to check and add vlan3 rule */
uint32_t add_http_ipv4_rule(char *action, char *version, uint32_t sip, 
                        uint32_t sprt, char *direction, uint32_t dip, uint32_t dprt)
{
   static uint32_t i;
   uint32_t temp;
// printf("inside add_vlan3_rule\n");
   for (temp = 0; temp <= i; temp++)
   {
       if (ipv4_rule_lookup(&http_ipv4_rule[temp], &http_ipv4_result[temp], action, version, sip, sprt, direction, dip, dprt))
       {
           return 0;
       }
   }
    strcpy(http_ipv4_result[i].action, action);
    strcpy(http_ipv4_rule[i].flow, direction);
    http_ipv4_rule[i].sipaddr = sip;
    http_ipv4_rule[i].sport = sprt;
    http_ipv4_rule[i].dipaddr = dip;
    http_ipv4_rule[i].dport = dprt;
    http_ipv4_rule[i].proto = rHTTP;


   i++;
   http_ipv4_rule_cnt = i;
   return 1;

}


/* API  to check and add vxlan rule */ 
uint32_t add_icmp_ipv4_rule(char *action, char *version, uint32_t sip, 
                        uint32_t sprt, char *direction, uint32_t dip, uint32_t dprt)
{
   static uint32_t i;
   uint32_t temp;
// printf("inside add_vxlan_rule\n");
   for (temp = 0; temp <= i; temp++)
   {
       if (ipv4_rule_lookup(&icmp_ipv4_rule[temp], &icmp_ipv4_result[temp], action, version, sip, sprt, direction, dip, dprt))
       {
           return 0;
       }
   }
    strcpy(icmp_ipv4_result[i].action, action);
    strcpy(icmp_ipv4_rule[i].flow, direction);
    icmp_ipv4_rule[i].sipaddr = sip;
    icmp_ipv4_rule[i].sport = sprt;
    icmp_ipv4_rule[i].dipaddr = dip;
    icmp_ipv4_rule[i].dport = dprt;
    icmp_ipv4_rule[i].proto = rICMP;


   i++;
   icmp_ipv4_rule_cnt = i;
   return 1;
}


// function to print the rule format


// API to print stats
void show_stats( uint8_t status )
{

 if (status == 1 )
 {
    printf("\n\t ----------- Flow -----------\n");
    printf("%30s : %"PRId64" \n", "flow_mgr.closed_pruned",flow_stat.flow_mgr_closed_pruned);
    printf("%30s : %"PRId64" \n", "flow_mgr.new_pruned",flow_stat.flow_mgr_new_pruned);
    printf("%30s : %"PRId64" \n", "flow_mgr.est_pruned",flow_stat.flow_mgr_est_pruned);
    printf("%30s : %"PRId64" \n", "flow.memuse", flow_stat.flow_memuse);
    printf("%30s : %"PRId64" \n", "flow.spare", flow_stat.flow_spare);
    printf("%30s : %"PRId64" \n", "flow.emerg_mode_entered",flow_stat.flow_emerg_mode_entered);
    printf("%30s : %"PRId64" \n", "flow.emerg_mode_over",
            flow_stat.flow_emerg_mode_over);
}
if (status == 2)
{ 
    printf("\n\t ----------- Decoder -----------\n");
    printf("%30s : %"PRId64" \n", "decoder.pkts", decoder_stat.decoder_pkts);
    printf("%30s : %"PRId64" \n", "decoder.bytes", decoder_stat.decoder_bytes);
    printf("%30s : %"PRId64" \n", "decoder.ipv4", decoder_stat.decoder_ipv4);
    printf("%30s : %"PRId64" \n", "decoder.ipv6", decoder_stat.decoder_ipv6);
    printf("%30s : %"PRId64" \n", "decoder.ethernet",decoder_stat.decoder_ethernet);
    printf("%30s : %"PRId64" \n", "decoder.raw", decoder_stat.decoder_raw);
    printf("%30s : %"PRId64" \n", "decoder.sll", decoder_stat.decoder_sll);
    printf("%30s : %"PRId64" \n", "decoder.tcp", decoder_stat.decoder_tcp);
    printf("%30s : %"PRId64" \n", "decoder.udp", decoder_stat.decoder_udp);
    printf("%30s : %"PRId64" \n", "decoder.sctp", decoder_stat.decoder_sctp);
    printf("%30s : %"PRId64" \n", "decoder.icmpv4", decoder_stat.decoder_icmpv4);
    printf("%30s : %"PRId64" \n", "decoder.icmpv6", decoder_stat.decoder_icmpv6);
    printf("%30s : %"PRId64" \n", "decoder.ppp", decoder_stat.decoder_ppp);
    printf("%30s : %"PRId64" \n", "decoder.pppoe", decoder_stat.decoder_pppoe);
    printf("%30s : %"PRId64" \n", "decoder.gre", decoder_stat.decoder_gre);
    printf("%30s : %"PRId64" \n", "decoder.vlan", decoder_stat.decoder_vlan);
    printf("%30s : %"PRId64" \n", "decoder.avg_pkt_size", 
            decoder_stat.decoder_avg_pkt_size);
    printf("%30s : %"PRId64" \n", "decoder.max_pkt_size", 
            decoder_stat.decoder_max_pkt_size);
}
if (status == 3)
{
    printf("\n\t ----------- TCP ------------\n");
    printf("%30s : %"PRId64" \n", "tcp.sessions", tcp_stat.tcp_sessions);
    printf("%30s : %"PRId64" \n", "tcp.ssn_memcap_drop", 
            tcp_stat.tcp_ssn_memcap_drop);
    printf("%30s : %"PRId64" \n", "tcp.pseudo", tcp_stat.tcp_pseudo);
    printf("%30s : %"PRId64" \n", "tcp.invalid_checksum", 
            tcp_stat.tcp_invalid_checksum);
    printf("%30s : %"PRId64" \n", "tcp.no_flow", tcp_stat.tcp_no_flow);
    printf("%30s : %"PRId64" \n", "tcp.reused_ssn", tcp_stat.tcp_reused_ssn);
    printf("%30s : %"PRId64" \n", "tcp.memuse", tcp_stat.tcp_memuse);
    printf("%30s : %"PRId64" \n", "tcp.syn", tcp_stat.tcp_syn);
    printf("%30s : %"PRId64" \n", "tcp.synack", tcp_stat.tcp_synack);
    printf("%30s : %"PRId64" \n", "tcp.rst", tcp_stat.tcp_rst);
    printf("%30s : %"PRId64" \n", "tcp.segment_memcap_drop", 
            tcp_stat.tcp_segment_memcap_drop);
    printf("%30s : %"PRId64" \n", "tcp.stream_depth_reached", 
            tcp_stat.tcp_stream_depth_reached);
    printf("%30s : %"PRId64" \n", "tcp.reassembly_memuse", 
            tcp_stat.tcp_reassembly_memuse);
    printf("%30s : %"PRId64" \n", "tcp.reassembly_gap", 
            tcp_stat.tcp_reassembly_gap);
}
if (status == 4 )
{
    printf("\n\t ----------- Defrag ----------\n");
    printf("%30s : %"PRId64" \n", "defrag.ipv4.fragments", 
            defrag_stat.defrag_ipv4_fragments);
    printf("%30s : %"PRId64" \n", "defrag.ipv4.reassembled", 
            defrag_stat.defrag_ipv4_reassembled);
    printf("%30s : %"PRId64" \n", "defrag.ipv4.timeouts", 
            defrag_stat.defrag_ipv4_timeouts);
    printf("%30s : %"PRId64" \n", "defrag.ipv6.fragments", 
            defrag_stat.defrag_ipv6_fragments);
    printf("%30s : %"PRId64" \n", "defrag.ipv6.reassembled", 
            defrag_stat.defrag_ipv6_reassembled);
    printf("%30s : %"PRId64" \n", "defrag.ipv6.timeouts", 
            defrag_stat.defrag_ipv6_timeouts);
}
if (status == 5)
{
    printf("\n\t ----------- Detect ----------\n");
    printf("%30s : %"PRId64" \n", "detect.alert", detect_stat.detect_alert);
}
}

// API to printf interface statitics
void show_interface_stats(uint8_t intf)
{
    struct rte_eth_stats stats;

    if (intf > 4)
        intf = 4;

    uint16_t port = (intf == 0)?intf:(intf -1), max = (intf == 0)?4:(intf);

    for (; port < max; port++)
    {
        printf("\n\t --------------- INTF-%d ------------\n",port + 1);
        rte_eth_stats_get(port, &stats);
        //if(0 == rte_eth_stats_get(port, &stats))
        {
            printf("%25s : %"PRId64" \n", "Filter Match", stats.fdirmatch);
            printf("%25s : %"PRId64" \n", "Filter Miss", stats.fdirmiss);
            printf("%25s : %"PRId64" \n", "BAD CRC", stats.ibadcrc);
            printf("%25s : %"PRId64" \n", "BAD Len", stats.ibadlen);
            printf("%25s : %"PRId64" \n", "RX bytes", stats.ibytes);
            printf("%25s : %"PRId64" \n", "RX Err", stats.ierrors);
            printf("%25s : %"PRId64" \n", "LB RX bytes", stats.ilbbytes);
            printf("%25s : %"PRId64" \n", "LB pkts", stats.ilbpackets);
            printf("%25s : %"PRId64" \n", "MCAST", stats.imcasts);
            printf("%25s : %"PRId64" \n", "FIFO Full RX", stats.imissed);
            printf("%25s : %"PRId64" \n", "RX pkts", stats.ipackets);
            printf("%25s : %"PRId64" \n", "TX bytes", stats.obytes);
            printf("%25s : %"PRId64" \n", "TX err", stats.oerrors);
            printf("%25s : %"PRId64" \n", "LB TX bytes", stats.olbbytes);
            printf("%25s : %"PRId64" \n", "TX packets", stats.opackets);
            printf("%25s : %"PRId64" \n", "MBUF Fail", stats.rx_nombuf);
            printf("%25s : %"PRId64" \n", "RX PAUSE Xoff", stats.rx_pause_xoff);
            printf("%25s : %"PRId64" \n", "RX PAUSE xon", stats.rx_pause_xon);
            printf("%25s : %"PRId64" \n", "TX PAUSE Xoff", stats.tx_pause_xoff);
            printf("%25s : %"PRId64" \n", "TX PAUSE Xon", stats.tx_pause_xon);
        }
    }
}

// get rule info 
void get_info(void)
{
   printf("%30s :%d \n","TCP    -  rule cnt",tcp_ipv4_rule_cnt + tcp_ipv6_rule_cnt);
   printf("%30s :%d \n","UDP    -  rule cnt",udp_ipv4_rule_cnt + udp_ipv6_rule_cnt);
   printf("%30s :%d \n","ICMP   -  rule cnt",icmp_ipv4_rule_cnt + icmp_ipv6_rule_cnt);
   printf("%30s :%d \n","HTTP   -  rule cnt",http_ipv4_rule_cnt + http_ipv6_rule_cnt);
   printf("%30s :%d \n","SSL    -  rule cnt",ssl_ipv4_rule_cnt + icmp_ipv6_rule_cnt);
   printf("%30s :%d \n","TLS    -  rule cnt",tls_ipv4_rule_cnt + tls_ipv6_rule_cnt);
   printf("%30s :%d \n","GRE    -  rule cnt",gre_ipv4_rule_cnt + gre_ipv6_rule_cnt);
   printf("%30s :%d \n","GTP    -  rule cnt",gtp_ipv4_rule_cnt + gtp_ipv6_rule_cnt);
   printf("%30s :%d \n","FTP    -  rule cnt",ftp_ipv4_rule_cnt + ftp_ipv6_rule_cnt);
   printf("%30s :%d \n","SCTP   -  rule cnt",sctp_ipv4_rule_cnt + sctp_ipv6_rule_cnt);
   printf("%30s :%d \n","SMTP   -  rule cnt",smtp_ipv4_rule_cnt + smtp_ipv6_rule_cnt);
   printf("%30s :%d \n","DNS    -  rule cnt",dns_ipv4_rule_cnt + dns_ipv6_rule_cnt);
   printf("%30s :%d \n","SMP    -  rule cnt",smb_ipv4_rule_cnt + dns_ipv6_rule_cnt);
   printf("%30s :%d \n","DCERPC -  rule cnt",dcerpc_ipv4_rule_cnt + dcerpc_ipv6_rule_cnt);

  printf ("\n%s%25s", "", "--- IPv4 ACL Rule Count ---");
  printf ("\n%35s : %u", "IP", ruleCnt.ipv4);
  printf ("\n%35s : %u", "TCP", ruleCnt.tcp);
  printf ("\n%35s : %u", "UDP", ruleCnt.udp);
  printf ("\n%35s : %u", "ICMP", ruleCnt.icmp);
  printf ("\n%35s : %u", "SCTP", ruleCnt.sctp);
  printf ("\n%35s : %u", "GTP", ruleCnt.gtp);
  printf ("\n%35s : %u", "GRE", ruleCnt.gre);
  printf ("\n");
}


void get_intf_info(uint8_t num)
{
  struct rte_eth_dev_info dev_info;
  struct rte_eth_link link;
  uint16_t mtu = 0;

  if (num > 4)
     num = 4;

  uint16_t port = (num == 0)?num:(num -1), max = (num == 0)?4:(num);
  for (; port < max; port++)
  {
     rte_eth_dev_info_get (port, &dev_info);
     rte_eth_dev_get_mtu (port, &mtu);
     rte_eth_link_get(port, &link);

     printf("\n\t  --------- Intf-%d details ---------\n", port+1);
     printf("%25s : ", "PCI ");
     printf("%s:%u %s:%u; ", "vendor", dev_info.pci_dev->id.vendor_id, 
                                "device", dev_info.pci_dev->id.device_id);
     printf("%u:%u:%u:%u\n", dev_info.pci_dev->addr.domain,
                               dev_info.pci_dev->addr.bus,
                               dev_info.pci_dev->addr.devid,
                               dev_info.pci_dev->addr.function);
     printf("%25s : %s \n", "Driver ", dev_info.driver_name);
     printf("%25s : %u \n", "Buff-Size ", dev_info.min_rx_bufsize);
     printf("%25s : %u \n", "Max RX-Queue ", dev_info.max_rx_queues);
     printf("%25s : %u \n", "Max TX-Queue ", dev_info.max_tx_queues);
     printf("%25s : %u \n", "RX-Offload ", dev_info.rx_offload_capa);
     printf("%25s : %u \n", "TX-Offload ", dev_info.tx_offload_capa);
     printf("%25s : %u \n", "MTU ", mtu);
     printf("%25s : %s \n", "Link ", (link.link_status)?"Up":"Down");
     printf("%25s : %s \n", "Duplex", (link.link_duplex)?"FULL":"HALF");
     printf("%25s : %u \n", "Speed", link.link_speed);
  }
}

void get_rxfp_stats(void)
{
    uint8_t port =0;

    for (; port < 4; port++)
    {
    printf("\n\t\t  --------- RXFP for Intf-%d ---------\n", port + 1);
    printf("%25s : %"PRId64" \n", "rx_pkt",rxfp.rx_pkt);
    printf("%25s : %"PRId64" \n", "rx_byte",rxfp.rx_byte);
    printf("%25s : %"PRId64" \n", "tx_pkt",rxfp.tx_pkt);
    printf("%25s : %"PRId64" \n", "tx_byte",rxfp.tx_byte);
    printf("%25s : %"PRId64" \n", "arp", dp_metadata[port].dp_cnt.arp_cnt);
    printf("%25s : %"PRId64" \n", "vlan", dp_metadata[port].dp_cnt.vlan_cnt);
    printf("%25s : %"PRId64" \n", "ipv4", dp_metadata[port].dp_cnt.ipv4_cnt);
    printf("%25s : %"PRId64" \n", "ipv6", dp_metadata[port].dp_cnt.ipv6_cnt);
    printf("%25s : %"PRId64" \n", "tcp", dp_metadata[port].dp_cnt.tcp_cnt);
    printf("%25s : %"PRId64" \n", "udp", dp_metadata[port].dp_cnt.udp_cnt);
    printf("%25s : %"PRId64" \n", "icmp", dp_metadata[port].dp_cnt.icmp_cnt);
    printf("%25s : %"PRId64" \n", "ssl",rxfp.ssl);
    printf("%25s : %"PRId64" \n", "tls",rxfp.tls);
    printf("%25s : %"PRId64" \n", "gtp",rxfp.gtp);
    printf("%25s : %"PRId64" \n", "smb",rxfp.smp);
    printf("%25s : %"PRId64" \n", "ftp",rxfp.ftp);
    printf("%25s : %"PRId64" \n", "smtp",rxfp.smtp);
    printf("%25s : %"PRId64" \n", "sctp",rxfp.sctp);
    printf("%25s : %"PRId64" \n", "dns",rxfp.dns);
    printf("%25s : %"PRId64" \n", "http", dp_metadata[port].dp_cnt.http_cnt);
    printf("%25s : %"PRId64" \n", "gre",rxfp.gre);
    printf("%25s : %"PRId64" \n", "dcerpc",rxfp.dcerpc);
    }

    printf("\n\t\t  --------- RXFP ---------\n");
    printf("%25s : %"PRId64" \n", "port map drop ", rxfpCount.pmap_drop);
    printf("%25s : %"PRId64" \n", "port act drop ", rxfpCount.pact_drop);
    printf("%25s : %"PRId64" \n", "no acl lkp ", rxfpCount.acl_nLkp);
    printf("%25s : %"PRId64" \n", "trg acl lkp ", rxfpCount.acl_Lkp);
    printf("%25s : %"PRId64" \n", "hit acl ", rxfpCount.acl_hit);
    printf("%25s : %"PRId64" \n", "miss acl ", rxfpCount.acl_miss);
    printf("%25s : %"PRId64" \n", "st fwd ", rxfpCount.st_fwd);
    printf("%25s : %"PRId64" \n", "st err ", rxfpCount.st_err);
    printf("%25s : %"PRId64" \n", "frag IPv4 Drop ", rxfpCount.f4drp);
    printf("%25s : %"PRId64" \n", "Port FWD ", rxfpCount.pfwd);
    printf("%25s : %"PRId64" \n", "Port Drop ", rxfpCount.pdrop);
}

void get_txfw_stats(void)
{
    printf("\n\t\t  --------- TXFW ---------\n");
    printf("%30s : %"PRId64" \n", "tx_pkt",txfw.tx_pkt);
    printf("%30s : %"PRId64" \n", "tx_byte",txfw.tx_byte);
    printf("%30s : %"PRId64" \n", "failed",txfw.failed);
    printf("%30s : %"PRId64" \n", "Err",txfw.Err);
}

void get_st1_stats(void)
{
    printf("\n\t\t  --------- ST1 ---------\n");
    printf("%30s : %"PRId64" \n", "rx_pkt",st1.rx_pkt);
    printf("%30s : %"PRId64" \n", "rx_processed",st1.rx_processed);
    printf("%30s : %"PRId64" \n", "acl_lookup",st1.acl_lookup);
    printf("%30s : %"PRId64" \n", "acl_match",st1.acl_match);
    printf("%30s : %"PRId64" \n", "acl_nomatch",st1.acl_nomatch);
    printf("%30s : %"PRId64" \n", "failed",st1.failed);
    printf("%30s : %"PRId64" \n", "tx_pkts",st1.tx_pkts);
}

void get_st2_stats(void)
{
    printf("\n\t\t  --------- ST2 ---------\n");
    printf("%30s : %"PRId64" \n", "rx_pkt",st2.rx_pkt);
    printf("%30s : %"PRId64" \n", "rx_processed",st2.rx_processed);
    printf("%30s : %"PRId64" \n", "acl_lookup",st2.acl_lookup);
    printf("%30s : %"PRId64" \n", "acl_match",st2.acl_match);
    printf("%30s : %"PRId64" \n", "acl_nomatch",st2.acl_nomatch);
    printf("%30s : %"PRId64" \n", "failed",st2.failed);
    printf("%30s : %"PRId64" \n", "tx_pkts",st2.tx_pkts);
}
void show_core_info(char *mode)
{
    printf("%30s : %s \n","Core 0"," OAM ");
    printf("%30s : %s \n","Core 1"," Distributor,TX");
    printf("%30s : %s \n","Core 2"," ST1,TX ");
    printf("%30s : %s \n","Core 3"," ST2,TX ");
}
void show_pmap(void)
{
    printf("%31s -> %s","INGRESS","EGRESS");
    printf("  (0 - port1 | 1 -port2 | 2- port3 | 3 - port4 |255 - drop packets)\n");
    if (pmap_p0 == 255)
    {
        printf("%30s  -> %d \n","0",pmap_p0);
    }
    else if ((pmap_p0 >= 0) && (pmap_p0 <= 3))
    {
        printf("%30s  -> %d \n","0", pmap_p0);
    }
    else 
        printf("%30s  -> %s\n","0","Not Mapped");
    
    if (pmap_p1 == 255)
    {
        printf("%30s  -> %d \n","1",pmap_p1);
    }
    else if ((pmap_p1 >=0) && (pmap_p1 <=3))
    {
        printf("%30s  -> %d \n","1",pmap_p1);
    }
    else 
        printf("%30s  -> %s\n","1","Not Mapped");

    if (pmap_p2 == 255)
    {
        printf("%30s  -> %d \n","2",pmap_p2);
    }
    else if ((pmap_p2 >= 0) && (pmap_p2 <=3))
    {
        printf("%30s  -> %d \n","2",pmap_p2);
    }
    else 
        printf("%30s  -> %s\n","2","Not Mapped");
    
    if (pmap_p3 == 255)
    {
        printf("%30s  -> %d \n","3",pmap_p3);
    }
    else if (pmap_p3 == 1)
    {
        printf("%30s  -> %d \n","3",pmap_p3);
    }
    else 
        printf("%30s  -> %s\n","3","Not Mapped");
}
uint8_t del_ipv6_rule(char *proto, uint32_t rulenum)
{
   uint32_t cnt;
   if(!strcmp(proto, "tcp"))
   {
      if (tcp_ipv6_rule_cnt == 0)
      {
         printf("Rule array is empty\n");
         return 0;  
      }
      else if(rulenum > tcp_ipv6_rule_cnt)
      {
         printf("Rule No doesn't exist\n");
      }
      else
      {
         for (cnt = rulenum - 1; cnt < tcp_ipv6_rule_cnt - 1; cnt++)
         {
            strcpy(tcp_ipv6_result[cnt].action, tcp_ipv6_result[cnt + 1].action);
            strcpy(tcp_ipv6_rule[cnt].flow, tcp_ipv6_rule[cnt + 1].flow);
            tcp_ipv6_rule[cnt].sipaddr = tcp_ipv6_rule[cnt + 1].sipaddr;
            tcp_ipv6_rule[cnt].sport = tcp_ipv6_rule[cnt + 1].sport;
            tcp_ipv6_rule[cnt].dipaddr = tcp_ipv6_rule[cnt + 1].dipaddr;
            tcp_ipv6_rule[cnt].dport = tcp_ipv6_rule[cnt + 1].dport;
         }
          tcp_ipv6_rule_cnt--;
       }
   }
   else if (!strcmp(proto, "udp"))
   {
      if (udp_ipv6_rule_cnt == 0)
      {
         printf("Rule array is empty\n");
         return 0;  
      }
      else if(rulenum > udp_ipv6_rule_cnt)
      {
         printf("Rule No doesn't exist\n");
      }
      else
      {
          for (cnt = rulenum - 1; cnt <= udp_ipv6_rule_cnt - 1; cnt++)
          {
             strcpy(udp_ipv6_result[cnt].action, udp_ipv6_result[cnt + 1].action);
             strcpy(udp_ipv6_rule[cnt].flow, udp_ipv6_rule[cnt + 1].flow);
             udp_ipv6_rule[cnt].sipaddr = udp_ipv6_rule[cnt + 1].sipaddr;
             udp_ipv6_rule[cnt].sport = udp_ipv6_rule[cnt + 1].sport;
             udp_ipv6_rule[cnt].dipaddr = udp_ipv6_rule[cnt + 1].dipaddr;
             udp_ipv6_rule[cnt].dport = udp_ipv6_rule[cnt + 1].dport;
          }
          udp_ipv6_rule_cnt--;
       }
   }
   else if (!strcmp(proto, "tls"))
   {
      if (tls_ipv6_rule_cnt == 0)
      {
         printf("Rule array is empty\n");
         return 0;  
      }
      else if(rulenum > tls_ipv6_rule_cnt)
      {
         printf("Rule No doesn't exist\n");
      }
      else
      {
          for (cnt = rulenum - 1; cnt < tls_ipv6_rule_cnt - 1; cnt++)
          {
             strcpy(tls_ipv6_result[cnt].action, tls_ipv6_result[cnt + 1].action);
             strcpy(tls_ipv6_rule[cnt].flow, tls_ipv6_rule[cnt + 1].flow);
             tls_ipv6_rule[cnt].sipaddr = tls_ipv6_rule[cnt + 1].sipaddr;
             tls_ipv6_rule[cnt].sport = tls_ipv6_rule[cnt + 1].sport;
             tls_ipv6_rule[cnt].dipaddr = tls_ipv6_rule[cnt + 1].dipaddr;
             tls_ipv6_rule[cnt].dport = tls_ipv6_rule[cnt + 1].dport;
          }
          tls_ipv6_rule_cnt--;
       }
   }
   else if (!strcmp(proto, "ssl"))
   {
      if  (ssl_ipv6_rule_cnt == 0)
      {
         printf("Rule array is empty\n");
         return 0;  
      }
      else if(rulenum > ssl_ipv6_rule_cnt)
      {
         printf("Rule No doesn't exist\n");
      }
      else
      {
          for (cnt = rulenum - 1; cnt < ssl_ipv6_rule_cnt - 1; cnt++)
          {
             strcpy(ssl_ipv6_result[cnt].action, ssl_ipv6_result[cnt + 1].action);
             strcpy(ssl_ipv6_rule[cnt].flow, ssl_ipv6_rule[cnt + 1].flow);
             ssl_ipv6_rule[cnt].sipaddr = ssl_ipv6_rule[cnt + 1].sipaddr;
             ssl_ipv6_rule[cnt].sport = ssl_ipv6_rule[cnt + 1].sport;
             ssl_ipv6_rule[cnt].dipaddr = ssl_ipv6_rule[cnt + 1].dipaddr;
             ssl_ipv6_rule[cnt].dport = ssl_ipv6_rule[cnt + 1].dport;
          }
          ssl_ipv6_rule_cnt--;
       }
   }
   else if (!strcmp(proto, "ftp"))
   {
      if (ftp_ipv6_rule_cnt == 0)
      {
         printf("Rule array is empty\n");
         return 0;  
      }
      else if(rulenum > ftp_ipv6_rule_cnt)
      {
         printf("Rule No doesn't exist\n");
      }
      else
      {
          for(cnt = rulenum - 1; cnt < ftp_ipv6_rule_cnt - 1; cnt++)
          {
             strcpy(ftp_ipv6_result[cnt].action, ftp_ipv6_result[cnt + 1].action);
             strcpy(ftp_ipv6_rule[cnt].flow, ftp_ipv6_rule[cnt + 1].flow);
             ftp_ipv6_rule[cnt].sipaddr = ftp_ipv6_rule[cnt + 1].sipaddr;
             ftp_ipv6_rule[cnt].sport = ftp_ipv6_rule[cnt + 1].sport;
             ftp_ipv6_rule[cnt].dipaddr = ftp_ipv6_rule[cnt + 1].dipaddr;
             ftp_ipv6_rule[cnt].dport = ftp_ipv6_rule[cnt + 1].dport;
          }
          ftp_ipv6_rule_cnt--;
       }
   }
   else if (!strcmp(proto, "icmp"))
   {
      if (icmp_ipv6_rule_cnt == 0)
      {
         printf("Rule array is empty\n");
         return 0;  
      }
      else if(rulenum > icmp_ipv6_rule_cnt)
      {
         printf("Rule No doesn't exist\n");
      }
      else
      {
          for (cnt = rulenum - 1; cnt < icmp_ipv6_rule_cnt - 1; cnt++)
          {
             strcpy(icmp_ipv6_result[cnt].action, icmp_ipv6_result[cnt + 1].action);
             strcpy(icmp_ipv6_rule[cnt].flow, icmp_ipv6_rule[cnt + 1].flow);
             icmp_ipv6_rule[cnt].sipaddr = icmp_ipv6_rule[cnt + 1].sipaddr;
             icmp_ipv6_rule[cnt].sport = icmp_ipv6_rule[cnt + 1].sport;
             icmp_ipv6_rule[cnt].dipaddr = icmp_ipv6_rule[cnt + 1].dipaddr;
             icmp_ipv6_rule[cnt].dport = icmp_ipv6_rule[cnt + 1].dport;
          }
          icmp_ipv6_rule_cnt--;
       }
   }
   else if (!strcmp(proto, "http"))
   {
      if (http_ipv6_rule_cnt == 0)
      {
         printf("Rule array is empty\n");
         return 0;  
      }
      else if(rulenum > http_ipv6_rule_cnt)
      {
         printf("Rule No doesn't exist\n");
      }
      else
      {
          for (cnt = rulenum - 1; cnt < http_ipv6_rule_cnt - 1; cnt++)
          {
             strcpy(http_ipv6_result[cnt].action, http_ipv6_result[cnt + 1].action);
             strcpy(http_ipv6_rule[cnt].flow, http_ipv6_rule[cnt + 1].flow);
             http_ipv6_rule[cnt].sipaddr = http_ipv6_rule[cnt + 1].sipaddr;
             http_ipv6_rule[cnt].sport = http_ipv6_rule[cnt + 1].sport;
             http_ipv6_rule[cnt].dipaddr = http_ipv6_rule[cnt + 1].dipaddr;
             http_ipv6_rule[cnt].dport = http_ipv6_rule[cnt + 1].dport;
          }
          http_ipv6_rule_cnt--;
       }
   }
   else if (!strcmp(proto, "gre"))
   {
      if(gre_ipv6_rule_cnt == 0)
      {
         printf("Rule array is empty\n");
         return 0;  
      }
      else if(rulenum > gre_ipv6_rule_cnt)
      {
         printf("Rule No doesn't exist\n");
      }
      else
      {
          for (cnt = rulenum - 1; cnt < gre_ipv6_rule_cnt - 1; cnt++)
          {
             strcpy(gre_ipv6_result[cnt].action, gre_ipv6_result[cnt + 1].action);
             strcpy(gre_ipv6_rule[cnt].flow, gre_ipv6_rule[cnt + 1].flow);
             gre_ipv6_rule[cnt].sipaddr = gre_ipv6_rule[cnt + 1].sipaddr;
             gre_ipv6_rule[cnt].sport = gre_ipv6_rule[cnt + 1].sport;
             gre_ipv6_rule[cnt].dipaddr = gre_ipv6_rule[cnt + 1].dipaddr;
             gre_ipv6_rule[cnt].dport = gre_ipv6_rule[cnt + 1].dport;
          }
          gre_ipv6_rule_cnt--;
       }
   }
   else if(!strcmp(proto, "gtp"))
   {
      if(gtp_ipv6_rule_cnt == 0)
      {
         printf("Rule array is empty\n");
         return 0;  
      }
      else if(rulenum > gtp_ipv6_rule_cnt)
      {
         printf("Rule No doesn't exist\n");
      }
      else
      {
          for (cnt = rulenum - 1; cnt < gtp_ipv6_rule_cnt - 1; cnt++)
          {
             strcpy(gtp_ipv6_result[cnt].action, gtp_ipv6_result[cnt + 1].action);
             strcpy(gtp_ipv6_rule[cnt].flow, gtp_ipv6_rule[cnt + 1].flow);
             gtp_ipv6_rule[cnt].sipaddr = gtp_ipv6_rule[cnt + 1].sipaddr;
             gtp_ipv6_rule[cnt].sport = gtp_ipv6_rule[cnt + 1].sport;
             gtp_ipv6_rule[cnt].dipaddr = gtp_ipv6_rule[cnt + 1].dipaddr;
             gtp_ipv6_rule[cnt].dport = gtp_ipv6_rule[cnt + 1].dport;
          }
          gtp_ipv6_rule_cnt--;
       }
   }
   else if(!strcmp(proto, "sctp"))
   {
      if(sctp_ipv6_rule_cnt == 0)
      {
         printf("Rule array is empty\n");
         return 0;  
      }
      else if(rulenum >sctp_ipv6_rule_cnt)
      {
         printf("Rule No doesn't exist\n");
      }
      else
      {
          for(cnt = rulenum - 1; cnt < sctp_ipv6_rule_cnt - 1; cnt++)
          {
             strcpy(sctp_ipv6_result[cnt].action, sctp_ipv6_result[cnt + 1].action);
             strcpy(sctp_ipv6_rule[cnt].flow, sctp_ipv6_rule[cnt + 1].flow);
             sctp_ipv6_rule[cnt].sipaddr = sctp_ipv6_rule[cnt + 1].sipaddr;
             sctp_ipv6_rule[cnt].sport = sctp_ipv6_rule[cnt + 1].sport;
             sctp_ipv6_rule[cnt].dipaddr = sctp_ipv6_rule[cnt + 1].dipaddr;
             sctp_ipv6_rule[cnt].dport = sctp_ipv6_rule[cnt + 1].dport;
          }
          sctp_ipv6_rule_cnt--;
       }
   }
   else if(!strcmp(proto, "smtp"))
   {
      if(smtp_ipv6_rule_cnt == 0)
      {
         printf("Rule array is empty\n");
         return 0;  
      }
      else if(rulenum > smtp_ipv6_rule_cnt)
      {
         printf("Rule No doesn't exist\n");
      }
      else
      {
          for(cnt = rulenum - 1; cnt < smtp_ipv6_rule_cnt - 1; cnt++)
          {
             strcpy(smtp_ipv6_result[cnt].action, smtp_ipv6_result[cnt + 1].action);
             strcpy(smtp_ipv6_rule[cnt].flow, smtp_ipv6_rule[cnt + 1].flow);
             smtp_ipv6_rule[cnt].sipaddr = smtp_ipv6_rule[cnt + 1].sipaddr;
             smtp_ipv6_rule[cnt].sport = smtp_ipv6_rule[cnt + 1].sport;
             smtp_ipv6_rule[cnt].dipaddr = smtp_ipv6_rule[cnt + 1].dipaddr;
             smtp_ipv6_rule[cnt].dport = smtp_ipv6_rule[cnt + 1].dport;
          }
          smtp_ipv6_rule_cnt--;
       }
   }
   else if(!strcmp(proto, "dns"))
   {
      if(dns_ipv6_rule_cnt == 0)
      {
         printf("Rule array is empty\n");
         return 0;  
      }
      else if(rulenum > dns_ipv6_rule_cnt)
      {
         printf("Rule No doesn't exist\n");
      }
      else
      {
          for(cnt = rulenum - 1; cnt < dns_ipv6_rule_cnt - 1; cnt++)
          {
             strcpy(dns_ipv6_result[cnt].action, dns_ipv6_result[cnt + 1].action);
             strcpy(dns_ipv6_rule[cnt].flow, dns_ipv6_rule[cnt + 1].flow);
             dns_ipv6_rule[cnt].sipaddr = dns_ipv6_rule[cnt + 1].sipaddr;
             dns_ipv6_rule[cnt].sport = dns_ipv6_rule[cnt + 1].sport;
             dns_ipv6_rule[cnt].dipaddr = dns_ipv6_rule[cnt + 1].dipaddr;
             dns_ipv6_rule[cnt].dport = dns_ipv6_rule[cnt + 1].dport;
          }
          dns_ipv6_rule_cnt--;
       }
   }
   else if(!strcmp(proto, "smb"))
   {
      if(smb_ipv6_rule_cnt == 0)
      {
         printf("Rule array is empty\n");
         return 0;  
      }
      else if(rulenum > smb_ipv6_rule_cnt)
      {
         printf("Rule No doesn't exist\n");
      }
      else
      {
          for(cnt = rulenum - 1; cnt < smb_ipv6_rule_cnt - 1; cnt++)
          {
             strcpy(smb_ipv6_result[cnt].action, smb_ipv6_result[cnt + 1].action);
             strcpy(smb_ipv6_rule[cnt].flow, smb_ipv6_rule[cnt + 1].flow);
             smb_ipv6_rule[cnt].sipaddr = smb_ipv6_rule[cnt + 1].sipaddr;
             smb_ipv6_rule[cnt].sport = smb_ipv6_rule[cnt + 1].sport;
             smb_ipv6_rule[cnt].dipaddr = smb_ipv6_rule[cnt + 1].dipaddr;
             smb_ipv6_rule[cnt].dport = smb_ipv6_rule[cnt + 1].dport;
          }
          smb_ipv6_rule_cnt--;
       }
   }
   else if(!strcmp(proto, "smb2"))
   {
      if(smb_ipv6_rule_cnt == 0)
      {
         printf("Rule array is empty\n");
         return 0;  
      }
      else if(rulenum > smb2_ipv6_rule_cnt)
      {
         printf("Rule No doesn't exist\n");
      }
      else
      {
          for(cnt = rulenum - 1; cnt < smb2_ipv6_rule_cnt - 1; cnt++)
          {
             strcpy(smb2_ipv6_result[cnt].action, smb2_ipv6_result[cnt + 1].action);
             strcpy(smb2_ipv6_rule[cnt].flow, smb2_ipv6_rule[cnt + 1].flow);
             smb2_ipv6_rule[cnt].sipaddr = smb2_ipv6_rule[cnt + 1].sipaddr;
             smb2_ipv6_rule[cnt].sport = smb2_ipv6_rule[cnt + 1].sport;
             smb2_ipv6_rule[cnt].dipaddr = smb2_ipv6_rule[cnt + 1].dipaddr;
             smb2_ipv6_rule[cnt].dport = smb2_ipv6_rule[cnt + 1].dport;
          }
          smb2_ipv6_rule_cnt--;
       }
   }
   else if(!strcmp(proto, "dcerpc"))
   {
      if(dcerpc_ipv6_rule_cnt == 0)
      {
         printf("Rule array is empty\n");
         return 0;  
      }
      else if(rulenum > dcerpc_ipv6_rule_cnt)
      {
         printf("Rule No doesn't exist\n");
      }
      else
      {
          for(cnt = rulenum - 1; cnt < dcerpc_ipv6_rule_cnt - 1; cnt++)
          {
             strcpy(dcerpc_ipv6_result[cnt].action, dcerpc_ipv6_result[cnt + 1].action);
             strcpy(dcerpc_ipv6_rule[cnt].flow, dcerpc_ipv6_rule[cnt + 1].flow);
             dcerpc_ipv6_rule[cnt].sport = dcerpc_ipv6_rule[cnt + 1].sport;
             dcerpc_ipv6_rule[cnt].dipaddr = dcerpc_ipv6_rule[cnt + 1].dipaddr;
             dcerpc_ipv6_rule[cnt].dport = dcerpc_ipv6_rule[cnt + 1].dport;
          }
          dcerpc_ipv6_rule_cnt--;
       }
   }
   return 1;
}

uint8_t del_ipv4_rule(char *proto, uint32_t rulenum)
{
   uint32_t cnt;
   if(!strcmp(proto, "tcp"))
   {
      if (tcp_ipv4_rule_cnt == 0)
      {
         printf("Rule array is empty\n");
         return 0;  
      }
      for (cnt = rulenum - 1; cnt < tcp_ipv4_rule_cnt - 1; cnt++)
      {
         strcpy(tcp_ipv4_result[cnt].action, tcp_ipv4_result[cnt + 1].action);
         strcpy(tcp_ipv4_rule[cnt].flow, tcp_ipv4_rule[cnt + 1].flow);
         tcp_ipv4_rule[cnt].sipaddr = tcp_ipv4_rule[cnt + 1].sipaddr;
         tcp_ipv4_rule[cnt].sport = tcp_ipv4_rule[cnt + 1].sport;
         tcp_ipv4_rule[cnt].dipaddr = tcp_ipv4_rule[cnt + 1].dipaddr;
         tcp_ipv4_rule[cnt].dport = tcp_ipv4_rule[cnt + 1].dport;
      }
      tcp_ipv4_rule_cnt--;
   }
   else if (!strcmp(proto, "udp"))
   {
      if (udp_ipv4_rule_cnt == 0)
      {
         printf("Rule array is empty\n");
         return 0;  
      }
      for (cnt = rulenum - 1; cnt <= udp_ipv4_rule_cnt - 1; cnt++)
      {
         strcpy(udp_ipv4_result[cnt].action, udp_ipv4_result[cnt + 1].action);
         strcpy(udp_ipv4_rule[cnt].flow, udp_ipv4_rule[cnt + 1].flow);
         udp_ipv4_rule[cnt].sipaddr = udp_ipv4_rule[cnt + 1].sipaddr;
         udp_ipv4_rule[cnt].sport = udp_ipv4_rule[cnt + 1].sport;
         udp_ipv4_rule[cnt].dipaddr = udp_ipv4_rule[cnt + 1].dipaddr;
         udp_ipv4_rule[cnt].dport = udp_ipv4_rule[cnt + 1].dport;
      }
      udp_ipv4_rule_cnt--;
   }
   else if (!strcmp(proto, "tls"))
   {
      if (tls_ipv4_rule_cnt == 0)
      {
         printf("Rule array is empty\n");
         return 0;  
      }
      for (cnt = rulenum - 1; cnt < tls_ipv4_rule_cnt - 1; cnt++)
      {
         strcpy(tls_ipv4_result[cnt].action, tls_ipv4_result[cnt + 1].action);
         strcpy(tls_ipv4_rule[cnt].flow, tls_ipv4_rule[cnt + 1].flow);
         tls_ipv4_rule[cnt].sipaddr = tls_ipv4_rule[cnt + 1].sipaddr;
         tls_ipv4_rule[cnt].sport = tls_ipv4_rule[cnt + 1].sport;
         tls_ipv4_rule[cnt].dipaddr = tls_ipv4_rule[cnt + 1].dipaddr;
         tls_ipv4_rule[cnt].dport = tls_ipv4_rule[cnt + 1].dport;
      }
      tls_ipv4_rule_cnt--;
   }
   else if (!strcmp(proto, "ssl"))
   {
      if  (ssl_ipv4_rule_cnt == 0)
      {
         printf("Rule array is empty\n");
         return 0;  
      }
      for (cnt = rulenum - 1; cnt < ssl_ipv4_rule_cnt - 1; cnt++)
      {
         strcpy(ssl_ipv4_result[cnt].action, ssl_ipv4_result[cnt + 1].action);
         strcpy(ssl_ipv4_rule[cnt].flow, ssl_ipv4_rule[cnt + 1].flow);
         ssl_ipv4_rule[cnt].sipaddr = ssl_ipv4_rule[cnt + 1].sipaddr;
         ssl_ipv4_rule[cnt].sport = ssl_ipv4_rule[cnt + 1].sport;
         ssl_ipv4_rule[cnt].dipaddr = ssl_ipv4_rule[cnt + 1].dipaddr;
         ssl_ipv4_rule[cnt].dport = ssl_ipv4_rule[cnt + 1].dport;
      }
      ssl_ipv4_rule_cnt--;
   }
   else if (!strcmp(proto, "ftp"))
   {
      if (ftp_ipv4_rule_cnt == 0)
      {
         printf("Rule array is empty\n");
         return 0;  
      }
      for(cnt = rulenum - 1; cnt < ftp_ipv4_rule_cnt - 1; cnt++)
      {
         strcpy(ftp_ipv4_result[cnt].action, ftp_ipv4_result[cnt + 1].action);
         strcpy(ftp_ipv4_rule[cnt].flow, ftp_ipv4_rule[cnt + 1].flow);
         ftp_ipv4_rule[cnt].sipaddr = ftp_ipv4_rule[cnt + 1].sipaddr;
         ftp_ipv4_rule[cnt].sport = ftp_ipv4_rule[cnt + 1].sport;
         ftp_ipv4_rule[cnt].dipaddr = ftp_ipv4_rule[cnt + 1].dipaddr;
         ftp_ipv4_rule[cnt].dport = ftp_ipv4_rule[cnt + 1].dport;
      }
      ftp_ipv4_rule_cnt--;
   }
   else if (!strcmp(proto, "icmp"))
   {
      if (icmp_ipv4_rule_cnt == 0)
      {
         printf("Rule array is empty\n");
         return 0;  
      }
      for (cnt = rulenum - 1; cnt < icmp_ipv4_rule_cnt - 1; cnt++)
      {
         strcpy(icmp_ipv4_result[cnt].action, icmp_ipv4_result[cnt + 1].action);
         strcpy(icmp_ipv4_rule[cnt].flow, icmp_ipv4_rule[cnt + 1].flow);
         icmp_ipv4_rule[cnt].sipaddr = icmp_ipv4_rule[cnt + 1].sipaddr;
         icmp_ipv4_rule[cnt].sport = icmp_ipv4_rule[cnt + 1].sport;
         icmp_ipv4_rule[cnt].dipaddr = icmp_ipv4_rule[cnt + 1].dipaddr;
         icmp_ipv4_rule[cnt].dport = icmp_ipv4_rule[cnt + 1].dport;
      }
      icmp_ipv4_rule_cnt--;
   }
   else if (!strcmp(proto, "http"))
   {
      if (http_ipv4_rule_cnt == 0)
      {
         printf("Rule array is empty\n");
         return 0;  
      }
      for (cnt = rulenum - 1; cnt < http_ipv4_rule_cnt - 1; cnt++)
      {
         strcpy(http_ipv4_result[cnt].action, http_ipv4_result[cnt + 1].action);
         strcpy(http_ipv4_rule[cnt].flow, http_ipv4_rule[cnt + 1].flow);
         http_ipv4_rule[cnt].sipaddr = http_ipv4_rule[cnt + 1].sipaddr;
         http_ipv4_rule[cnt].sport = http_ipv4_rule[cnt + 1].sport;
         http_ipv4_rule[cnt].dipaddr = http_ipv4_rule[cnt + 1].dipaddr;
         http_ipv4_rule[cnt].dport = http_ipv4_rule[cnt + 1].dport;
      }
      http_ipv4_rule_cnt--;
   }
   else if (!strcmp(proto, "gre"))
   {
      if(gre_ipv4_rule_cnt == 0)
      {
         printf("Rule array is empty\n");
         return 0;  
      }
      for (cnt = rulenum - 1; cnt < gre_ipv4_rule_cnt - 1; cnt++)
      {
         strcpy(gre_ipv4_result[cnt].action, gre_ipv4_result[cnt + 1].action);
         strcpy(gre_ipv4_rule[cnt].flow, gre_ipv4_rule[cnt + 1].flow);
         gre_ipv4_rule[cnt].sipaddr = gre_ipv4_rule[cnt + 1].sipaddr;
         gre_ipv4_rule[cnt].sport = gre_ipv4_rule[cnt + 1].sport;
         gre_ipv4_rule[cnt].dipaddr = gre_ipv4_rule[cnt + 1].dipaddr;
         gre_ipv4_rule[cnt].dport = gre_ipv4_rule[cnt + 1].dport;
      }
      gre_ipv4_rule_cnt--;
   }
   else if(!strcmp(proto, "gtp"))
   {
      if(gtp_ipv4_rule_cnt == 0)
      {
         printf("Rule array is empty\n");
         return 0;  
      }
      for (cnt = rulenum - 1; cnt < gtp_ipv4_rule_cnt - 1; cnt++)
      {
         strcpy(gtp_ipv4_result[cnt].action, gtp_ipv4_result[cnt + 1].action);
         strcpy(gtp_ipv4_rule[cnt].flow, gtp_ipv4_rule[cnt + 1].flow);
         gtp_ipv4_rule[cnt].sipaddr = gtp_ipv4_rule[cnt + 1].sipaddr;
         gtp_ipv4_rule[cnt].sport = gtp_ipv4_rule[cnt + 1].sport;
         gtp_ipv4_rule[cnt].dipaddr = gtp_ipv4_rule[cnt + 1].dipaddr;
         gtp_ipv4_rule[cnt].dport = gtp_ipv4_rule[cnt + 1].dport;
      }
      gtp_ipv4_rule_cnt--;
   }
   else if(!strcmp(proto, "sctp"))
   {
      if(sctp_ipv4_rule_cnt == 0)
      {
         printf("Rule array is empty\n");
         return 0;  
      }
      for(cnt = rulenum - 1; cnt < sctp_ipv4_rule_cnt - 1; cnt++)
      {
         strcpy(sctp_ipv4_result[cnt].action, sctp_ipv4_result[cnt + 1].action);
         strcpy(sctp_ipv4_rule[cnt].flow, sctp_ipv4_rule[cnt + 1].flow);
         sctp_ipv4_rule[cnt].sipaddr = sctp_ipv4_rule[cnt + 1].sipaddr;
         sctp_ipv4_rule[cnt].sport = sctp_ipv4_rule[cnt + 1].sport;
         sctp_ipv4_rule[cnt].dipaddr = sctp_ipv4_rule[cnt + 1].dipaddr;
         sctp_ipv4_rule[cnt].dport = sctp_ipv4_rule[cnt + 1].dport;
      }
      sctp_ipv4_rule_cnt--;
   }
   else if(!strcmp(proto, "smtp"))
   {
      if(smtp_ipv4_rule_cnt == 0)
      {
         printf("Rule array is empty\n");
         return 0;  
      }
      for(cnt = rulenum - 1; cnt < smtp_ipv4_rule_cnt - 1; cnt++)
      {
         strcpy(smtp_ipv4_result[cnt].action, smtp_ipv4_result[cnt + 1].action);
         strcpy(smtp_ipv4_rule[cnt].flow, smtp_ipv4_rule[cnt + 1].flow);
         smtp_ipv4_rule[cnt].sipaddr = smtp_ipv4_rule[cnt + 1].sipaddr;
         smtp_ipv4_rule[cnt].sport = smtp_ipv4_rule[cnt + 1].sport;
         smtp_ipv4_rule[cnt].dipaddr = smtp_ipv4_rule[cnt + 1].dipaddr;
         smtp_ipv4_rule[cnt].dport = smtp_ipv4_rule[cnt + 1].dport;
      }
      smtp_ipv4_rule_cnt--;
   }
   else if(!strcmp(proto, "dns"))
   {
      if(dns_ipv4_rule_cnt == 0)
      {
         printf("Rule array is empty\n");
         return 0;  
      }
      for(cnt = rulenum - 1; cnt < dns_ipv4_rule_cnt - 1; cnt++)
      {
         strcpy(dns_ipv4_result[cnt].action, dns_ipv4_result[cnt + 1].action);
         strcpy(dns_ipv4_rule[cnt].flow, dns_ipv4_rule[cnt + 1].flow);
         dns_ipv4_rule[cnt].sipaddr = dns_ipv4_rule[cnt + 1].sipaddr;
         dns_ipv4_rule[cnt].sport = dns_ipv4_rule[cnt + 1].sport;
         dns_ipv4_rule[cnt].dipaddr = dns_ipv4_rule[cnt + 1].dipaddr;
         dns_ipv4_rule[cnt].dport = dns_ipv4_rule[cnt + 1].dport;
      }
      dns_ipv4_rule_cnt--;
   }
   else if(!strcmp(proto, "smb"))
   {
      if(smb_ipv4_rule_cnt == 0)
      {
         printf("Rule array is empty\n");
         return 0;  
      }
      for(cnt = rulenum - 1; cnt < smb_ipv4_rule_cnt - 1; cnt++)
      {
         strcpy(smb_ipv4_result[cnt].action, smb_ipv4_result[cnt + 1].action);
         strcpy(smb_ipv4_rule[cnt].flow, smb_ipv4_rule[cnt + 1].flow);
         smb_ipv4_rule[cnt].sipaddr = smb_ipv4_rule[cnt + 1].sipaddr;
         smb_ipv4_rule[cnt].sport = smb_ipv4_rule[cnt + 1].sport;
         smb_ipv4_rule[cnt].dipaddr = smb_ipv4_rule[cnt + 1].dipaddr;
         smb_ipv4_rule[cnt].dport = smb_ipv4_rule[cnt + 1].dport;
      }
      smb_ipv4_rule_cnt--;
   }
   else if(!strcmp(proto, "smb"))
   {
      if(smb_ipv4_rule_cnt == 0)
      {
         printf("Rule array is empty\n");
         return 0;  
      }
      for(cnt = rulenum - 1; cnt < smb2_ipv4_rule_cnt - 1; cnt++)
      {
         strcpy(smb2_ipv4_result[cnt].action, smb2_ipv4_result[cnt + 1].action);
         strcpy(smb2_ipv4_rule[cnt].flow, smb2_ipv4_rule[cnt + 1].flow);
         smb2_ipv4_rule[cnt].sipaddr = smb2_ipv4_rule[cnt + 1].sipaddr;
         smb2_ipv4_rule[cnt].sport = smb2_ipv4_rule[cnt + 1].sport;
         smb2_ipv4_rule[cnt].dipaddr = smb2_ipv4_rule[cnt + 1].dipaddr;
         smb2_ipv4_rule[cnt].dport = smb2_ipv4_rule[cnt + 1].dport;
      }
      smb2_ipv4_rule_cnt--;
   }
   
    else if(!strcmp(proto, "dcerpc"))
   {
      if(dcerpc_ipv4_rule_cnt == 0)
      {
         printf("Rule array is empty\n");
         return 0;  
      }
      for(cnt = rulenum - 1; cnt < dcerpc_ipv4_rule_cnt - 1; cnt++)
      {
         strcpy(dcerpc_ipv4_result[cnt].action, dcerpc_ipv4_result[cnt + 1].action);
         strcpy(dcerpc_ipv4_rule[cnt].flow, dcerpc_ipv4_rule[cnt + 1].flow);
         dcerpc_ipv4_rule[cnt].sport = dcerpc_ipv4_rule[cnt + 1].sport;
         dcerpc_ipv4_rule[cnt].dipaddr = dcerpc_ipv4_rule[cnt + 1].dipaddr;
         dcerpc_ipv4_rule[cnt].dport = dcerpc_ipv4_rule[cnt + 1].dport;
      }
      dcerpc_ipv4_rule_cnt--;
   }
   return 1;
}

uint32_t add_tcp_ipv6_rule(char *action, char *version, uint64_t sip, 
                      uint32_t sprt, char *direction, uint64_t dip, uint32_t dprt)
{
    static uint32_t i;
    uint32_t temp;

    for (temp = 0 ; temp <= i; temp++)
    {
        if (ipv6_rule_lookup(&tcp_ipv6_rule[temp], &tcp_ipv6_result[temp], action, version, sip, sprt, direction, dip, dprt))
        {
            return 0;
        }
    }
    strcpy(tcp_ipv6_result[i].action, action);
    strcpy(tcp_ipv6_rule[i].flow, direction);
    tcp_ipv6_rule[i].sipaddr = sip;
    tcp_ipv6_rule[i].sport = sprt;
    tcp_ipv6_rule[i].dipaddr = dip;
    tcp_ipv6_rule[i].dport = dprt;
    tcp_ipv6_rule[i].proto = rTCP;

/*
    strcpy(tcp_ipv6_rule[i].action, action);
    strcpy(tcp_ipv6_rule[i].version, version);
    strcpy(tcp_ipv6_rule[i].direction, direction);
    tcp_ipv6_rule[i].sipaddr = sip;
    tcp_ipv6_rule[i].sport = sprt;
    tcp_ipv6_rule[i].dipaddr = dip;
    tcp_ipv6_rule[i].dport = dprt;*/

    i++;
    tcp_ipv6_rule_cnt = i;
    return 1;
}


uint32_t add_udp_ipv6_rule(char *action, char *version, uint64_t sip, 
                      uint32_t sprt, char *direction, uint64_t dip, uint32_t  dprt)
{

    static uint32_t i;
    uint32_t temp;
//  printf("inside add_udp_ipv4_rule\n");
    for (temp = 0 ; temp <= i; temp++)
    {
        if (ipv6_rule_lookup(&udp_ipv6_rule[temp], &udp_ipv6_result[temp], action, version, sip, sprt, direction, dip, dprt))
        {
            return 0;
        }
    }
    strcpy(udp_ipv6_result[i].action, action);
    strcpy(udp_ipv6_rule[i].flow, direction);
    udp_ipv6_rule[i].sipaddr = sip;
    udp_ipv6_rule[i].sport = sprt;
    udp_ipv6_rule[i].dipaddr = dip;
    udp_ipv6_rule[i].dport = dprt;
    udp_ipv6_rule[i].proto = rUDP;


    i++;
    udp_ipv6_rule_cnt = i;
    return 1;
} 
uint32_t add_icmp_ipv6_rule(char *action, char *version, uint64_t sip, 
                        uint32_t sprt, char *direction, uint64_t dip, uint32_t dprt)
{
   static uint32_t i;
   uint32_t temp;
// printf("inside add_vxlan_rule\n");
   for (temp = 0; temp <= i; temp++)
   {
       if (ipv6_rule_lookup(&icmp_ipv6_rule[temp], &icmp_ipv6_result[temp], action, version, sip, sprt, direction, dip, dprt))
       {
           return 0;
       }
   }

    strcpy(icmp_ipv6_result[i].action, action);
    strcpy(icmp_ipv6_rule[i].flow, direction);
    icmp_ipv6_rule[i].sipaddr = sip;
    icmp_ipv6_rule[i].sport = sprt;
    icmp_ipv6_rule[i].dipaddr = dip;
    icmp_ipv6_rule[i].dport = dprt;
    icmp_ipv6_rule[i].proto = rICMP;


   i++;
   icmp_ipv6_rule_cnt = i;
   return 1;
}

uint32_t add_http_ipv6_rule(char *action, char *version, uint64_t sip, 
                        uint32_t sprt, char *direction, uint64_t dip, uint32_t dprt)
{
   static uint32_t i;
   uint32_t temp;
// printf("inside add_vlan3_rule\n");
   for (temp = 0; temp <= i; temp++)
   {
       if (ipv6_rule_lookup(&http_ipv6_rule[temp], &http_ipv6_result[temp], action, version, sip, sprt, direction, dip, dprt))
       {
           return 0;
       }
   }
    strcpy(http_ipv6_result[i].action, action);
    strcpy(http_ipv6_rule[i].flow, direction);
    http_ipv6_rule[i].sipaddr = sip;
    http_ipv6_rule[i].sport = sprt;
    http_ipv6_rule[i].dipaddr = dip;
    http_ipv6_rule[i].dport = dprt;
    http_ipv6_rule[i].proto = rHTTP;


   i++;
   http_ipv6_rule_cnt = i;
   return 1;

}
uint32_t add_ftp_ipv6_rule(char *action, char *version, uint64_t sip, 
                       uint32_t sprt, char *direction, uint64_t dip, uint32_t dprt)
{
    static uint32_t i;
    uint32_t temp;
//  printf("inside add_gtpu_rule\n");
    for (temp = 0; temp <= i; temp++)
    {
        if (ipv6_rule_lookup(&ftp_ipv6_rule[temp], &ftp_ipv6_result[temp], action, version, sip, sprt, direction, dip, dprt))
        {
            return 0;
        }
    }
    strcpy(ftp_ipv6_result[i].action, action);
    strcpy(ftp_ipv6_rule[i].flow, direction);
    ftp_ipv6_rule[i].sipaddr = sip;
    ftp_ipv6_rule[i].sport = sprt;
    ftp_ipv6_rule[i].dipaddr = dip;
    ftp_ipv6_rule[i].dport = dprt;
    ftp_ipv6_rule[i].proto = rFTP;


    i++;
    ftp_ipv6_rule_cnt = i;
    return 1;
} 

uint32_t add_sctp_ipv6_rule(char *action, char *version, uint64_t sip, 
                        uint32_t sprt, char *direction, uint64_t dip, uint32_t dprt)
{
    static uint32_t i;
    uint32_t temp;
//  printf("inside add_nvgre_ipv4_rule\n");
    for (temp = 0; temp <= i; temp++)
    {
        if (ipv6_rule_lookup(&sctp_ipv6_rule[temp], &sctp_ipv6_result[temp], action, version, sip, sprt, direction, dip, dprt))
        {
            return 0;
        }
    }
    strcpy(sctp_ipv6_result[i].action, action);
    strcpy(sctp_ipv6_rule[i].flow, direction);
    sctp_ipv6_rule[i].sipaddr = sip;
    sctp_ipv6_rule[i].sport = sprt;
    sctp_ipv6_rule[i].dipaddr = dip;
    sctp_ipv6_rule[i].dport = dprt;
    sctp_ipv6_rule[i].proto = rSCTP;


    i++;
    sctp_ipv6_rule_cnt = i;
    return 1;
} 

uint32_t add_smtp_ipv6_rule(char *action, char *version, uint64_t sip, 
                        uint32_t sprt, char *direction, uint64_t dip, uint32_t dprt)
{
    static uint32_t i;
    uint32_t temp;
//  printf("inside add_nvgre_ipv4_rule\n");
    for (temp = 0; temp <= i; temp++)
    {
        if (ipv6_rule_lookup(&smtp_ipv6_rule[temp], &smtp_ipv6_result[temp], action, version, sip, sprt, direction, dip, dprt))
        {
            return 0;
        }
    }
    strcpy(smtp_ipv6_result[i].action, action);
    strcpy(smtp_ipv6_rule[i].flow, direction);
    smtp_ipv6_rule[i].sipaddr = sip;
    smtp_ipv6_rule[i].sport = sprt;
    smtp_ipv6_rule[i].dipaddr = dip;
    smtp_ipv6_rule[i].dport = dprt;
    smtp_ipv6_rule[i].proto = rSMTP;


    i++;
    smtp_ipv6_rule_cnt = i;
    return 1;
} 
uint32_t add_dns_ipv6_rule(char *action, char *version, uint64_t sip, 
                        uint32_t sprt, char *direction, uint64_t dip, uint32_t dprt)
{
    static uint32_t i;
    uint32_t temp;
//  printf("inside add_nvgre_ipv4_rule\n");
    for (temp = 0; temp <= i; temp++)
    {
        if (ipv6_rule_lookup(&dns_ipv6_rule[temp], &dns_ipv6_result[temp], action, version, sip, sprt, direction, dip, dprt))
        {
            return 0;
        }
    }
    strcpy(dns_ipv6_result[i].action, action);
    strcpy(dns_ipv6_rule[i].flow, direction);
    dns_ipv6_rule[i].sipaddr = sip;
    dns_ipv6_rule[i].sport = sprt;
    dns_ipv6_rule[i].dipaddr = dip;
    dns_ipv6_rule[i].dport = dprt;
    dns_ipv6_rule[i].proto = rDNS;


    i++;
    dns_ipv6_rule_cnt = i;
    return 1;
} 

uint32_t add_gre_ipv6_rule(char *action, char *version, uint64_t  sip, 
                 uint32_t sprt, char *direction, uint64_t dip, uint32_t dprt)
{
    static uint32_t i;
    uint32_t temp;
//  printf("inside add_udp_ipv4_rule\n");
    for (temp = 0 ; temp <= i; temp++)
    {
        if (ipv6_rule_lookup(&gre_ipv6_rule[temp], &gre_ipv6_result[temp], action, version, sip, sprt, direction, dip, dprt))
        {
            return 0;
        }
    }
    strcpy(gre_ipv6_result[i].action, action);
    strcpy(gre_ipv6_rule[i].flow, direction);
    gre_ipv6_rule[i].sipaddr = sip;
    gre_ipv6_rule[i].sport = sprt;
    gre_ipv6_rule[i].dipaddr = dip;
    gre_ipv6_rule[i].dport = dprt;
    gre_ipv6_rule[i].proto = rGRE;


    i++;
    gre_ipv6_rule_cnt = i;
    return 1;
}


uint32_t add_gtp_ipv6_rule( char *action, char *version, uint64_t sip, 
                        uint32_t sprt, char *direction, uint64_t dip, uint32_t  dprt)
{
    static uint32_t i;
    uint32_t temp;
//  printf("inside add_gtpc_rule\n");
    for (temp = 0; temp <= i; temp++)
    {
        if (ipv6_rule_lookup(&gtp_ipv6_rule[temp], &gtp_ipv6_result[temp], action, version, sip, sprt, direction, dip, dprt))
        {
            return 0;
        }
    }
    strcpy(gtp_ipv6_result[i].action, action);
    strcpy(gtp_ipv6_rule[i].flow, direction);
    gtp_ipv6_rule[i].sipaddr = sip;
    gtp_ipv6_rule[i].sport = sprt;
    gtp_ipv6_rule[i].dipaddr = dip;
    gtp_ipv6_rule[i].dport = dprt;
    gtp_ipv6_rule[i].proto = rGTP;


    i++;
    gtp_ipv6_rule_cnt = i;
    return 1;
} 

uint32_t add_smb_ipv6_rule(char *action, char *version, uint64_t sip, 
                        uint32_t sprt, char *direction, uint64_t dip, uint32_t dprt)
{
     static uint32_t i;
   uint32_t temp;
// printf("inside add_vlan1_rule\n");
   for (temp = 0; temp <= i; temp++)
   {
       if (ipv6_rule_lookup(&smb_ipv6_rule[temp], &smb_ipv6_result[temp], action, version, sip, sprt, direction, dip, dprt))
       {
           return 0;
       }
   }
    strcpy(smb_ipv6_result[i].action, action);
    strcpy(smb_ipv6_rule[i].flow, direction);
    smb_ipv6_rule[i].sipaddr = sip;
    smb_ipv6_rule[i].sport = sprt;
    smb_ipv6_rule[i].dipaddr = dip;
    smb_ipv6_rule[i].dport = dprt;
    smb_ipv6_rule[i].proto = rSMB;


   i++;
   smb_ipv6_rule_cnt = i;
   return 1;
}
uint32_t add_smb2_ipv6_rule(char *action, char *version, uint64_t sip, 
                        uint32_t sprt, char *direction, uint64_t dip, uint32_t dprt)
{
     static uint32_t i;
   uint32_t temp;
// printf("inside add_vlan1_rule\n");
   for (temp = 0; temp <= i; temp++)
   {
       if (ipv6_rule_lookup(&smb2_ipv6_rule[temp], &smb2_ipv6_result[temp], action, version, sip, sprt, direction, dip, dprt))
       {
           return 0;
       }
   }
    strcpy(smb2_ipv6_result[i].action, action);
    strcpy(smb2_ipv6_rule[i].flow, direction);
    smb2_ipv6_rule[i].sipaddr = sip;
    smb2_ipv6_rule[i].sport = sprt;
    smb2_ipv6_rule[i].dipaddr = dip;
    smb2_ipv6_rule[i].dport = dprt;
    smb2_ipv6_rule[i].proto = rSMB2;


   i++;
   smb2_ipv6_rule_cnt = i;
   return 1;
}
uint32_t add_dcerpc_ipv6_rule(char *action, char *version, uint64_t sip, 
                        uint32_t sprt, char *direction, uint64_t dip, uint32_t dprt)
{
     static uint32_t i;
   uint32_t temp;
// printf("inside add_vlan2_rule\n");
   for (temp = 0; temp <= i; temp++)
   {
       if (ipv6_rule_lookup(&dcerpc_ipv6_rule[temp], &dcerpc_ipv6_result[temp], action, version, sip, sprt, direction, dip, dprt))
       {
           return 0;
       }
   }
    strcpy(dcerpc_ipv6_result[i].action, action);
    strcpy(dcerpc_ipv6_rule[i].flow, direction);
    dcerpc_ipv6_rule[i].sipaddr = sip;
    dcerpc_ipv6_rule[i].sport = sprt;
    dcerpc_ipv6_rule[i].dipaddr = dip;
    dcerpc_ipv6_rule[i].dport = dprt;
    dcerpc_ipv6_rule[i].proto = rDCERPC;


   i++;
   dcerpc_ipv6_rule_cnt = i;
   return 1;
}

uint32_t add_ssl_ipv6_rule(char *action, char *version, uint64_t sip, 
                      uint32_t sprt, char *direction, uint64_t dip, uint32_t dprt)
{
   static uint32_t i;
   uint32_t temp;
// printf("inside add_ssl_ipv4_rule\n");
   for (temp = 0; temp <= i; temp++)
   {
       if (ipv6_rule_lookup(&ssl_ipv6_rule[temp], &ssl_ipv6_result[temp], action, version, sip, sprt, direction, dip, dprt))
       {
           return 0;
       }
   }
    strcpy(ssl_ipv6_result[i].action, action);
    strcpy(ssl_ipv6_rule[i].flow, direction);
    ssl_ipv6_rule[i].sipaddr = sip;
    ssl_ipv6_rule[i].sport = sprt;
    ssl_ipv6_rule[i].dipaddr = dip;
    ssl_ipv6_rule[i].dport = dprt;
    ssl_ipv6_rule[i].proto = rSSL;


   i++;
   ssl_ipv6_rule_cnt = i;
   return 1;
}

uint32_t add_tls_ipv6_rule(char *action, char *version, uint64_t sip, 
                      uint32_t sprt, char *direction, uint64_t dip, uint32_t dprt)
{
      static uint32_t i;
   uint32_t temp;
// printf("inside add_tls_ipv4_rule\n");
   for (temp = 0; temp <= i; temp++)
   {
       if (ipv6_rule_lookup(&tls_ipv6_rule[temp], &tls_ipv6_result[temp], action, version, sip, sprt, direction, dip, dprt))
       {
           return 0;
       }
   }
    strcpy(tls_ipv6_result[i].action, action);
    strcpy(tls_ipv6_rule[i].flow, direction);
    tls_ipv6_rule[i].sipaddr = sip;
    tls_ipv6_rule[i].sport = sprt;
    tls_ipv6_rule[i].dipaddr = dip;
    tls_ipv6_rule[i].dport = dprt;
    tls_ipv6_rule[i].proto = rTLS;


   i++;
   tls_ipv6_rule_cnt = i;
   return 1;
}
void show_pact_info(void)
{
    
    printf("\n%30s\n","PORT1 action info : ");
    if (fdrp_p1 == 1)
    {
        printf("%30s\n","Fragment drop enabled");
    }
    else 
    {
       printf("%30s\n","Fragment drop disabled");
    }
    if (drp_p1 == 1)
    {
        printf("%30s\n","Drop packet enabled");
    }
    else 
    {
       printf("%30s\n","Drop packet disabled");
    }
    if (acl_p1 == 1)
    {
        printf("%30s\n","Fragment drop enabled");
    }
    else 
    {
       printf("%30s\n","Fragment drop disabled");
    }
    if (st_p1 == 1)
    {
        printf("%30s\n","Suricata  enabled");
    }
    else 
    {
       printf("%30s\n","Suricata disabled ");
    }

    printf("\n%30s\n","PORT2  action info : ");
    if (fdrp_p2 == 1)
    {
        printf("%30s\n","Fragment drop enabled");
    }
    else 
    {
       printf("%30s\n","Fragment drop disabled");
    }
    if (drp_p2 == 1)
    {
        printf("%30s\n","Drop packet enabled");
    }
    else 
    {
       printf("%30s\n","Drop packet disabled");
    }
    if (acl_p2 == 1)
    {
        printf("%30s\n","Fragment drop enabled");
    }
    else 
    {
       printf("%30s\n","Fragment drop disabled");
    }
    if (st_p2 == 1)
    {
        printf("%30s\n","Suricata  enabled");
    }
    else 
    {
       printf("%30s\n","Suricata disabled\n");
    }

    printf("\n%30s\n","PORT3  action info : ");
    if (fdrp_p3 == 1)
    {
        printf("%30s\n","Fragment drop enabled");
    }
    else 
    {
       printf("%30s\n","Fragment drop disabled");
    }
    if (drp_p3 == 1)
    {
        printf("%30s\n","Drop packet enabled");
    }
    else 
    {
       printf("%30s\n","Drop packet disabled");
    }
    if (acl_p3 == 1)
    {
        printf("%30s\n","Fragment drop enabled");
    }
    else 
    {
       printf("%30s\n","Fragment drop disabled");
    }
    if (st_p3 == 1)
    {
        printf("%30s\n","Suricata  enabled");
    }
    else 
    {
       printf("%30s\n","Suricata disabled\n");
    }

    printf("\n%30s\n","PORT4 action info :");
    if (fdrp_p4 == 1)
    {
        printf("%30s\n","Fragment drop enabled");
    }
    else 
    {
       printf("%30s\n","Fragment drop disabled");
    }
    if (drp_p4 == 1)
    {
        printf("%30s\n","Drop packet enabled");
    }
    else 
    {
       printf("%30s\n","Drop packet disabled");
    }
    if (acl_p4 == 1)
    {
        printf("%30s\n","Fragment drop enabled");
    }
    else 
    {
       printf("%30s\n","Fragment drop disabled");
    }
    if (st_p4 == 1)
    {
        printf("%30s\n","Suricata  enabled");
    }
    else 
    {
       printf("%30s\n","Suricata disabled\n");
    }
}
