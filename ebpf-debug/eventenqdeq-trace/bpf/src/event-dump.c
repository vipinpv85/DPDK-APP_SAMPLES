/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2018 Intel Corporation
 */

#include <stdint.h>
#include <net/ethernet.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <arpa/inet.h>

#include <rte_config.h>
#include "mbuf.h"
#include <rte_hexdump.h>

extern void rte_hexdump(FILE *f, const char * title, const void * buf, unsigned int len);
extern void rte_pktmbuf_dump(FILE *, const struct rte_mbuf *, unsigned int);

uint64_t
entry(void *pkt)
{
	rte_hexdump(stdout, NULL, pkt, 32);

	return 1;
}
