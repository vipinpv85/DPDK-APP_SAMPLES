#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <stdint.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>

#include <rte_config.h>
#include <rte_branch_prediction.h>
#include <rte_debug.h>
#include <rte_eal.h>
#include <rte_common.h>
#include <rte_cfgfile.h>
#include <rte_mempool.h>
#include <rte_malloc.h>
#include <rte_mbuf.h>
#include <rte_memcpy.h>
#include <rte_lcore.h>
#include <rte_ethdev.h>
#include <rte_errno.h>
#include <rte_ip.h>
#include <rte_udp.h>
#include <rte_ethdev.h>
#include <rte_version.h>
#include <rte_hash.h>
#include <rte_arp.h>
#include <rte_qm.h>
#include <rte_hash_crc.h>

/* D E F I N E S */   
#define MAX_NUMANODE   (4)
#define MAX_LCORECOUNT (32)
#define MAX_INTFCOUNT  (8)

#define NB_MBUF   ((8192 * 2) - 1)
#define MBUF_SIZE (2048 + sizeof(struct rte_mbuf) + RTE_PKTMBUF_HEADROOM)

#define RX_RING_SIZE (1024)
#define TX_RING_SIZE (256)
#define NUM_MBUFS ((8192 * 2) -1)
#define MBUF_CACHE_SIZE (RTE_MBUF_DEFAULT_DATAROOM + sizeof(struct rte_mbuf) + RTE_PKTMBUF_HEADROOM)
#define BURST_SIZE (32)
#define PREFETCH_OFFSET (3)

#define CMD_LINE_OPT_RX "rx"
#define CMD_LINE_OPT_FIFO "fifo"
#define CMD_LINE_OPT_PARSE_IPV4 "4"
#define CMD_LINE_OPT_PARSE_IPV6 "6"
#define CMD_LINE_OPT_PARSE_IP "all"

#define RTE_RING_SIZE 4096

/* S T R U C T U R E S */
typedef struct sqmInfo_s {
  struct rte_qm_dev *qm;
  struct rte_qm_port *Ingress[8];
  struct rte_qm_port *Egress[8];
  struct rte_qm_port *Workers[32];

  int32_t qid[8];
  struct rte_qm_port *qIngress[8];
  struct rte_qm_port *qEgress[8];
  struct rte_qm_port *qWorkers[32];

  uint8_t cntIngress;
  uint8_t cntEgress;
  uint8_t cntWorkers;
  uint8_t cntQid;
} sqmInfo_t;

#endif /*__CONFIG_H__*/
