#include <stdio.h>
#include <stdlib.h>

#include "dpapi.h"

/* DEFINE */
/* GLOBAL */
uint32_t aclIpv4_index = 1;
uint32_t aclIpv6_index = 1;

/* EXTERN */
extern uint64_t rulesSet;

extern struct dp_ruleCount ruleCnt;
extern struct dp_resultTuple aclIpv4Result [1024];
extern struct dp_resultTuple aclIpv6Result [1024];

int32_t updateAcl_ipv4(struct dp_ipv4Tuple *param, struct dp_resultTuple *reslt)
{
    if ((NULL == param) || (NULL == reslt))
        return -1;

    if (aclIpv4_index > MAX_RULE_COUNT)
        return -2;

    aclIpv4Result [aclIpv4_index -1].action   = (*reslt).action;
    aclIpv4Result [aclIpv4_index -1].msgIndex = (*reslt).msgIndex;
    aclIpv4Result [aclIpv4_index -1].counter  = (*reslt).counter;
    aclIpv4Result [aclIpv4_index -1].next     = (*reslt).next;

    /* TODO: Add to Result Table */
    if (tableLookupAddIpv4Rule(param, aclIpv4_index) < 0)
        return -2;

    aclIpv4_index++;
    ruleCnt.ipv4++;
    rulesSet |= (1 << MDPOS_IP4);
    return 0;
}

int32_t updateAcl_ipv4Tcp(struct dp_ipv4Tuple *param, struct dp_resultTuple *reslt)
{
    if ((NULL == param) || (NULL == reslt))
        return -1;

    if (aclIpv4_index > MAX_RULE_COUNT)
        return -2;

    aclIpv4Result [aclIpv4_index -1].action   = (*reslt).action;
    aclIpv4Result [aclIpv4_index -1].msgIndex = (*reslt).msgIndex;
    aclIpv4Result [aclIpv4_index -1].counter  = (*reslt).counter;
    aclIpv4Result [aclIpv4_index -1].next     = (*reslt).next;

    /* TODO: Add to Result Table */
    if (tableLookupAddIpv4Rule(param, aclIpv4_index) < 0)
        return -2;

    aclIpv4_index++;
    ruleCnt.tcp++;
    rulesSet |= (1 << MDPOS_TCP4);
    return 0;
}

int32_t updateAcl_ipv4Udp(struct dp_ipv4Tuple *param, struct dp_resultTuple *reslt)
{
    if ((NULL == param) || (NULL == reslt))
        return -1;

    if (aclIpv4_index > MAX_RULE_COUNT)
        return -2;

    aclIpv4Result [aclIpv4_index -1].action   = (*reslt).action;
    aclIpv4Result [aclIpv4_index -1].msgIndex = (*reslt).msgIndex;
    aclIpv4Result [aclIpv4_index -1].counter  = (*reslt).counter;
    aclIpv4Result [aclIpv4_index -1].next     = (*reslt).next;

    /* TODO: Add to Result Table */
    if (tableLookupAddIpv4Rule(param, aclIpv4_index) < 0)
        return -2;

    aclIpv4_index++;
    ruleCnt.udp++;
    rulesSet |= (1 << MDPOS_UDP4);
    return 0;
}

int32_t updateAcl_ipv4Icmp(struct dp_ipv4Tuple *param, struct dp_resultTuple *reslt)
{
    if ((NULL == param) || (NULL == reslt))
        return -1;

    if (aclIpv4_index > MAX_RULE_COUNT)
        return -2;

    aclIpv4Result [aclIpv4_index -1].action   = (*reslt).action;
    aclIpv4Result [aclIpv4_index -1].msgIndex = (*reslt).msgIndex;
    aclIpv4Result [aclIpv4_index -1].counter  = (*reslt).counter;
    aclIpv4Result [aclIpv4_index -1].next     = (*reslt).next;

    /* TODO: Add to Result Table */
    if (tableLookupAddIpv4Rule(param, aclIpv4_index) < 0)
        return -2;

    aclIpv4_index++;
    ruleCnt.icmp++;
    rulesSet |= (1 << MDPOS_ICMP4);
    return 0;
}

int32_t updateAcl_ipv4Http(struct dp_ipv4Tuple *param, struct dp_resultTuple *reslt)
{
    if ((NULL == param) || (NULL == reslt))
        return -1;

    if (aclIpv4_index > MAX_RULE_COUNT)
        return -2;

    aclIpv4Result [aclIpv4_index -1].action   = (*reslt).action;
    aclIpv4Result [aclIpv4_index -1].msgIndex = (*reslt).msgIndex;
    aclIpv4Result [aclIpv4_index -1].counter  = (*reslt).counter;
    aclIpv4Result [aclIpv4_index -1].next     = (*reslt).next;

    /* TODO: Add to Result Table */
    if (tableLookupAddIpv4Rule(param, aclIpv4_index) < 0)
        return -2;

    aclIpv4_index++;
    ruleCnt.http++;
    rulesSet |= (1 << MDPOS_HTTP4);
    return 0;
}

int32_t updateAcl_ipv4Ssl(struct dp_ipv4Tuple *param, struct dp_resultTuple *reslt)
{
    if ((NULL == param) || (NULL == reslt))
        return -1;

    if (aclIpv4_index > MAX_RULE_COUNT)
        return -2;

    aclIpv4Result [aclIpv4_index -1].action   = (*reslt).action;
    aclIpv4Result [aclIpv4_index -1].msgIndex = (*reslt).msgIndex;
    aclIpv4Result [aclIpv4_index -1].counter  = (*reslt).counter;
    aclIpv4Result [aclIpv4_index -1].next     = (*reslt).next;

    /* TODO: Add to Result Table */
    if (tableLookupAddIpv4Rule(param, aclIpv4_index) < 0)
        return -2;

    aclIpv4_index++;
    ruleCnt.ssl++;
    rulesSet |= (1 << MDPOS_SSL4);
    return 0;
}

int32_t updateAcl_ipv4Tls(struct dp_ipv4Tuple *param, struct dp_resultTuple *reslt)
{
    if ((NULL == param) || (NULL == reslt))
        return -1;

    if (aclIpv4_index > MAX_RULE_COUNT)
        return -2;

    aclIpv4Result [aclIpv4_index -1].action   = (*reslt).action;
    aclIpv4Result [aclIpv4_index -1].msgIndex = (*reslt).msgIndex;
    aclIpv4Result [aclIpv4_index -1].counter  = (*reslt).counter;
    aclIpv4Result [aclIpv4_index -1].next     = (*reslt).next;

    /* TODO: Add to Result Table */
    if (tableLookupAddIpv4Rule(param, aclIpv4_index) < 0)
        return -2;

    aclIpv4_index++;
    ruleCnt.tls++;
    rulesSet |= (1 << MDPOS_TLS4);
    return 0;
}

int32_t updateAcl_ipv4Smtp(struct dp_ipv4Tuple *param, struct dp_resultTuple *reslt)
{
    if ((NULL == param) || (NULL == reslt))
        return -1;

    if (aclIpv4_index > MAX_RULE_COUNT)
        return -2;

    aclIpv4Result [aclIpv4_index -1].action   = (*reslt).action;
    aclIpv4Result [aclIpv4_index -1].msgIndex = (*reslt).msgIndex;
    aclIpv4Result [aclIpv4_index -1].counter  = (*reslt).counter;
    aclIpv4Result [aclIpv4_index -1].next     = (*reslt).next;

    /* TODO: Add to Result Table */
    if (tableLookupAddIpv4Rule(param, aclIpv4_index) < 0)
        return -2;

    aclIpv4_index++;
    ruleCnt.smtp++;
    rulesSet |= (1 << MDPOS_SMTP4);
    return 0;
}

int32_t updateAcl_ipv4Ftp(struct dp_ipv4Tuple *param, struct dp_resultTuple *reslt)
{
    if ((NULL == param) || (NULL == reslt))
        return -1;

    if (aclIpv4_index > MAX_RULE_COUNT)
        return -2;

    aclIpv4Result [aclIpv4_index -1].action   = (*reslt).action;
    aclIpv4Result [aclIpv4_index -1].msgIndex = (*reslt).msgIndex;
    aclIpv4Result [aclIpv4_index -1].counter  = (*reslt).counter;
    aclIpv4Result [aclIpv4_index -1].next     = (*reslt).next;

    /* TODO: Add to Result Table */
    if (tableLookupAddIpv4Rule(param, aclIpv4_index) < 0)
        return -2;

    aclIpv4_index++;
    ruleCnt.ftp++;
    rulesSet |= (1 << MDPOS_FTP4);
    return 0;
}

int32_t updateAcl_ipv4Sctp(struct dp_ipv4Tuple *param, struct dp_resultTuple *reslt)
{
    if ((NULL == param) || (NULL == reslt))
        return -1;

    if (aclIpv4_index > MAX_RULE_COUNT)
        return -2;

    aclIpv4Result [aclIpv4_index -1].action   = (*reslt).action;
    aclIpv4Result [aclIpv4_index -1].msgIndex = (*reslt).msgIndex;
    aclIpv4Result [aclIpv4_index -1].counter  = (*reslt).counter;
    aclIpv4Result [aclIpv4_index -1].next     = (*reslt).next;

    /* TODO: Add to Result Table */
    if (tableLookupAddIpv4Rule(param, aclIpv4_index) < 0)
        return -2;

    aclIpv4_index++;
    ruleCnt.sctp++;
    rulesSet |= (1 << MDPOS_SCTP4);
    return 0;
}

int32_t updateAcl_ipv4Gre(struct dp_ipv4Tuple *param, struct dp_resultTuple *reslt)
{
    if ((NULL == param) || (NULL == reslt))
        return -1;

    if (aclIpv4_index > MAX_RULE_COUNT)
        return -2;

    aclIpv4Result [aclIpv4_index -1].action   = (*reslt).action;
    aclIpv4Result [aclIpv4_index -1].msgIndex = (*reslt).msgIndex;
    aclIpv4Result [aclIpv4_index -1].counter  = (*reslt).counter;
    aclIpv4Result [aclIpv4_index -1].next     = (*reslt).next;

    /* TODO: Add to Result Table */
    if (tableLookupAddIpv4Rule(param, aclIpv4_index) < 0)
        return -2;

    aclIpv4_index++;
    ruleCnt.gre++;
    rulesSet |= (1 << MDPOS_GRE4);
    return 0;
}

int32_t updateAcl_ipv6(struct dp_ipv6Tuple *param, struct dp_resultTuple *reslt)
{
    if ((NULL == param) || (NULL == reslt))
        return -1;

    ruleCnt.ipv6++;
    rulesSet |= (1UL << MDPOS_IP6);
    return 0;
}

int32_t updateAcl_ipv6Tcp(struct dp_ipv6Tuple *param, struct dp_resultTuple *reslt)
{
    if ((NULL == param) || (NULL == reslt))
        return -1;

    ruleCnt.tcp++;
    rulesSet |= (1UL << MDPOS_TCP6);
    return 0;
}

int32_t updateAcl_ipv6Udp(struct dp_ipv6Tuple *param, struct dp_resultTuple *reslt)
{
    if ((NULL == param) || (NULL == reslt))
        return -1;

    ruleCnt.udp++;
    rulesSet |= (1UL << MDPOS_UDP6);
    return 0;
}

int32_t updateAcl_ipv6Icmp(struct dp_ipv6Tuple *param, struct dp_resultTuple *reslt)
{
    if ((NULL == param) || (NULL == reslt))
        return -1;

    ruleCnt.icmp++;
    rulesSet |= (1UL << MDPOS_ICMP6);
    return 0;
}

int32_t updateAcl_ipv6Http(struct dp_ipv6Tuple *param, struct dp_resultTuple *reslt)
{
    if ((NULL == param) || (NULL == reslt))
        return -1;

    ruleCnt.http++;
    rulesSet |= (1UL << MDPOS_HTTP6);
    return 0;
}

int32_t updateAcl_ipv6Ssl(struct dp_ipv6Tuple *param, struct dp_resultTuple *reslt)
{
    if ((NULL == param) || (NULL == reslt))
        return -1;

    ruleCnt.ssl++;
    rulesSet |= (1UL << MDPOS_SSL6);
    return 0;
}

int32_t updateAcl_ipv6Tls(struct dp_ipv6Tuple *param, struct dp_resultTuple *reslt)
{
    if ((NULL == param) || (NULL == reslt))
        return -1;

    ruleCnt.tls++;
    rulesSet |= (1UL << MDPOS_TLS6);
    return 0;
}

int32_t updateAcl_ipv6Smtp(struct dp_ipv6Tuple *param, struct dp_resultTuple *reslt)
{
    if ((NULL == param) || (NULL == reslt))
        return -1;

    ruleCnt.smtp++;
    rulesSet |= (1UL << MDPOS_SMTP6);
    return 0;
}

int32_t updateAcl_ipv6Ftp(struct dp_ipv6Tuple *param, struct dp_resultTuple *reslt)
{
    if ((NULL == param) || (NULL == reslt))
        return -1;

    ruleCnt.ftp++;
    rulesSet |= (1UL << MDPOS_FTP6);
    return 0;
}

int32_t updateAcl_ipv6Sctp(struct dp_ipv6Tuple *param, struct dp_resultTuple *reslt)
{
    if ((NULL == param) || (NULL == reslt))
        return -1;

    ruleCnt.sctp++;
    rulesSet |= (1UL << MDPOS_SCTP6);
    return 0;
}

int32_t updateAcl_ipv6Gre(struct dp_ipv6Tuple *param, struct dp_resultTuple *reslt)
{
    if ((NULL == param) || (NULL == reslt))
        return -1;

    ruleCnt.gre++;
    rulesSet |= (1UL << MDPOS_GRE6);
    return 0;
}

