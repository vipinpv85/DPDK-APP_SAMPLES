#include <rte_acl.h>
#include "dpcommon.h"
#include "dptables.h"
#include "acl.h"

/* gloabls */
uint8_t   destPort [S_DPDK_MAX_ETHPORTS];
uint8_t   defPortMap [S_DPDK_MAX_ETHPORTS] = {2, 1, 4, 3}; /* 1 <--> 2, 3 <--> 4*/
uint16_t  aclIpv4Count = 0;
uint64_t  rulesSet = 0x00;

struct   dp_ruleCount  ruleCnt;
struct   rte_acl_ctx  *ctxIpv4 = NULL, *ctxIpv6 = NULL, *tempCtx = NULL;
struct   dp_resultTuple aclIpv4Result [1024];
struct   dp_resultTuple aclIpv6Result [1024];

//uint8_t  defPortAction [S_DPDK_MAX_ETHPORTS] = {, , , };

static struct rte_acl_param dpParam = 
{
    .name = S_DPDK_IPV4_ACL,
    .socket_id = SOCKET_ID_ANY,
};

struct  rte_acl_rule ruleIpv4[MAX_RULE_COUNT];



struct rte_acl_field_def ipv4Defs [NUM_FIELDS_IP] = {
    {
        .type = RTE_ACL_FIELD_TYPE_MASK,
        .size = sizeof(uint8_t),
        .field_index = DPLKP_IPV4_FLWDR,
        .input_index = DPLKP_IPV4_FLWDR,
        .offset = offsetof(struct dp_ipv4Tuple, flow),
        //.input_index = 0,
        //.offset = 0,
    },
    {
        .type = RTE_ACL_FIELD_TYPE_BITMASK,
        .size = sizeof(uint8_t),
        .field_index = DPLKP_IPV4_PROTO,
        .input_index = DPLKP_IPV4_PROTO,
        .offset = offsetof(struct dp_ipv4Tuple, proto),
        //.input_index = 0,
        //.offset = 1,
    },
    {
        .type = RTE_ACL_FIELD_TYPE_RANGE,
        .size = sizeof(uint16_t),
        .field_index = DPLKP_IPV4_SPORT,
        .input_index = DPLKP_IPV4_SPORT,
        .offset = offsetof(struct dp_ipv4Tuple, sPort),
        //.input_index = 1,
        //.offset = 2,
    },
    {
        .type = RTE_ACL_FIELD_TYPE_RANGE,
        .size = sizeof(uint16_t),
        .field_index = DPLKP_IPV4_DPORT,
        .input_index = DPLKP_IPV4_SPORT,
        //.input_index = DPLKP_IPV4_DPORT,
        .offset = offsetof(struct dp_ipv4Tuple, dPort),
        //.input_index = 1,
        //.offset = 4,
    },
    {
        .type = RTE_ACL_FIELD_TYPE_MASK,
        .size = sizeof(uint32_t),
        .field_index = DPLKP_IPV4_SRCIP,
        .input_index = DPLKP_IPV4_SRCIP,
        .offset = offsetof(struct dp_ipv4Tuple, srcIp),
        //.input_index = 2,
        //.offset = 6,
    },
    {
        .type = RTE_ACL_FIELD_TYPE_MASK,
        .size = sizeof(uint32_t),
        .field_index = DPLKP_IPV4_DSTIP,
        .input_index = DPLKP_IPV4_DSTIP,
        .offset = offsetof(struct dp_ipv4Tuple, dstIp),
        //.input_index = 2,
        //.offset = 10,
    },
 };

int32_t tableLookupCreate()
{
    int32_t dim          = RTE_DIM(ipv4Defs);
    dpParam.rule_size    = RTE_ACL_RULE_SZ(dim);
    dpParam.max_rule_num = MAX_RULE_COUNT;

    ctxIpv4 = rte_acl_create(&dpParam);
    if (NULL == ctxIpv4)
    {
         MSG("ERROR: ACL Context");
         return -1;
    }

    if (rte_acl_set_ctx_classify(ctxIpv4,
             RTE_ACL_CLASSIFY_SCALAR) != 0)
    {
        MSG("ERROR: ACL Classify Algo");
        return -1;
    }

    tempCtx = rte_acl_find_existing(S_DPDK_IPV4_ACL);
    if (NULL == tempCtx)
    {
         MSG("ERROR: ACL Table");
         return -1;
    }

    printf("%p Name: %s Rules %u", tempCtx, ((*tempCtx)).name, ((struct rte_acl_ctx*)tempCtx)->max_rules);
    printf("\n ctxIpv4 %p temp %p", ctxIpv4, tempCtx);

    return 0;
}

int32_t tableLookupAddIpv4Rule(struct dp_ipv4Tuple *params, uint32_t userData)
{
    static uint32_t index = 0;
    int32_t ret = 0;

    if ((NULL == params) || (userData == 0))
    {
        MSG("ERROR: invalid entries");
        return -1;
    }

    ruleIpv4[index].field[DPLKP_IPV4_FLWDR].value.u8   = params->flow;
    ruleIpv4[index].field[DPLKP_IPV4_PROTO].value.u8   = params->proto;
    ruleIpv4[index].field[DPLKP_IPV4_SPORT].value.u16  = params->sPort;
    ruleIpv4[index].field[DPLKP_IPV4_DPORT].value.u16  = params->dPort;
    ruleIpv4[index].field[DPLKP_IPV4_SRCIP].value.u32  = params->srcIp;
    ruleIpv4[index].field[DPLKP_IPV4_DSTIP].value.u32  = params->dstIp;

    ruleIpv4[index].field[DPLKP_IPV4_FLWDR].mask_range.u8  = (params->flow  != 0)?0x00:0xFF;
    ruleIpv4[index].field[DPLKP_IPV4_PROTO].mask_range.u8  = (params->proto != 0)?0x00:0xFF;
    ruleIpv4[index].field[DPLKP_IPV4_SPORT].mask_range.u16 = (params->sPort != 0)?0x00:0xFFFF;
    ruleIpv4[index].field[DPLKP_IPV4_DPORT].mask_range.u16 = (params->dPort != 0)?0x00:0xFFFF;
    ruleIpv4[index].field[DPLKP_IPV4_SRCIP].mask_range.u32 = (params->srcIp != 0)?0x00:0xFFFFFFFF;
    ruleIpv4[index].field[DPLKP_IPV4_DSTIP].mask_range.u32 = (params->dstIp != 0)?0x00:0xFFFFFFFF;

    ruleIpv4[index].data.category_mask = LEN2MASK(RTE_ACL_MAX_CATEGORIES);
    ruleIpv4[index].data.priority      = 1;
    ruleIpv4[index].data.userdata      = userData;

    ret = rte_acl_add_rules(ctxIpv4, (struct rte_acl_rule *)&(ruleIpv4 [index]), 1);
    if (ret < 0)
    {
        MSG("ERROR: Add Rules IPV4 %d", ret);
        return -1;
    }

    aclIpv4Count++;
    index++;
    MSG("DEBUG: Added IPV4 Rule");

    MSG("DEBUG: TCP rule: %u UDP: %u ICMP: %u", ruleCnt.tcp, ruleCnt.udp, ruleCnt.icmp);

    rte_acl_dump(ctxIpv4);
    return 0;
}

int32_t tableLookup_ipv4Rule_addBulk(struct dp_ipv4Tuple *params, uint32_t num, uint32_t *userData)
{
    uint32_t index = 0, aclRules = 0;
    int32_t ret = 0;

    if ((NULL == params) || (num == 0) || (userData == NULL))
    {
        MSG("ERROR: invalid entries");
        return -1;
    }

    /* add user rule to table one by one */
    for (; index < num; index++)
    {
        if (tableLookupAddIpv4Rule(&params [index], userData [index]) != 0)
        {
            MSG("ERROR: failed to add rule %u", index);
            continue;
        }
        aclRules += 1;
    }

    MSG("INFO: IPV4 ACL rules added %u", aclRules);

    rte_acl_dump(ctxIpv4);
    return 0;
}

int32_t tableLookup_ipv4Rules_build(void)
{
    int32_t ret = 0; 
    struct rte_acl_config cfg;

    cfg.num_categories = DEFAULT_MAX_CATEGORIES;
    cfg.num_fields     = 6;
    memcpy(cfg.defs, &ipv4Defs, sizeof(ipv4Defs));

    ret = rte_acl_build(ctxIpv4, &cfg);
    if (ret < 0)
    {
        MSG("ERROR: IPV4 ACL Build %d", ret);
        return -1;
    }

    MSG("DEBUG: Build IPV4 ACL Success");

    rte_acl_dump(ctxIpv4);
    return 0;
}

int32_t tableLookup_ipv4Rules_lookup(struct dp_ipv4Tuple *params)
{
    int32_t ret = 0;
    uint8_t ele [14];
    const uint8_t *ptr[1];
    uint32_t result = 0;

    if (NULL == params)
    {
        MSG("ERROR: input params invalid!!!");
        return -1;
    }

    ele [ 0] = params->flow;
    ele [ 1] = params->proto;
    ele [ 2] = (params->srcIp & 0xFF000000) >> 24;
    ele [ 3] = (params->srcIp & 0x00FF0000) >> 16;
    ele [ 4] = (params->srcIp & 0x0000FF00) >>  8;
    ele [ 5] = (params->srcIp & 0x000000FF) >>  0;
    ele [ 6] = (params->dstIp & 0xFF000000) >> 24;
    ele [ 7] = (params->dstIp & 0x00FF0000) >> 16;
    ele [ 8] = (params->dstIp & 0x0000FF00) >>  8;
    ele [ 9] = (params->dstIp & 0x000000FF) >>  0;
    ele [10] = (params->sPort & 0xFF00) >> 8;
    ele [10] = (params->sPort & 0x00FF) >> 0;
    ele [12] = (params->dPort & 0xFF00) >> 8;
    ele [12] = (params->dPort & 0x00FF) >> 0;

    ptr [0] = ele;

    ret = rte_acl_classify(ctxIpv4, ptr, &result, 1, 1);
    if (ret != 0)
    {
        MSG("ERROR: failed to lookup in ACL IPv4");
        return -2;
    }

    if (result <= 0)
        return -3;

    return result;
}

int32_t tableLookupDestroy(void)
{
    if (NULL != rte_acl_find_existing(S_DPDK_IPV4_ACL))
    {
        rte_acl_reset(ctxIpv4);
        rte_acl_free(ctxIpv4);

        /* check for residue */
        if (unlikely(NULL != rte_acl_find_existing(S_DPDK_IPV4_ACL)))
        {
            MSG("ERROR: failed to delete %s", S_DPDK_IPV4_ACL);
            return -1;
        }
    }

    if (NULL != rte_acl_find_existing(S_DPDK_IPV6_ACL))
    {
        rte_acl_reset(ctxIpv6);
        rte_acl_free(ctxIpv6);

        /* check for residue */
        if (unlikely(NULL != rte_acl_find_existing(S_DPDK_IPV6_ACL)))
        {
            MSG("ERROR: failed to delete %s", S_DPDK_IPV6_ACL);
            return -1;
        }
    }

    MSG("DEBUG: Deleted IPV4 & IPV6 ACL");
    return 0;
}

