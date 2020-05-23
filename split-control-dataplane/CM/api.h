// api.h

/* I N C L U D E S */
#include <stdint.h>

//#include "common_struct.h"

extern uint32_t tcp_ipv4_rule_cnt; 
extern uint32_t udp_ipv4_rule_cnt; 
extern uint32_t ssl_ipv4_rule_cnt; 
extern uint32_t tls_ipv4_rule_cnt; 
extern uint32_t gre_ipv4_rule_cnt; 
extern uint32_t gtp_ipv4_rule_cnt; 
extern uint32_t ftp_ipv4_rule_cnt; 
extern uint32_t smtp_ipv4_rule_cnt; 
extern uint32_t sctp_ipv4_rule_cnt; 
extern uint32_t dns_ipv4_rule_cnt; 
extern uint32_t http_ipv4_rule_cnt; 
extern uint32_t icmp_ipv4_rule_cnt; 
extern uint32_t dcerpc_ipv4_rule_cnt; 
extern uint32_t smb_ipv4_rule_cnt; 
extern uint32_t smb2_ipv4_rule_cnt; 

extern uint32_t tcp_ipv6_rule_cnt; 
extern uint32_t udp_ipv6_rule_cnt; 
extern uint32_t ssl_ipv6_rule_cnt; 
extern uint32_t tls_ipv6_rule_cnt; 
extern uint32_t gre_ipv6_rule_cnt; 
extern uint32_t gtp_ipv6_rule_cnt; 
extern uint32_t ftp_ipv6_rule_cnt; 
extern uint32_t smtp_ipv6_rule_cnt; 
extern uint32_t sctp_ipv6_rule_cnt; 
extern uint32_t dns_ipv6_rule_cnt; 
extern uint32_t http_ipv6_rule_cnt; 
extern uint32_t icmp_ipv6_rule_cnt; 
extern uint32_t dcerpc_ipv6_rule_cnt; 
extern uint32_t smb_ipv6_rule_cnt; 
extern uint32_t smb2_ipv6_rule_cnt; 

extern  char opmode[4];
extern  char default_dir[8];
extern  uint8_t pmap_p0;
extern  uint8_t pmap_p1;
extern  uint8_t pmap_p2;
extern  uint8_t pmap_p3;

extern  uint8_t acl_p4;
extern  uint8_t acl_p1;
extern  uint8_t acl_p2;
extern  uint8_t acl_p3;
extern  uint8_t fdrp_p4;
extern  uint8_t fdrp_p1;
extern  uint8_t fdrp_p2;
extern  uint8_t fdrp_p3;
extern  uint8_t drp_p4;
extern  uint8_t drp_p1;
extern  uint8_t drp_p2;
extern  uint8_t drp_p3;
extern  uint8_t   st_p1;
extern  uint8_t   st_p2;
extern  uint8_t   st_p3;
extern  uint8_t   st_p4;

#define STATUS_FLOW 1
#define STATUS_DECODER 2
#define STATUS_TCP 3
#define STATUS_DFRAG 4
#define STATUS_DTCT 5
/* P R O T O T Y P E S */
uint32_t add_tcp_ipv4_rule(char *action, char *version, uint32_t sip, uint32_t sprt,
                      char *direction, uint32_t dip, uint32_t dprt);
uint32_t add_udp_ipv4_rule(char *action, char *version, uint32_t sip, uint32_t sprt, 
                      char *direction, uint32_t dip, uint32_t dprt);
uint32_t add_gre_ipv4_rule(char *action, char *version, uint32_t sip, uint32_t sprt, 
                      char *direction, uint32_t dip, uint32_t dprt);
uint32_t add_gtp_ipv4_rule(char *action, char *version, uint32_t sip, uint32_t sprt,
                       char *direction, uint32_t dip, uint32_t dprt);
uint32_t add_tls_ipv4_rule(char *action, char *version, uint32_t sip, uint32_t sprt,
                      char *direction, uint32_t dip, uint32_t dprt);
uint32_t add_ssl_ipv4_rule(char *action, char *version, uint32_t sip, uint32_t sprt, 
                      char *direction, uint32_t dip, uint32_t dprt);
uint32_t add_dcerpc_ipv4_rule(char *action, char *version, uint32_t sip, uint32_t sprt,
                      char *direction, uint32_t dip, uint32_t dprt);
uint32_t add_sctp_ipv4_rule(char *action, char *version, uint32_t sip, uint32_t sprt, 
                      char *direction, uint32_t dip, uint32_t dprt);
uint32_t add_smb_ipv4_rule(char *action, char *version, uint32_t sip, uint32_t sprt, 
                      char *direction, uint32_t dip, uint32_t dprt);
uint32_t add_smtp_ipv4_rule(char *action, char *version, uint32_t sip, uint32_t sprt,
                       char *direction, uint32_t dip, uint32_t dprt);
uint32_t add_http_ipv4_rule(char *action, char *version, uint32_t sip, uint32_t sprt,
                      char *direction, uint32_t dip, uint32_t dprt);
uint32_t add_icmp_ipv4_rule(char *action, char *version, uint32_t sip, uint32_t sprt, 
                      char *direction, uint32_t dip, uint32_t dprt);
uint32_t add_ftp_ipv4_rule(char *action, char *version, uint32_t sip, uint32_t sprt,
                      char *direction, uint32_t dip, uint32_t dprt);
uint32_t add_dns_ipv4_rule(char *action, char *version, uint32_t sip, uint32_t sprt, 
                      char *direction, uint32_t dip, uint32_t dprt);

uint32_t add_tcp_ipv6_rule(char *action, char *version, uint64_t sip, uint32_t sprt,
                      char *direction, uint64_t dip, uint32_t dprt);
uint32_t add_udp_ipv6_rule(char *action, char *version, uint64_t sip, uint32_t sprt, 
                      char *direction, uint64_t dip, uint32_t dprt);
uint32_t add_gre_ipv6_rule(char *action, char *version, uint64_t sip, uint32_t sprt, 
                      char *direction, uint64_t dip, uint32_t dprt);
uint32_t add_gtp_ipv6_rule(char *action, char *version, uint64_t sip, uint32_t sprt,
                       char *direction, uint64_t dip, uint32_t dprt);
uint32_t add_tls_ipv6_rule(char *action, char *version, uint64_t sip, uint32_t sprt,
                      char *direction, uint64_t dip, uint32_t dprt);
uint32_t add_ssl_ipv6_rule(char *action, char *version, uint64_t sip, uint32_t sprt, 
                      char *direction, uint64_t dip, uint32_t dprt);
uint32_t add_dcerpc_ipv6_rule(char *action, char *version, uint64_t sip, uint32_t sprt,
                      char *direction, uint64_t dip, uint32_t dprt);
uint32_t add_sctp_ipv6_rule(char *action, char *version, uint64_t sip, uint32_t sprt, 
                      char *direction, uint64_t dip, uint32_t dprt);
uint32_t add_smb_ipv6_rule(char *action, char *version, uint64_t sip, uint32_t sprt, 
                      char *direction, uint64_t dip, uint32_t dprt);
uint32_t add_smtp_ipv6_rule(char *action, char *version, uint64_t sip, uint32_t sprt,
                       char *direction, uint64_t dip, uint32_t dprt);
uint32_t add_http_ipv6_rule(char *action, char *version, uint64_t sip, uint32_t sprt,
                      char *direction, uint64_t dip, uint32_t dprt);
uint32_t add_icmp_ipv6_rule(char *action, char *version, uint64_t sip, uint32_t sprt, 
                      char *direction, uint64_t dip, uint32_t dprt);
uint32_t add_ftp_ipv6_rule(char *action, char *version, uint64_t sip, uint32_t sprt,
                      char *direction, uint64_t dip, uint32_t dprt);
uint32_t add_dns_ipv6_rule(char *action, char *version, uint64_t sip, uint32_t sprt, 
                      char *direction, uint64_t dip, uint32_t dprt);
void show_ipv4_rule(any_ipv4_rule_t *any_rule, result_ipv4_t *result);

void print_format(void);
void show_ipv6_rule(any_ipv6_rule_t *any_rule, result_ipv6_t *result);
uint8_t del_ipv4_rule(char *proto, uint32_t rulenum);
uint8_t del_ipv6_rule(char *proto, uint32_t rulenum);
void show_stats( uint8_t status);
void show_interface_stats( uint8_t status);
void get_info(void);
void get_intf_info(uint8_t num);
void get_rxfp_stats(void);
void get_txfw_stats(void);
void get_st1_stats(void);
void get_st2_stats(void);
void show_core_info(char *mode);
void show_pmap(void);
void show_pact_info(void);
