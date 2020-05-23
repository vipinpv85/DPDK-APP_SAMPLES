//config.c


/* I N C L U D E S */
#include "cparser.h"
#include "cparser_tree.h"
#include "cparser_token.h"
#include "cparser_priv.h"
//header file included for tls rule
#include "common_struct.h"
#include "config.h"
#include "api.h"

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <inttypes.h>

static uint32_t ipv4_rule_cnt;
static uint32_t ipv6_rule_cnt;

cparser_result_t cparser_ssli_submode_enter(cparser_t *parser, int8_t *prompt)
{

        cparser_node_t *new_root;

        if (!parser) {
                return CPARSER_ERR_INVALID_PARAMS;
        }
        if ((CPARSER_MAX_NESTED_LEVELS-1) == parser->root_level) {
                return CPARSER_NOT_OK;
        }
        parser->root_level++;
        new_root = parser->cur_node->children;
        assert(new_root);
        assert(CPARSER_NODE_ROOT == new_root->type);
        parser->root[parser->root_level] = new_root;
        snprintf(parser->prompt[parser->root_level],
                        sizeof(parser->prompt[parser->root_level]), "%s", prompt);
        return CPARSER_OK;
}

cparser_result_t cparser_cmd_cnfg(cparser_context_t *context)
{
        int8_t prompt[30]={0};
        snprintf((char*)prompt, CPARSER_MAX_PROMPT, "%s> ", "CNFG");
        return cparser_ssli_submode_enter(context->parser, prompt);
}
cparser_result_t cparser_cmd_cnfg_rule(cparser_context_t *context)
{
        int8_t prompt[30]={0};
        snprintf((char*)prompt, CPARSER_MAX_PROMPT, "%s-%s> ", "CNFG","RULE");
        return cparser_ssli_submode_enter(context->parser, prompt);
}
cparser_result_t cparser_cmd_rule_add_action_version_proto_sip_sprt_direction_dip_dprt(cparser_context_t *context,
    char **action_ptr,
    char **version_ptr,
    char **proto_ptr,
    uint32_t *sip_ptr,
    uint32_t *sprt_ptr,
    char **direction_ptr,
    uint32_t *dip_ptr,
    uint32_t *dprt_ptr)
{
    
    int ret = 0;
    uint32_t ipaddr;
    if(!strcmp(*version_ptr, "ipv4"))
    {
        ipaddr = *sip_ptr;
        ret = validate_ip(ipaddr);

        if (ret == 0)
        {
            printf("\n invalid source address\n");
            return 0;
        }

        ret = validate_port(sprt_ptr);
        if (ret == 0)
        {
            printf("invalid source port\n");
            return 0;
        }

        ipaddr = *dip_ptr;
        ret = validate_ip(ipaddr);

        if (ret == 0)
        {
            printf("\n invalid destination address\n");
            return 0;
        }

        ret = validate_port(dprt_ptr);

        if (ret == 0)
        {
            printf("invalid destination port\n");
            return 0;
        }


        if (!strcmp(*proto_ptr,"tcp"))
        {// api to add tcp rule   
            ret =  add_tcp_ipv4_rule(*action_ptr, *version_ptr, *sip_ptr, *sprt_ptr, *direction_ptr, *dip_ptr, *dprt_ptr);
            if (ret == 1)
            {
                ipv4_rule_cnt++;
                printf("Rule added successfully\n");
            }
            else
            {
                 printf("Rule already exist\n");
            }
        }
        else if (!strcmp(*proto_ptr, "udp"))
        {
            ret = add_udp_ipv4_rule(*action_ptr, *version_ptr, *sip_ptr, *sprt_ptr, *direction_ptr, *dip_ptr, *dprt_ptr);
            if (ret == 1)
            {
                ipv4_rule_cnt++;
                printf("Rule added successfully\n");
            }
            else
            {
                printf("Rule already exist\n");
            }
        }
        else if (!strcmp(*proto_ptr, "smtp"))
        {
            ret = add_smtp_ipv4_rule(*action_ptr, *version_ptr, *sip_ptr, *sprt_ptr, *direction_ptr, *dip_ptr, *dprt_ptr);
            if (ret == 1)
            {
                ipv4_rule_cnt++;
                printf("Rule added successfully\n");
            }
            else
            {
                printf("Rule already exist\n");
            }
        }
        else if (!strcmp(*proto_ptr, "sctp"))
        {
            ret = add_sctp_ipv4_rule(*action_ptr, *version_ptr, *sip_ptr, *sprt_ptr, *direction_ptr, *dip_ptr, *dprt_ptr);
            if (ret == 1)
            {
                ipv4_rule_cnt++;
                printf("Rule added successfully\n");
            }
            else
            {
                printf("Rule already exist\n");
            }
        }
        else if (!strcmp(*proto_ptr, "smb"))
        {
            ret = add_smb_ipv4_rule(*action_ptr, *version_ptr, *sip_ptr, *sprt_ptr, *direction_ptr, *dip_ptr, *dprt_ptr);
            if (ret == 1)
            {
                ipv4_rule_cnt++;
                printf("Rule added successfully\n");
            }
            else
            {
                printf("Rule already exist\n");
            }
        }
        else if (!strcmp(*proto_ptr, "tls"))
        {
            ret = add_tls_ipv4_rule(*action_ptr, *version_ptr, *sip_ptr, *sprt_ptr, *direction_ptr, *dip_ptr, *dprt_ptr);
            if (ret == 1)
            {
                ipv4_rule_cnt++;
                printf("Rule added successfully\n");
            }
            else
            {
                printf("Rule already exist\n");
            }
        } 
        else if (!strcmp(*proto_ptr, "ssl"))
        {
            ret = add_ssl_ipv4_rule(*action_ptr, *version_ptr, *sip_ptr, *sprt_ptr, *direction_ptr, *dip_ptr, *dprt_ptr);
            if (ret == 1)
            {
                ipv4_rule_cnt++;
                printf("Rule added successfully\n");
            }
            else
            {
                printf("Rule already exist\n");
            }
        }
        else if (!strcmp(*proto_ptr, "gre"))
        {
            ret = add_gre_ipv4_rule(*action_ptr, *version_ptr, *sip_ptr, *sprt_ptr, *direction_ptr, *dip_ptr, *dprt_ptr);
            if (ret == 1)
            {
                ipv4_rule_cnt++;
                printf("Rule added successfully\n");
            }
            else
            {
                printf("Rule already exist\n");
            }
        }
        else if (!strcmp(*proto_ptr, "gtp"))
        {
            ret = add_gtp_ipv4_rule(*action_ptr, *version_ptr, *sip_ptr, *sprt_ptr, *direction_ptr, *dip_ptr, *dprt_ptr);
            if (ret == 1)
            {
                ipv4_rule_cnt++;
                printf("Rule added successfully\n");
            }
            else
            {
                printf("Rule already exist\n");
            }
        }
        else if (!strcmp(*proto_ptr, "ftp"))
        {
            ret = add_ftp_ipv4_rule(*action_ptr, *version_ptr, *sip_ptr, *sprt_ptr, *direction_ptr, *dip_ptr, *dprt_ptr);
            if (ret == 1)
            {
                ipv4_rule_cnt++;
                printf("Rule added successfully\n");
            }
            else
            {
                printf("Rule already exist\n");
            }
       }
       else if (!strcmp(*proto_ptr, "dns"))
       {
           ret = add_dns_ipv4_rule(*action_ptr, *version_ptr, *sip_ptr, *sprt_ptr, *direction_ptr, *dip_ptr, *dprt_ptr);
           if (ret == 1)
            {
                ipv4_rule_cnt++;
                printf("Rule added successfully\n");
            }
            else
            {
                printf("Rule already exist\n");
            }
        }
        else if (!strcmp(*proto_ptr, "http"))
        {
            ret =  add_http_ipv4_rule(*action_ptr, *version_ptr, *sip_ptr, *sprt_ptr, *direction_ptr, *dip_ptr, *dprt_ptr);
            if (ret == 1)
            {
                ipv4_rule_cnt++;
                printf("Rule added successfully\n");
            }
            else
            {
                printf("Rule already exist\n");
            }
        }
        else if (!strcmp(*proto_ptr, "icmp"))
        {
            ret =  add_icmp_ipv4_rule(*action_ptr, *version_ptr, *sip_ptr, *sprt_ptr, *direction_ptr, *dip_ptr, *dprt_ptr);
            if (ret == 1)
            {
                ipv4_rule_cnt++;
                printf("Rule added successfully\n");
            }
            else
            {
                printf("Rule already exist\n");
            }
        }
        else if (!strcmp(*proto_ptr, "dcerpc"))
        {
            ret =  add_dcerpc_ipv4_rule(*action_ptr, *version_ptr, *sip_ptr, *sprt_ptr, *direction_ptr, *dip_ptr, *dprt_ptr);
            if (ret == 1)
            {
                ipv4_rule_cnt++;
                printf("Rule added successfully\n");
            }
            else
            {
                printf("Rule already exist\n");
            }
        }
    }
    else
    {
        if (!strcmp(*proto_ptr,"tcp"))
        {// api to add tcp rule   
            ret =  add_tcp_ipv6_rule(*action_ptr, *version_ptr, *sip_ptr, *sprt_ptr, *direction_ptr, *dip_ptr, *dprt_ptr);
            if (ret == 1)
            {
                ipv6_rule_cnt++;
                printf("Rule added successfully\n");
            }
            else
            {
                 printf("Rule already exist\n");
            }
        }
        else if (!strcmp(*proto_ptr, "udp"))
        {
            ret = add_udp_ipv6_rule(*action_ptr, *version_ptr, *sip_ptr, *sprt_ptr, *direction_ptr, *dip_ptr, *dprt_ptr);
            if (ret == 1)
            {
                ipv6_rule_cnt++;
                printf("Rule added successfully\n");
            }
            else
            {
                printf("Rule already exist\n");
            }
        }
        else if (!strcmp(*proto_ptr, "smtp"))
        {
            ret = add_smtp_ipv6_rule(*action_ptr, *version_ptr, *sip_ptr, *sprt_ptr, *direction_ptr, *dip_ptr, *dprt_ptr);
            if (ret == 1)
            {
                ipv6_rule_cnt++;
                printf("Rule added successfully\n");
            }
            else
            {
                printf("Rule already exist\n");
            }
        }
        else if (!strcmp(*proto_ptr, "sctp"))
        {
            ret = add_sctp_ipv6_rule(*action_ptr, *version_ptr, *sip_ptr, *sprt_ptr, *direction_ptr, *dip_ptr, *dprt_ptr);
            if (ret == 1)
            {
                ipv6_rule_cnt++;
                printf("Rule added successfully\n");
            }
            else
            {
                printf("Rule already exist\n");
            }
        }
        else if (!strcmp(*proto_ptr, "smb"))
        {
            ret = add_smb_ipv6_rule(*action_ptr, *version_ptr, *sip_ptr, *sprt_ptr, *direction_ptr, *dip_ptr, *dprt_ptr);
            if (ret == 1)
            {
                ipv6_rule_cnt++;
                printf("Rule added successfully\n");
            }
            else
            {
                printf("Rule already exist\n");
            }
        }
        else if (!strcmp(*proto_ptr, "tls"))
        {
            ret = add_tls_ipv6_rule(*action_ptr, *version_ptr, *sip_ptr, *sprt_ptr, *direction_ptr, *dip_ptr, *dprt_ptr);
            if (ret == 1)
            {
                ipv6_rule_cnt++;
                printf("Rule added successfully\n");
            }
            else
            {
                printf("Rule already exist\n");
            }
        } 
        else if (!strcmp(*proto_ptr, "ssl"))
        {
            ret = add_ssl_ipv6_rule(*action_ptr, *version_ptr, *sip_ptr, *sprt_ptr, *direction_ptr, *dip_ptr, *dprt_ptr);
            if (ret == 1)
            {
                ipv6_rule_cnt++;
                printf("Rule added successfully\n");
            }
            else
            {
                printf("Rule already exist\n");
            }
        }
        else if (!strcmp(*proto_ptr, "gre"))
        {
            ret = add_gre_ipv6_rule(*action_ptr, *version_ptr, *sip_ptr, *sprt_ptr, *direction_ptr, *dip_ptr, *dprt_ptr);
            if (ret == 1)
            {
                ipv6_rule_cnt++;
                printf("Rule added successfully\n");
            }
            else
            {
                printf("Rule already exist\n");
            }
        }
        else if (!strcmp(*proto_ptr, "gtp"))
        {
            ret = add_gtp_ipv6_rule(*action_ptr, *version_ptr, *sip_ptr, *sprt_ptr, *direction_ptr, *dip_ptr, *dprt_ptr);
            if (ret == 1)
            {
                ipv6_rule_cnt++;
                printf("Rule added successfully\n");
            }
            else
            {
                printf("Rule already exist\n");
            }
        }
        else if (!strcmp(*proto_ptr, "ftp"))
        {
            ret = add_ftp_ipv6_rule(*action_ptr, *version_ptr, *sip_ptr, *sprt_ptr, *direction_ptr, *dip_ptr, *dprt_ptr);
            if (ret == 1)
            {
                ipv6_rule_cnt++;
                printf("Rule added successfully\n");
            }
            else
            {
                printf("Rule already exist\n");
            }
       }
       else if (!strcmp(*proto_ptr, "dns"))
       {
           ret = add_dns_ipv6_rule(*action_ptr, *version_ptr, *sip_ptr, *sprt_ptr, *direction_ptr, *dip_ptr, *dprt_ptr);
           if (ret == 1)
            {
                ipv6_rule_cnt++;
                printf("Rule added successfully\n");
            }
            else
            {
                printf("Rule already exist\n");
            }
        }
        else if (!strcmp(*proto_ptr, "http"))
        {
            ret =  add_http_ipv6_rule(*action_ptr, *version_ptr, *sip_ptr, *sprt_ptr, *direction_ptr, *dip_ptr, *dprt_ptr);
            if (ret == 1)
            {
                ipv6_rule_cnt++;
                printf("Rule added successfully\n");
            }
            else
            {
                printf("Rule already exist\n");
            }
        }
        else if (!strcmp(*proto_ptr, "icmp"))
        {
            ret =  add_icmp_ipv6_rule(*action_ptr, *version_ptr, *sip_ptr, *sprt_ptr, *direction_ptr, *dip_ptr, *dprt_ptr);
            if (ret == 1)
            {
                ipv6_rule_cnt++;
                printf("Rule added successfully\n");
            }
            else
            {
                printf("Rule already exist\n");
            }
        }
        else if (!strcmp(*proto_ptr, "dcerpc"))
        {
            ret =  add_dcerpc_ipv6_rule(*action_ptr, *version_ptr, *sip_ptr, *sprt_ptr, *direction_ptr, *dip_ptr, *dprt_ptr);
            if (ret == 1)
            {
                ipv6_rule_cnt++;
                printf("Rule added successfully\n");
            }
            else
            {
                printf("Rule already exist\n");
            }
        }
    }
    return CPARSER_OK;
}
cparser_result_t cparser_cmd_rule_lst(cparser_context_t *context)
{
        int8_t prompt[30]={0};
        snprintf((char*)prompt, CPARSER_MAX_PROMPT, "%s-%s-%s> ", "CNFG","RULE","LST");
        return cparser_ssli_submode_enter(context->parser, prompt);
}

cparser_result_t cparser_cmd_lst_tcp(cparser_context_t *context)
{
    uint32_t cnt;
    if((tcp_ipv4_rule_cnt + tcp_ipv6_rule_cnt) == 0)
    {
        printf("No Rule Specified\n");
    return CPARSER_OK;
    }
    printf("%25s : %d \n","tcp ipv4 rule count ", tcp_ipv4_rule_cnt);
    printf("%25s : %d \n","tcp ipv6 rule count ", tcp_ipv6_rule_cnt);

    printf("\n%15s\n","IPV4 Rules:");
    for (cnt = 0; cnt < tcp_ipv4_rule_cnt; cnt++)
    {
        printf("\t\t%d. ", cnt + 1);
        show_ipv4_rule(&tcp_ipv4_rule[cnt], &tcp_ipv4_result[cnt]);
    }
    printf("\n%15s\n","IPV6 Rules:");
    for (cnt = 0; cnt < tcp_ipv6_rule_cnt; cnt++)
    {
        printf("\t\t%d. ", cnt + 1);
        show_ipv6_rule(&tcp_ipv6_rule[cnt], &tcp_ipv6_result[cnt]);
    }
    
    return CPARSER_OK;
}

cparser_result_t cparser_cmd_lst_udp(cparser_context_t *context)
{
    uint32_t cnt;
    if((udp_ipv4_rule_cnt + udp_ipv6_rule_cnt) == 0)
    {
        printf("No Rule Specified\n");
    return CPARSER_OK;
    }
    printf("%25s : %d \n","udp ipv4 rule count",udp_ipv4_rule_cnt);
    printf("%25s : %d \n","udp ipv6 rule count",udp_ipv6_rule_cnt);

    printf("\n%15s\n","IPV4 Rules:");
    for (cnt = 0; cnt < udp_ipv4_rule_cnt; cnt++)
    {
        printf("\t\t%d. ", cnt + 1);
        show_ipv4_rule(&udp_ipv4_rule[cnt], &udp_ipv4_result[cnt]);
    }
    printf("\n%15s\n","IPV6 Rules:");
    for (cnt = 0; cnt < udp_ipv6_rule_cnt; cnt++)
    {
        printf("\t\t%d. ", cnt + 1);
        show_ipv6_rule(&udp_ipv6_rule[cnt], &udp_ipv6_result[cnt]);
    }
    return CPARSER_OK;
}
cparser_result_t cparser_cmd_lst_ssl(cparser_context_t *context)
{
    uint32_t cnt;
    if((ssl_ipv4_rule_cnt + ssl_ipv6_rule_cnt) == 0)
    {
        printf("No Rule Specified\n");
    return CPARSER_OK;
    }
    printf("%25s : %d \n","ssl ipv4 rule count",ssl_ipv4_rule_cnt);
    printf("%25s : %d \n","ssl ipv6 rule count",ssl_ipv6_rule_cnt);

    printf("\n %15s\n","IPV4 Rules:");
    for (cnt = 0; cnt < ssl_ipv4_rule_cnt; cnt++)
    {
        printf("\t\t%d. ", cnt + 1);
        show_ipv4_rule(&ssl_ipv4_rule[cnt], &ssl_ipv4_result[cnt]);
    }
    printf("\n%15s\n","IPV6 Rules:");
    for (cnt = 0; cnt < ssl_ipv6_rule_cnt; cnt++)
    {
        printf("\t\t%d. ", cnt + 1);
        show_ipv6_rule(&ssl_ipv6_rule[cnt], &ssl_ipv6_result[cnt]);
    }
    return CPARSER_OK;
}

cparser_result_t cparser_cmd_lst_icmp(cparser_context_t *context)
{
    uint32_t cnt;
    if((icmp_ipv4_rule_cnt + icmp_ipv6_rule_cnt) == 0)
    {
        printf("No Rule Specified\n");
    return CPARSER_OK;
    }
    printf("%25s : %d \n","icmp ipv4 rule count",icmp_ipv4_rule_cnt);
    printf("%25s : %d \n","icmp ipv6 rule count",icmp_ipv6_rule_cnt);

    printf("\n%15s\n","IPV4 Rules:");
    for (cnt = 0; cnt < icmp_ipv4_rule_cnt; cnt++)
    {
        printf("\t\t%d. ", cnt + 1);
        show_ipv4_rule(&icmp_ipv4_rule[cnt], &icmp_ipv4_result[cnt]);
    }
    printf("\n%15s\n","IPV6 Rules:");
    for (cnt = 0; cnt < icmp_ipv6_rule_cnt; cnt++)
    {
        printf("\t\t%d. ", cnt + 1);
        show_ipv6_rule(&icmp_ipv6_rule[cnt], &icmp_ipv6_result[cnt]);
    }
    return CPARSER_OK;
}

cparser_result_t cparser_cmd_lst_http(cparser_context_t *context)
{
    uint32_t cnt;
    if((http_ipv4_rule_cnt + http_ipv6_rule_cnt) == 0)
    {
        printf("No Rule Specified\n");
    return CPARSER_OK;
    }
    printf("%25s : %d \n","http ipv4 rule count",http_ipv4_rule_cnt);
    printf("%25s : %d \n","http ipv6 rule count",http_ipv6_rule_cnt);

    printf("\n%15s\n","IPV4 Rules:");
    for (cnt = 0; cnt < http_ipv4_rule_cnt; cnt++)
    {
        printf("\t\t%d. ", cnt + 1);
        show_ipv4_rule(&http_ipv4_rule[cnt], &http_ipv4_result[cnt]);
    }
    printf("\n%15s\n","IPV6 Rules:");
    for (cnt = 0; cnt < http_ipv6_rule_cnt; cnt++)
    {
        printf("\t\t%d. ", cnt + 1);
        show_ipv6_rule(&http_ipv6_rule[cnt], &http_ipv6_result[cnt]);
    }
    return CPARSER_OK;
}
cparser_result_t cparser_cmd_lst_tls(cparser_context_t *context)
{
    uint32_t cnt;
    if((tls_ipv4_rule_cnt + tls_ipv6_rule_cnt) == 0)
    {
        printf("No Rule Specified\n");
    return CPARSER_OK;
    }
    printf("%25s : %d \n","tls ipv4 rule count",tls_ipv4_rule_cnt);
    printf("%25s : %d \n","tls ipv6 rule count",tls_ipv6_rule_cnt);

    printf("\n%15s","IPV4 Rules:");
    for (cnt = 0; cnt < tls_ipv4_rule_cnt; cnt++)
    {
        printf("\t\t%d. ", cnt + 1);
        show_ipv4_rule(&tls_ipv4_rule[cnt], &tls_ipv4_result[cnt]);
    }
    printf("\n%15s\n","IPV6 Rules:");
    for (cnt = 0; cnt < tls_ipv6_rule_cnt; cnt++)
    {
        printf("\t\t%d. ", cnt + 1);
        show_ipv6_rule(&tls_ipv6_rule[cnt], &tls_ipv6_result[cnt]);
    }
    return CPARSER_OK;
}
cparser_result_t cparser_cmd_lst_gre(cparser_context_t *context)
{
    uint32_t cnt;
    if((gre_ipv4_rule_cnt + gre_ipv6_rule_cnt) == 0)
    {
        printf("No Rule Specified\n");
    return CPARSER_OK;
    }
    printf("%25s : %d \n","gre ipv4 rule count",gre_ipv4_rule_cnt);
    printf("%25s : %d \n","gre ipv6 rule count",gre_ipv6_rule_cnt);

    printf("\n%15s\n","IPV4 Rules:");
    for (cnt = 0; cnt < gre_ipv4_rule_cnt; cnt++)
    {
        printf("\t\t%d. ", cnt + 1);
        show_ipv4_rule(&gre_ipv4_rule[cnt], &gre_ipv4_result[cnt]);
    }
    printf("\n%15s\n","IPV6 Rules:");
    for (cnt = 0; cnt < gre_ipv6_rule_cnt; cnt++)
    {
        printf("\t\t%d. ", cnt + 1);
        show_ipv6_rule(&gre_ipv6_rule[cnt], &gre_ipv6_result[cnt]);
    }
    return CPARSER_OK;
}
cparser_result_t cparser_cmd_lst_gtp(cparser_context_t *context)
{
    uint32_t cnt;
    if((gtp_ipv4_rule_cnt + gtp_ipv6_rule_cnt) == 0)
    {
        printf("No Rule Specified\n");
    return CPARSER_OK;
    }
    printf("%25s : %d \n","gtp ipv4 rule count",gtp_ipv4_rule_cnt);
    printf("%25s : %d \n","gtp ipv6 rule count",gtp_ipv6_rule_cnt);

    printf("\n%15s\n","IPV4 Rules:");
    for (cnt = 0; cnt < gtp_ipv4_rule_cnt; cnt++)
    {
        printf("\t\t%d. ", cnt + 1);
        show_ipv4_rule(&gtp_ipv4_rule[cnt], &gtp_ipv4_result[cnt]);
    }
    printf("\n%15s\n","IPV6 Rules: \n");
    for (cnt = 0; cnt < gtp_ipv6_rule_cnt; cnt++)
    {
        printf("\t\t%d. ", cnt + 1);
        show_ipv6_rule(&gtp_ipv6_rule[cnt], &gtp_ipv6_result[cnt]);
    }
    return CPARSER_OK;
}

cparser_result_t cparser_cmd_lst_ftp(cparser_context_t *context)
{
    uint32_t cnt;
    if((ftp_ipv4_rule_cnt + ftp_ipv6_rule_cnt) == 0)
    {
        printf("No Rule Specified\n");
    return CPARSER_OK;
    }
    printf("%25s : %d \n","ftp ipv4 rule count",ftp_ipv4_rule_cnt);
    printf("%25s : %d \n","ftp ipv6 rule count",ftp_ipv6_rule_cnt);

    printf("\n%15s\n"," IPV4 Rules:");
    for (cnt = 0; cnt < ftp_ipv4_rule_cnt; cnt++)
    {
        printf("\t\t%d. ", cnt + 1);
        show_ipv4_rule(&ftp_ipv4_rule[cnt], &ftp_ipv4_result[cnt]);
    }
    printf("\n%15s\n","IPV6 Rules:");
    for (cnt = 0; cnt < ftp_ipv6_rule_cnt; cnt++)
    {
        printf("\t\t%d. ", cnt + 1);
        show_ipv6_rule(&ftp_ipv6_rule[cnt], &ftp_ipv6_result[cnt]);
    }
    return CPARSER_OK;
}
cparser_result_t cparser_cmd_lst_sctp(cparser_context_t *context)
{
    uint32_t cnt;
    if((sctp_ipv4_rule_cnt + sctp_ipv6_rule_cnt) == 0)
    {
        printf("No Rule Specified\n");
    return CPARSER_OK;
    }
    printf("%25s : %d \n","sctp ipv4 rule count",sctp_ipv4_rule_cnt);
    printf("%25s : %d \n","sctp ipv6 rule count",sctp_ipv6_rule_cnt);

    printf("\n%15s\n","IPV4 Rules:");
    for (cnt = 0; cnt < sctp_ipv4_rule_cnt; cnt++)
    {
        printf("\t\t%d. ", cnt + 1);
        show_ipv4_rule(&sctp_ipv4_rule[cnt], &sctp_ipv4_result[cnt]);
    }
    printf("\n%15s\n","IPV6 Rules:");
    for (cnt = 0; cnt < sctp_ipv6_rule_cnt; cnt++)
    {
        printf("\t\t%d. ", cnt + 1);
        show_ipv6_rule(&sctp_ipv6_rule[cnt], &sctp_ipv6_result[cnt]);
    }
    return CPARSER_OK;
}
cparser_result_t cparser_cmd_lst_smtp(cparser_context_t *context)
{
    uint32_t cnt;
    if((smtp_ipv4_rule_cnt + smtp_ipv6_rule_cnt) == 0)
    {
        printf("No Rule Specified\n");
    return CPARSER_OK;
    }
    printf("%25s : %d \n","smtp ipv4 rule count ",smtp_ipv4_rule_cnt);
    printf("%25s : %d \n","smtp ipv6 rule count ",smtp_ipv6_rule_cnt);

    printf("\n%15s\n","IPV4 Rules:");
    for (cnt = 0; cnt < smtp_ipv4_rule_cnt; cnt++)
    {
        printf("\t\t%d. ", cnt + 1);
        show_ipv4_rule(&smtp_ipv4_rule[cnt], &smtp_ipv4_result[cnt]);
    }
    printf("\n%15s\n","IPV6 Rules:");
    for (cnt = 0; cnt < smtp_ipv6_rule_cnt; cnt++)
    {
        printf("\t\t%d. ", cnt + 1);
        show_ipv6_rule(&smtp_ipv6_rule[cnt], &smtp_ipv6_result[cnt]);
    }
    return CPARSER_OK;
}
cparser_result_t cparser_cmd_lst_dns(cparser_context_t *context)
{
    uint32_t cnt;
    if((dns_ipv4_rule_cnt + dns_ipv6_rule_cnt) == 0)
    {
        printf("No Rule Specified\n");
    return CPARSER_OK;
    }
    printf("%25s : %d \n","dns ipv4 rule count",dns_ipv4_rule_cnt);
    printf("%25s : %d \n","dns ipv6 rule count",dns_ipv6_rule_cnt);

    printf("\n%15s\n","IPV4 Rules: \n");
    for (cnt = 0; cnt < dns_ipv4_rule_cnt; cnt++)
    {
        printf("\t\t%d. ", cnt + 1);
        show_ipv4_rule(&dns_ipv4_rule[cnt], &dns_ipv4_result[cnt]);
    }
    printf("\n%15s\n","IPV6 Rules: \n");
    for (cnt = 0; cnt < dns_ipv6_rule_cnt; cnt++)
    {
        printf("\t\t%d. ", cnt + 1);
        show_ipv6_rule(&dns_ipv6_rule[cnt], &dns_ipv6_result[cnt]);
    }
    return CPARSER_OK;
}
cparser_result_t cparser_cmd_lst_smb(cparser_context_t *context)
{
    uint32_t cnt;
    if((smb_ipv4_rule_cnt + smb_ipv6_rule_cnt) == 0)
    {
        printf("No Rule Specified\n");
    return CPARSER_OK;
    }
    printf("%25s : %d","smb ipv4 rule count",smb_ipv4_rule_cnt);
    printf("%25s : %d ","smb ipv6 rule count",smb_ipv6_rule_cnt);

    printf("\n%15s\n","IPV4 Rules: \n");
    for (cnt = 0; cnt < smb_ipv4_rule_cnt; cnt++)
    {
        printf("\t\t%d. ", cnt + 1);
        show_ipv4_rule(&smb_ipv4_rule[cnt], &smb_ipv4_result[cnt]);
    }
    printf("\n%15s\n","IPV6 Rules: \n");
    for (cnt = 0; cnt < smb_ipv6_rule_cnt; cnt++)
    {
        printf("\t\t%d. ", cnt + 1);
        show_ipv6_rule(&smb_ipv6_rule[cnt], &smb_ipv6_result[cnt]);
    }
    return CPARSER_OK;
}
cparser_result_t cparser_cmd_lst_smb2(cparser_context_t *context)
{
    uint32_t cnt;
    if((smb2_ipv4_rule_cnt + smb2_ipv6_rule_cnt) == 0)
    {
        printf("No Rule Specified\n");
    return CPARSER_OK;
    }
    printf("%25s : %d \n","smb2 ipv4 rule count",smb2_ipv4_rule_cnt);
    printf("%25s : %d \n","smb2 ipv6 rule count",smb2_ipv6_rule_cnt);

    printf("\n%15s\n","IPV4 Rules: \n");
    for (cnt = 0; cnt < smb2_ipv4_rule_cnt; cnt++)
    {
        printf("\t\t%d. ", cnt + 1);
        show_ipv4_rule(&smb2_ipv4_rule[cnt], &smb2_ipv4_result[cnt]);
    }
    printf("\n%15s\n","IPV6 Rules: \n");
    for (cnt = 0; cnt < smb2_ipv6_rule_cnt; cnt++)
    {
        printf("\t\t%d. ", cnt + 1);
        show_ipv6_rule(&smb2_ipv6_rule[cnt], &smb2_ipv6_result[cnt]);
    }
    return CPARSER_OK;
}
cparser_result_t cparser_cmd_lst_dcerpc(cparser_context_t *context)
{
    uint32_t cnt;
    if((dcerpc_ipv4_rule_cnt + dcerpc_ipv6_rule_cnt) == 0)
    {
        printf("No Rule Specified\n");
    return CPARSER_OK;
    }
    printf("%25s : %d \n","dcerpc ipv4 rule count",dcerpc_ipv4_rule_cnt);
    printf("%25s : %d \n","dcerpc ipv6 rule count",dcerpc_ipv6_rule_cnt);

    printf("\n%15s\n","IPV4 Rules: \n");
    for (cnt = 0; cnt < dcerpc_ipv4_rule_cnt; cnt++)
    {
        printf("\t\t%d. ", cnt + 1);
        show_ipv4_rule(&dcerpc_ipv4_rule[cnt], &dcerpc_ipv4_result[cnt]);
    }
    printf("\n%15s\n","IPV6 Rules: \n");
    for (cnt = 0; cnt < dcerpc_ipv6_rule_cnt; cnt++)
    {
        printf("\t\t%d. ", cnt + 1);
        show_ipv6_rule(&dcerpc_ipv6_rule[cnt], &dcerpc_ipv6_result[cnt]);
    }
    return CPARSER_OK;
}

cparser_result_t cparser_cmd_lst_clr(cparser_context_t *context)
{
        context= NULL;
        printf("\e[1;1H\e[2J");
        printf("\n");
        return CPARSER_OK;
}
cparser_result_t cparser_cmd_lst_exit(cparser_context_t *context)
{
        return cparser_submode_exit(context->parser);
}
 cparser_result_t cparser_cmd_rule_del_version_proto_rule_num(cparser_context_t *context,
     char **version_ptr,
     char **proto_ptr,
     uint32_t *rule_num_ptr)
{
	if( *rule_num_ptr == 0)
	{
		printf("Give a valid number \n");
	}
	else if (!strcmp(*version_ptr, "ipv4"))
	{
		del_ipv4_rule(*proto_ptr, *rule_num_ptr);
	}
  else
  {
		del_ipv6_rule(*proto_ptr, *rule_num_ptr);
  }
    return CPARSER_OK;
}
cparser_result_t cparser_cmd_rule_clr(cparser_context_t *context)
{
        context= NULL;
        printf("\e[1;1H\e[2J");
        printf("\n");
        return CPARSER_OK;
}
cparser_result_t cparser_cmd_rule_exit(cparser_context_t *context)
{
        return cparser_submode_exit(context->parser);
}

cparser_result_t cparser_cmd_cnfg_mode_mode(cparser_context_t *context,
     char **mode_ptr)
{ 
        strcpy(opmode,*mode_ptr);
        printf("%s mode Configred\n", *mode_ptr);
        return CPARSER_OK;
}
cparser_result_t cparser_cmd_cnfg_port_ingress_port(cparser_context_t *context,
     char **ingress_ptr,
     uint32_t *port_ptr)
{
	if(!strcmp(*ingress_ptr, "0"))
	{
		if(*port_ptr == 0)
		{
			pmap_p0 = 0;
			printf("Port 0 Mapped to Port 0\n");
		}
		else if(*port_ptr == 1)
		{
			pmap_p0 = 1;
			printf("Port 0 Mapped to Port 1\n");
		}
		else if(*port_ptr == 2)
		{
			pmap_p0 = 2;
			printf("Port 0 Mapped to Port 2\n");
		}
		else if(*port_ptr == 3)
		{
			pmap_p0 = 3;
			printf("Port 0 Mapped to Port 3\n");
		}
		else if(*port_ptr == 255)
		{
			pmap_p0 = 255;
			printf("Packets are dropped\n");
		}
		else
		{
			printf("Wrong Entry\n");
		}
	}
	else if(!strcmp(*ingress_ptr, "1"))
	{
		if(*port_ptr == 0)
		{
			pmap_p1 = 0;
			printf("Port 1 Mapped to Port 0\n");
		}
		else if(*port_ptr == 1)
		{
			pmap_p1 = 1;
			printf("Port 1 Mapped to Port 1\n");
		}
		else if(*port_ptr == 2)
		{
			pmap_p1 = 2;
			printf("Port 1 Mapped to Port 2\n");
		}
		else if(*port_ptr == 3)
		{
			pmap_p1 = 3;
			printf("Port 1 Mapped to Port 3\n");
		}
		else if(*port_ptr == 255)
		{
			pmap_p1 = 255;
			printf("Packets are dropped\n");
		}
		else
		{
			printf("Wrong Entry\n");
		}
	}
	else if(!strcmp(*ingress_ptr, "2"))
	{
		if(*port_ptr == 0)
		{
			pmap_p2 = 0;
			printf("Port 2 Mapped to Port 0\n");
		}
		else if(*port_ptr == 1)
		{
			pmap_p2 = 1;
			printf("Port 2 Mapped to Port 1\n");
		}
		else if(*port_ptr == 2)
		{
			pmap_p2 = 2;
			printf("Port 2 Mapped to Port 2\n");
		}
		else if(*port_ptr == 3)
		{
			pmap_p2 = 3;
			printf("Port 2 Mapped to Port 3\n");
		}
		else if(*port_ptr == 255)
		{
			pmap_p2 = 255;
			printf("Packets are dropped\n");
		}
		else
		{
			printf("Wrong Entry\n");
		}
	}
	else if(!strcmp(*ingress_ptr, "3"))
	{
		if(*port_ptr == 0)
		{
			pmap_p3 = 0;
			printf("Port 3 Mapped to Port 0\n");
		}
		else if(*port_ptr == 1)
		{
			pmap_p3 = 1;
			printf("Port 3 Mapped to Port 1\n");
		}
		else if(*port_ptr == 2)
		{
			pmap_p3 = 2;
			printf("Port 3 Mapped to Port 2\n");
		}
		else if(*port_ptr == 3)
		{
			pmap_p3 = 3;
			printf("Port 3 Mapped to Port 3\n");
		}
		else if(*port_ptr == 255)
		{
			pmap_p3 = 255;
			printf("Packets are dropped\n");
		}
		else
		{
			printf("Wrong Entry\n");
		}
	}
        return CPARSER_OK;
}


 cparser_result_t cparser_cmd_cnfg_pact(cparser_context_t *context)
{
    int8_t prompt[30]={0};
    snprintf((char*)prompt, CPARSER_MAX_PROMPT, "%s-%s> ", "CNFG","PACT");
    return cparser_ssli_submode_enter(context->parser, prompt);
}

cparser_result_t cparser_cmd_pact_port1(cparser_context_t *context)
{
    int8_t prompt[30]={0};
    snprintf((char*)prompt, CPARSER_MAX_PROMPT, "%s-%s-%s> ", "CNFG","PACT","PRT1");
    return cparser_ssli_submode_enter(context->parser, prompt);
}
cparser_result_t cparser_cmd_port1_fdrp_fdrp(cparser_context_t *context,
    uint32_t *fdrp_ptr)
{
    fdrp_p1 = *fdrp_ptr;
    if(fdrp_p1 == 1)
    {
        printf("Fragment drop enabled\n");
    }
    else if(!fdrp_p1)
    {
        printf("Fragment drop disabled\n");
    }
    else
    {
        printf("Wrong Entry\n");
    }
    return CPARSER_OK;
}
cparser_result_t cparser_cmd_port1_drop_drp(cparser_context_t *context,
    uint32_t *drp_ptr)
{
    drp_p1 = *drp_ptr;
    if(drp_p1 == 1)
    {
        printf("Drop Packet enabled\n");
    }
    else if(!drp_p1)
    {
        printf("Drop packet disabled\n");
    }
    else
    {
        printf("Wrong Entry\n");
    }
    return CPARSER_OK;
}
cparser_result_t cparser_cmd_port1_acl_acl(cparser_context_t *context,
    uint32_t *acl_ptr)
{
    acl_p1 = *acl_ptr;
    if(acl_p1 == 1)
    {
        printf("ACL operation enabled\n");
    }
    else if(!acl_p1)
    {
        printf("ACL operation disabled\n");
    }
    else
    {
        printf("Wrong Entry\n");
    }
    return CPARSER_OK;
}
cparser_result_t cparser_cmd_port1_st_st(cparser_context_t *context,
    uint32_t *st_ptr)
{
    st_p1 = *st_ptr;
    if(st_p1 == 1)
    {
        printf("ST enabled\n");
    }
    else if(!st_p1)
    {
        printf("ST disabled\n");
    }
    else
    {
        printf("Wrong Entry\n");
    }
    return CPARSER_OK;
}
cparser_result_t cparser_cmd_port1_clr(cparser_context_t *context)
{
        context= NULL;
        printf("\e[1;1H\e[2J");
        printf("\n");
        return CPARSER_OK;
}
cparser_result_t cparser_cmd_port1_exit(cparser_context_t *context)
{
        return cparser_submode_exit(context->parser);
}
cparser_result_t cparser_cmd_pact_port2(cparser_context_t *context)
{
    int8_t prompt[30]={0};
    snprintf((char*)prompt, CPARSER_MAX_PROMPT, "%s-%s-%s> ", "CNFG","PACT","PRT2");
    return cparser_ssli_submode_enter(context->parser, prompt);
}
cparser_result_t cparser_cmd_port2_fdrp_fdrp(cparser_context_t *context,
    uint32_t *fdrp_ptr)
{
    fdrp_p2 = *fdrp_ptr;
    if(fdrp_p2 == 1)
    {
        printf("Fragment drop enabled\n");
    }
    else if(!fdrp_p2)
    {
        printf("Fragment drop disabled\n");
    }
    else
    {
        printf("Wrong Entry\n");
    }
    return CPARSER_OK;
}
cparser_result_t cparser_cmd_port2_drop_drp(cparser_context_t *context,
    uint32_t *drp_ptr)
{
    drp_p2 = *drp_ptr;
    if(drp_p2 == 1)
    {
        printf("Drop Packet enabled\n");
    }
    else if(!drp_p2)
    {
        printf("Drop packet disabled\n");
    }
    else
    {
        printf("Wrong Entry\n");
    }
    return CPARSER_OK;
}
cparser_result_t cparser_cmd_port2_acl_acl(cparser_context_t *context,
    uint32_t *acl_ptr)
{
    acl_p2 = *acl_ptr;
    if(acl_p2 == 1)
    {
        printf("ACL operation  enabled\n");
    }
    else if(!acl_p2)
    {
        printf("ACL operation  disabled\n");
    }
    else
    {
        printf("Wrong Entry\n");
    }
    return CPARSER_OK;
}
cparser_result_t cparser_cmd_port2_st_st(cparser_context_t *context,
    uint32_t *st_ptr)
{
    st_p2 = *st_ptr;
    if(st_p2 == 1)
    {
        printf("ST enabled\n");
    }
    else if(!st_p2)
    {
        printf("ST disabled\n");
    }
    else
    {
        printf("Wrong Entry\n");
    }
    return CPARSER_OK;
}
cparser_result_t cparser_cmd_port2_clr(cparser_context_t *context)
{
        context= NULL;
        printf("\e[1;1H\e[2J");
        printf("\n");
        return CPARSER_OK;
}
cparser_result_t cparser_cmd_port2_exit(cparser_context_t *context)
{
        return cparser_submode_exit(context->parser);
}
cparser_result_t cparser_cmd_pact_port3(cparser_context_t *context)
{
    int8_t prompt[30]={0};
    snprintf((char*)prompt, CPARSER_MAX_PROMPT, "%s-%s-%s> ", "CNFG","PACT","PRT3");
    return cparser_ssli_submode_enter(context->parser, prompt);
}
cparser_result_t cparser_cmd_port3_fdrp_fdrp(cparser_context_t *context,
    uint32_t *fdrp_ptr)
{
    fdrp_p3 = *fdrp_ptr;
    if(fdrp_p3 == 1)
    {
        printf("Fragment drop enabled\n");
    }
    else if(!fdrp_p3)
    {
        printf("Fragment drop disabled\n");
    }
    else
    {
        printf("Wrong Entry\n");
    }
    return CPARSER_OK;
}
cparser_result_t cparser_cmd_port3_drop_drp(cparser_context_t *context,
    uint32_t *drp_ptr)
{
    drp_p3 = *drp_ptr;
    if(drp_p3 == 1)
    {
        printf("Drop Packet enabled\n");
    }
    else if(!drp_p3)
    {
        printf("Drop packet disabled\n");
    }
    else
    {
        printf("Wrong Entry\n");
    }
    return CPARSER_OK;
}
cparser_result_t cparser_cmd_port3_acl_acl(cparser_context_t *context,
    uint32_t *acl_ptr)
{
    acl_p3 = *acl_ptr;
    if(acl_p3 == 1)
    {
        printf("ACL operation enabled\n");
    }
    else if(!acl_p3)
    {
        printf("ACL operation disabled\n");
    }
    else
    {
        printf("Wrong Entry\n");
    }
    return CPARSER_OK;
}
cparser_result_t cparser_cmd_port3_st_st(cparser_context_t *context,
    uint32_t *st_ptr)
{
    st_p3 = *st_ptr;
    if(st_p3 == 1)
    {
        printf("ST enabled\n");
    }
    else if(!st_p3)
    {
        printf("ST disabled\n");
    }
    else
    {
        printf("Wrong Entry\n");
    }
    return CPARSER_OK;
}
cparser_result_t cparser_cmd_port3_clr(cparser_context_t *context)
{
        context= NULL;
        printf("\e[1;1H\e[2J");
        printf("\n");
        return CPARSER_OK;
}
cparser_result_t cparser_cmd_port3_exit(cparser_context_t *context)
{
        return cparser_submode_exit(context->parser);
}
cparser_result_t cparser_cmd_pact_port4(cparser_context_t *context)
{
    int8_t prompt[30]={0};
    snprintf((char*)prompt, CPARSER_MAX_PROMPT, "%s-%s-%s> ", "CNFG","PACT","PRT4");
    return cparser_ssli_submode_enter(context->parser, prompt);
}
cparser_result_t cparser_cmd_port4_fdrp_fdrp(cparser_context_t *context,
    uint32_t *fdrp_ptr)
{
    fdrp_p4 = *fdrp_ptr;
    if(fdrp_p4 == 1)
    {
        printf("Fragment drop enabled\n");
    }
    else if(!fdrp_p4)
    {
        printf("Fragment drop disabled\n");
    }
    else
    {
        printf("Wrong Entry\n");
    }
    return CPARSER_OK;
}
cparser_result_t cparser_cmd_port4_drop_drp(cparser_context_t *context,
    uint32_t *drp_ptr)
{
    drp_p4 = *drp_ptr;
    if(drp_p4 == 1)
    {
        printf("Drop Packet enabled\n");
    }
    else if(!drp_p4)
    {
        printf("Drop packet disabled\n");
    }
    else
    {
        printf("Wrong Entry\n");
    }
    return CPARSER_OK;
}
cparser_result_t cparser_cmd_port4_acl_acl(cparser_context_t *context,
    uint32_t *acl_ptr)
{
    acl_p4 = *acl_ptr;
    if(acl_p4 == 1)
    {
        printf("ACL operation enabled\n");
    }
    else if(!acl_p4)
    {
        printf("ACL operation disabled\n");
    }
    else
    {
        printf("Wrong Entry\n");
    }
    return CPARSER_OK;
}
cparser_result_t cparser_cmd_port4_st_st(cparser_context_t *context,
    uint32_t *st_ptr)
{
    st_p4 = *st_ptr;
    if(st_p4 == 1)
    {
        printf("ST enabled\n");
    }
    else if(!st_p4)
    {
        printf("ST disabled\n");
    }
    else
    {
        printf("Wrong Entry\n");
    }
    return CPARSER_OK;
}
cparser_result_t cparser_cmd_port4_clr(cparser_context_t *context)
{
        context= NULL;
        printf("\e[1;1H\e[2J");
        printf("\n");
        return CPARSER_OK;
}
cparser_result_t cparser_cmd_port4_exit(cparser_context_t *context)
{
        return cparser_submode_exit(context->parser);
}
cparser_result_t cparser_cmd_pact_clr(cparser_context_t *context)
{
        context= NULL;
        printf("\e[1;1H\e[2J");
        printf("\n");
        return CPARSER_OK;
}

  cparser_result_t cparser_cmd_pact_exit(cparser_context_t *context)
{
        return cparser_submode_exit(context->parser);
}
  cparser_result_t cparser_cmd_port_clr(cparser_context_t *context)
{
        context= NULL;
        printf("\e[1;1H\e[2J");
        printf("\n");
        return CPARSER_OK;
}
  cparser_result_t cparser_cmd_port_exit(cparser_context_t *context)
{
        return cparser_submode_exit(context->parser);
}
/*
cparser_result_t cparser_cmd_pmap_port0_port(cparser_context_t *context,
    uint32_t *port_ptr)
{
    pmap_p0 = *port_ptr;
    if(pmap_p0 == 0)
    {
	printf("port 0 directed to port 0\n");
    }
    else if(pmap_p0 == 1)
    {
	printf("port 0 directed to port 1\n");
    }
    else if(pmap_p0 == 2)
    {
	printf("port 0 directed to port 2\n");
    }
    else if(pmap_p0 == 3)
    {
	printf("port 0 directed to port 3\n");
    }
    else
    {
	printf("Wrong Entry\n");
    }

    return CPARSER_OK;
}
cparser_result_t cparser_cmd_pmap_port1_port(cparser_context_t *context,
    uint32_t *port_ptr)
{
    pmap_p1 = *port_ptr;
    if(pmap_p1 == 0)
    {
	printf("port 1 directed to port 0\n");
    }
    else if(pmap_p1 == 1)
    {
	printf("port 1 directed to port 1\n");
    }
    else if(pmap_p1 == 2)
    {
	printf("port 1 directed to port 2\n");
    }
    else if(pmap_p1 == 3)
    {
	printf("port 1 directed to port 3\n");
    }
    else
    {
	printf("Wrong Entry\n");
    }
    return CPARSER_OK;
}
cparser_result_t cparser_cmd_pmap_port2_port(cparser_context_t *context,
    uint32_t *port_ptr)
{
    pmap_p2 = *port_ptr;
    if(pmap_p2 == 0)
    {
	printf("port 2 directed to port 0\n");
    }
    else if(pmap_p2 == 1)
    {
	printf("port 2 directed to port 1\n");
    }
    else if(pmap_p2 == 2)
    {
	printf("port 2 directed to port 2\n");
    }
    else if(pmap_p2 == 3)
    {
	printf("port 2 directed to port 3\n");
    }
    else
    {
	printf("Wrong Entry\n");
    }
    return CPARSER_OK;
}
cparser_result_t cparser_cmd_pmap_port3_port(cparser_context_t *context,
    uint32_t *port_ptr)
{
    pmap_p3 = *port_ptr;
    if(pmap_p3 == 0)
    {
	printf("port 3 directed to port 0\n");
    }
    else if(pmap_p3 == 1)
    {
	printf("port 3 directed to port 1\n");
    }
    else if(pmap_p3 == 2)
    {
	printf("port 3 directed to port 2\n");
    }
    else if(pmap_p3 == 3)
    {
	printf("port 3 directed to port 3\n");
    }
    else
    {
	printf("Wrong Entry\n");
    }
    return CPARSER_OK;
}
cparser_result_t cparser_cmd_pmap_clr(cparser_context_t *context)
{
        context = NULL;
        printf("\e[1;1H\e[2J");
        printf("\n");
        return CPARSER_OK;
}
cparser_result_t cparser_cmd_pmap_exit(cparser_context_t *context)

{
        return cparser_submode_exit(context->parser);
}
*/

cparser_result_t cparser_cmd_cnfg_clr(cparser_context_t *context)
{
        context = NULL;
        printf("\e[1;1H\e[2J");
        printf("\n");
        return CPARSER_OK;
}
cparser_result_t cparser_cmd_cnfg_exit(cparser_context_t *context)
{
        return cparser_submode_exit(context->parser);
}

cparser_result_t cparser_cmd_show(cparser_context_t *context)
{
        int8_t prompt[30]={0};
        snprintf((char*)prompt, CPARSER_MAX_PROMPT, "%s> ", "SHW");
        return cparser_ssli_submode_enter(context->parser, prompt);
}
cparser_result_t cparser_cmd_show_rule(cparser_context_t *context)
{ 
        int8_t prompt[30]={0};
        snprintf((char*)prompt, CPARSER_MAX_PROMPT, "%s-%s> ", "SHW","RULE");
        return cparser_ssli_submode_enter(context->parser, prompt);
}

cparser_result_t cparser_cmd_rule1_info(cparser_context_t *context)
{
     get_info();
     return CPARSER_OK;
}
cparser_result_t cparser_cmd_rule1_dtls(cparser_context_t *context)
{
        int8_t prompt[30]={0};
        snprintf((char*)prompt, CPARSER_MAX_PROMPT, "%s-%s-%s> ", "SHW","RULE","DETLS");
        return cparser_ssli_submode_enter(context->parser, prompt);
}
cparser_result_t cparser_cmd_details_tcp(cparser_context_t *context)
{
   uint32_t cnt;

    if((tcp_ipv4_rule_cnt + tcp_ipv6_rule_cnt) == 0)
    {
        printf("No Rule Specified\n");
    return CPARSER_OK;
    }
    printf("\n%15s\n","IPV4 Rules:");
   for (cnt = 0; cnt < tcp_ipv4_rule_cnt; cnt++)
   {
            printf("\t\t%d. ", cnt + 1);
            show_ipv4_rule(&tcp_ipv4_rule[cnt], &tcp_ipv4_result[cnt]);
    }
    printf("\n%15s\n","IPV6 Rules:");
   for (cnt = 0; cnt < tcp_ipv6_rule_cnt; cnt++)
   {
            printf("\t\t%d. ", cnt + 1);
            show_ipv6_rule(&tcp_ipv6_rule[cnt], &tcp_ipv6_result[cnt]);
    }
    return CPARSER_OK;
}
cparser_result_t cparser_cmd_details_udp(cparser_context_t *context)
{
   uint32_t cnt;

    if((udp_ipv4_rule_cnt + udp_ipv6_rule_cnt) == 0)
    {
        printf("No Rule Specified\n");
    return CPARSER_OK;
    }
    printf("\n%15s\n","IPV4 Rules:");
   for (cnt = 0; cnt < udp_ipv4_rule_cnt; cnt++)
   {
            printf("\t\t%d. ", cnt + 1);
            show_ipv4_rule(&udp_ipv4_rule[cnt], &udp_ipv4_result[cnt]);
    }
    printf("\n%15s\n","IPV6 Rules:");
   for (cnt = 0; cnt < udp_ipv6_rule_cnt; cnt++)
   {
            printf("\t\t%d. ", cnt + 1);
            show_ipv6_rule(&udp_ipv6_rule[cnt], &udp_ipv6_result[cnt]);
    }
    return CPARSER_OK;
}
cparser_result_t cparser_cmd_details_icmp(cparser_context_t *context)
{
   uint32_t cnt;

    if((icmp_ipv4_rule_cnt + icmp_ipv6_rule_cnt) == 0)
    {
        printf("No Rule Specified\n");
    return CPARSER_OK;
    }
    printf("\n%15s\n","IPV4 Rules:");
   for (cnt = 0; cnt < icmp_ipv4_rule_cnt; cnt++)
   {
            printf("\t\t%d. ", cnt + 1);
            show_ipv4_rule(&icmp_ipv4_rule[cnt], &icmp_ipv4_result[cnt]);
    }
    printf("\n%15s\n","IPV6 Rules:");
   for (cnt = 0; cnt < icmp_ipv6_rule_cnt; cnt++)
   {
            printf("\t\t%d. ", cnt + 1);
            show_ipv6_rule(&icmp_ipv6_rule[cnt], &icmp_ipv6_result[cnt]);
    }
    return CPARSER_OK;
}
cparser_result_t cparser_cmd_details_http(cparser_context_t *context)
{
   uint32_t cnt;

    if((http_ipv4_rule_cnt + http_ipv6_rule_cnt) == 0)
    {
        printf("No Rule Specified\n");
    return CPARSER_OK;
    }
    printf("\n%15s\n","IPV4 Rules:");
   for (cnt = 0; cnt < http_ipv4_rule_cnt; cnt++)
   {
            printf("\t\t%d. ", cnt + 1);
            show_ipv4_rule(&http_ipv4_rule[cnt], &http_ipv4_result[cnt]);
    }
    printf("\n%15s\n","IPV6 Rules:");
   for (cnt = 0; cnt < icmp_ipv6_rule_cnt; cnt++)
   {
            printf("\t\t%d. ", cnt + 1);
            show_ipv6_rule(&http_ipv6_rule[cnt], &http_ipv6_result[cnt]);
    }
    return CPARSER_OK;
}
cparser_result_t cparser_cmd_details_ssl(cparser_context_t *context)
{
    uint32_t cnt;

    if((ssl_ipv4_rule_cnt + ssl_ipv6_rule_cnt) == 0)
    {
        printf("No Rule Specified\n");
    return CPARSER_OK;
    }
    printf("\n%15s\n","IPV4 Rules:");
        for (cnt = 0; cnt < ssl_ipv4_rule_cnt; cnt++)
        {
            printf("\t\t%d. ", cnt + 1);
            show_ipv4_rule(&ssl_ipv4_rule[cnt], &ssl_ipv4_result[cnt]);
        }
    printf("\n%15s\n","IPV6 Rules:");
        for (cnt = 0; cnt < ssl_ipv6_rule_cnt; cnt++)
        {
            printf("\t\t%d. ", cnt + 1);
            show_ipv6_rule(&ssl_ipv6_rule[cnt], &ssl_ipv6_result[cnt]);
        }
    return CPARSER_OK;
}
cparser_result_t cparser_cmd_details_tls(cparser_context_t *context)
{
   uint32_t cnt;
    if((tls_ipv4_rule_cnt + tls_ipv6_rule_cnt) == 0)
    {
        printf("No Rule Specified\n");
    return CPARSER_OK;
    }
    printf("\n%15s\n","IPV4 Rules:");
   for (cnt = 0; cnt < tls_ipv4_rule_cnt; cnt++)
   {
        printf("\t\t%d. ", cnt + 1);
        show_ipv4_rule(&tls_ipv4_rule[cnt], &tls_ipv4_result[cnt]);
   }
    printf("\n%15s\n","IPV6 Rules:");
   for (cnt = 0; cnt < tls_ipv6_rule_cnt; cnt++)
   {
        printf("\t\t%d. ", cnt + 1);
        show_ipv6_rule(&tls_ipv6_rule[cnt], &tls_ipv6_result[cnt]);
   }
   return CPARSER_OK;
}
cparser_result_t cparser_cmd_details_gre(cparser_context_t *context)
{
   uint32_t cnt;

    if((gre_ipv4_rule_cnt + gre_ipv6_rule_cnt) == 0)
    {
        printf("No Rule Specified\n");
    return CPARSER_OK;
    }
    printf("\n%15s\n","IPV4 Rules:");
   for (cnt = 0; cnt < gre_ipv4_rule_cnt; cnt++)
   {
        printf("\t\t%d. ", cnt + 1);
        show_ipv4_rule(&gre_ipv4_rule[cnt], &gre_ipv4_result[cnt]);
   }
    printf("\n%15s\n","IPV6 Rules:");
   for (cnt = 0; cnt < gre_ipv6_rule_cnt; cnt++)
   {
        printf("\t\t%d. ", cnt + 1);
        show_ipv6_rule(&gre_ipv6_rule[cnt], &gre_ipv6_result[cnt]);
   }
   return CPARSER_OK;
}
cparser_result_t cparser_cmd_details_gtp(cparser_context_t *context)
{
   uint32_t cnt;

    if((gtp_ipv4_rule_cnt + gtp_ipv6_rule_cnt) == 0)
    {
        printf("No Rule Specified\n");
    return CPARSER_OK;
    }
    printf("\n%15s\n","IPV4 Rules:");
   for (cnt = 0; cnt < gre_ipv4_rule_cnt; cnt++)
   {
        printf("\t\t%d. ", cnt + 1);
        show_ipv4_rule(&gtp_ipv4_rule[cnt], &gtp_ipv4_result[cnt]);
   }
    printf("\n%15s\n","IPV6 Rules:");
   for (cnt = 0; cnt < gre_ipv6_rule_cnt; cnt++)
   {
        printf("\t\t%d. ", cnt + 1);
        show_ipv6_rule(&gtp_ipv6_rule[cnt], &gtp_ipv6_result[cnt]);
   }
   return CPARSER_OK;
}
cparser_result_t cparser_cmd_details_ftp(cparser_context_t *context)
{
   uint32_t cnt;

    if((ftp_ipv4_rule_cnt + ftp_ipv6_rule_cnt) == 0)
    {
        printf("No Rule Specified\n");
    return CPARSER_OK;
    }
    printf("\n%15s\n","IPV4 Rules:");
   for (cnt = 0; cnt < ftp_ipv4_rule_cnt; cnt++)
   {
        printf("\t\t%d. ", cnt + 1);
        show_ipv4_rule(&ftp_ipv4_rule[cnt], &ftp_ipv4_result[cnt]);
   }
    printf("\n%15s\n","IPV6 Rules:");
   for (cnt = 0; cnt < ftp_ipv6_rule_cnt; cnt++)
   {
        printf("\t\t%d. ", cnt + 1);
        show_ipv6_rule(&ftp_ipv6_rule[cnt], &ftp_ipv6_result[cnt]);
   }
   return CPARSER_OK;
}
cparser_result_t cparser_cmd_details_sctp(cparser_context_t *context)
{
   uint32_t cnt;

    if((sctp_ipv4_rule_cnt + sctp_ipv6_rule_cnt) == 0)
    {
        printf("No Rule Specified\n");
    return CPARSER_OK;
    }
    printf("\n%15s\n","IPV4 Rules:");
   for (cnt = 0; cnt < sctp_ipv4_rule_cnt; cnt++)
   {
        printf("\t\t%d. ", cnt + 1);
        show_ipv4_rule(&sctp_ipv4_rule[cnt], &sctp_ipv4_result[cnt]);
   }
    printf("\n%15s\n","IPV6 Rules:");
   for (cnt = 0; cnt < sctp_ipv6_rule_cnt; cnt++)
   {
        printf("\t\t%d. ", cnt + 1);
        show_ipv6_rule(&sctp_ipv6_rule[cnt], &sctp_ipv6_result[cnt]);
   }
   return CPARSER_OK;
}
cparser_result_t cparser_cmd_details_smtp(cparser_context_t *context)
{
   uint32_t cnt;

    if((smtp_ipv4_rule_cnt + smtp_ipv6_rule_cnt) == 0)
    {
        printf("No Rule Specified\n");
    return CPARSER_OK;
    }
    printf("\n%15s\n","IPV4 Rules:");
   for (cnt = 0; cnt < smtp_ipv4_rule_cnt; cnt++)
   {
        printf("\t\t%d. ", cnt + 1);
        show_ipv4_rule(&smtp_ipv4_rule[cnt], &smtp_ipv4_result[cnt]);
   }
    printf("\n%15s\n","IPV6 Rules:");
   for (cnt = 0; cnt < smtp_ipv6_rule_cnt; cnt++)
   {
        printf("\t\t%d. ", cnt + 1);
        show_ipv6_rule(&smtp_ipv6_rule[cnt], &smtp_ipv6_result[cnt]);
   }
   return CPARSER_OK;
}
cparser_result_t cparser_cmd_details_dns(cparser_context_t *context)
{
   uint32_t cnt;

    if((dns_ipv4_rule_cnt + dns_ipv6_rule_cnt) == 0)
    {
        printf("No Rule Specified\n");
    return CPARSER_OK;
    }
    printf("\n%15s\n","IPV4 Rules:");
   for (cnt = 0; cnt < dns_ipv4_rule_cnt; cnt++)
   {
        printf("\t\t%d. ", cnt + 1);
        show_ipv4_rule(&dns_ipv4_rule[cnt], &dns_ipv4_result[cnt]);
   }
    printf("\n%15s\n","IPV6 Rules:");
   for (cnt = 0; cnt < dns_ipv6_rule_cnt; cnt++)
   {
        printf("\t\t%d. ", cnt + 1);
        show_ipv6_rule(&dns_ipv6_rule[cnt], &dns_ipv6_result[cnt]);
   }
   return CPARSER_OK;
}
cparser_result_t cparser_cmd_details_smb(cparser_context_t *context)
{
   uint32_t cnt;

    if((smb_ipv4_rule_cnt + smb_ipv6_rule_cnt) == 0)
    {
        printf("No Rule Specified\n");
    return CPARSER_OK;
    }
    printf("\n%15s\n","IPV4 Rules:");
   for (cnt = 0; cnt < smb_ipv4_rule_cnt; cnt++)
   {
        printf("\t\t%d. ", cnt + 1);
        show_ipv4_rule(&smb_ipv4_rule[cnt], &smb_ipv4_result[cnt]);
   }
    printf("\n%15s\n","IPV6 Rules:");
   for (cnt = 0; cnt < smb_ipv6_rule_cnt; cnt++)
   {
        printf("\t\t%d. ", cnt + 1);
        show_ipv6_rule(&smb_ipv6_rule[cnt], &smb_ipv6_result[cnt]);
   }
   return CPARSER_OK;
}
cparser_result_t cparser_cmd_details_smb2(cparser_context_t *context)
{
   uint32_t cnt;
 if((smb2_ipv4_rule_cnt + smb2_ipv6_rule_cnt) == 0)
    {
        printf("No Rule Specified\n");
    return CPARSER_OK;
    }
    printf("\n%15s\n","IPV4 Rules:");
   for (cnt = 0; cnt < smb2_ipv4_rule_cnt; cnt++)
   {
        printf("\t\t%d. ", cnt + 1);
        show_ipv4_rule(&smb2_ipv4_rule[cnt], &smb2_ipv4_result[cnt]);
   }
    printf("\n%15s\n","IPV6 Rules:");
   for (cnt = 0; cnt < smb2_ipv6_rule_cnt; cnt++)
   {
        printf("\t\t%d. ", cnt + 1);
        show_ipv6_rule(&smb2_ipv6_rule[cnt], &smb2_ipv6_result[cnt]);
   }

   return CPARSER_OK;
   
}
cparser_result_t cparser_cmd_details_dcerpc(cparser_context_t *context)
{
   uint32_t cnt;

    if((dcerpc_ipv4_rule_cnt + dcerpc_ipv6_rule_cnt) == 0)
    {
        printf("No Rule Specified\n");
    return CPARSER_OK;
    }
    printf("\n%15s\n","IPV4 Rules:");
   for (cnt = 0; cnt < dcerpc_ipv4_rule_cnt; cnt++)
   {
        printf("\t\t%d. ", cnt + 1);
        show_ipv4_rule(&dcerpc_ipv4_rule[cnt], &dcerpc_ipv4_result[cnt]);
   }
    printf("\n%15s\n","IPV6 Rules:");
   for (cnt = 0; cnt < dcerpc_ipv6_rule_cnt; cnt++)
   {
        printf("\t\t%d. ", cnt + 1);
        show_ipv6_rule(&dcerpc_ipv6_rule[cnt], &dcerpc_ipv6_result[cnt]);
   }
   return CPARSER_OK;
}
cparser_result_t cparser_cmd_details_clr(cparser_context_t *context)
{
        context = NULL;
        printf("\e[1;1H\e[2J");
        printf("\n");
        return CPARSER_OK;
}
cparser_result_t cparser_cmd_details_exit(cparser_context_t *context)
{
        return cparser_submode_exit(context->parser);
}
cparser_result_t cparser_cmd_rule1_clr(cparser_context_t *context)
{
        context = NULL;
        printf("\e[1;1H\e[2J");
        printf("\n");
        return CPARSER_OK;
}
cparser_result_t cparser_cmd_rule1_exit(cparser_context_t *context)
{
        return cparser_submode_exit(context->parser);
}
cparser_result_t cparser_cmd_show_mode(cparser_context_t *context)
{
    if(!strcmp(opmode, "bypass"))
    {
        printf("\n %29s : %s \n","Current Mode", opmode);
    }
    else if(!strcmp(opmode, "intercept"))
    {
        printf("%29s : %s \n","Current Mode", opmode);
	 //     printf("Default action : %s\n", default_dir);
    }
    else
    {
       printf("\nMode Not Configured\n");
    }
       printf("------------ Core info ---------------\n");
       show_core_info(opmode);
        return CPARSER_OK;
}
cparser_result_t cparser_cmd_show_port(cparser_context_t *context)
{
       printf("\nPort Map info:\n");
       show_pmap();
        return CPARSER_OK;
}
cparser_result_t cparser_cmd_show_pact(cparser_context_t *context)
{
     show_pact_info();   
     return CPARSER_OK;
	
}
cparser_result_t cparser_cmd_show_intf(cparser_context_t *context)
{
    int8_t prompt[30]={0};
    snprintf((char*)prompt, CPARSER_MAX_PROMPT, "%s-%s> ", "SHW","INTF");
    return cparser_ssli_submode_enter(context->parser, prompt);
}
cparser_result_t cparser_cmd_intf_info_intf(cparser_context_t *context,
    uint32_t *intf_ptr)
{
   get_intf_info( *intf_ptr);
   return CPARSER_OK;
}
cparser_result_t cparser_cmd_intf_stat_num(cparser_context_t *context,
    uint32_t *num_ptr)
{
     show_interface_stats( *num_ptr);
     return CPARSER_OK;
}
cparser_result_t cparser_cmd_intf_clr(cparser_context_t *context)
{        
        context = NULL;
        printf("\e[1;1H\e[2J");
        printf("\n");
        return CPARSER_OK;
}
cparser_result_t cparser_cmd_intf_exit(cparser_context_t *context)
{
        return cparser_submode_exit(context->parser);
}
cparser_result_t cparser_cmd_show_stat(cparser_context_t *context)
{
    int8_t prompt[30]={0};
    snprintf((char*)prompt, CPARSER_MAX_PROMPT, "%s-%s> ", "SHW","STAT");
    return cparser_ssli_submode_enter(context->parser, prompt);
}

cparser_result_t cparser_cmd_stat_app(cparser_context_t *context)
{
        int8_t prompt[30]={0};
        snprintf((char*)prompt, CPARSER_MAX_PROMPT, "%s-%s-%s> ", "SHW","STAT","APP");
        return cparser_ssli_submode_enter(context->parser, prompt);
}
cparser_result_t cparser_cmd_cont_flow(cparser_context_t *context)
{

    show_stats(STATUS_FLOW);
    return CPARSER_OK; 
}
cparser_result_t cparser_cmd_cont_dcdr(cparser_context_t *context)
{
    show_stats(STATUS_DECODER);
    return CPARSER_OK; 
}
cparser_result_t cparser_cmd_cont_tcp(cparser_context_t *context)
{
    show_stats(STATUS_TCP);
    return CPARSER_OK; 
}
cparser_result_t cparser_cmd_cont_dfrg(cparser_context_t *context)
{
    show_stats(STATUS_DFRAG);
    return CPARSER_OK;
}
cparser_result_t cparser_cmd_cont_dtct(cparser_context_t *context)
{
    show_stats(STATUS_DTCT);
    return CPARSER_OK; 
}
cparser_result_t cparser_cmd_cont_err(cparser_context_t *context)
{
    return CPARSER_OK; 
}
cparser_result_t cparser_cmd_cont_clr(cparser_context_t *context)
{
        context = NULL;
        printf("\e[1;1H\e[2J");
        printf("\n");
        return CPARSER_OK;
}
cparser_result_t cparser_cmd_cont_exit(cparser_context_t *context)
{
    return cparser_submode_exit(context->parser);
}
cparser_result_t cparser_cmd_stat_rxfp(cparser_context_t *context)
{
    get_rxfp_stats();
    return CPARSER_OK;
}
cparser_result_t cparser_cmd_stat_txfw(cparser_context_t *context)
{
    get_txfw_stats();
    return CPARSER_OK;
}
cparser_result_t cparser_cmd_stat_st1(cparser_context_t *context)
{
    get_st1_stats();
    return CPARSER_OK;
}
cparser_result_t cparser_cmd_stat_st2(cparser_context_t *context)
{
    get_st2_stats();
    return CPARSER_OK;
}
cparser_result_t cparser_cmd_stat_clr(cparser_context_t *context)
{
        context = NULL;
        printf("\e[1;1H\e[2J");
        printf("\n");
        return CPARSER_OK;
}
cparser_result_t cparser_cmd_stat_exit(cparser_context_t *context)
{
        return cparser_submode_exit(context->parser);
}
cparser_result_t cparser_cmd_show_clr(cparser_context_t *context)
       
{
        context = NULL;
        printf("\e[1;1H\e[2J");
        printf("\n");
        return CPARSER_OK;
}
cparser_result_t cparser_cmd_show_exit(cparser_context_t *context)
{
        return cparser_submode_exit(context->parser);
}
cparser_result_t cparser_cmd_clr(cparser_context_t *context)
{
        context = NULL;
        printf("\e[1;1H\e[2J");
        printf("\n");
        return CPARSER_OK;
}
cparser_result_t cparser_cmd_quit(cparser_context_t *context)
{
        cparser_quit(context ->parser);
        return CPARSER_OK;
}
