/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2018 Intel Corporation
 */

/*
 * eBPF program sample.
 * Accepts pointer to struct rte_mbuf as an input parameter.
 * Dump the mbuf into stdout if it is an ARP packet (aka tcpdump 'arp').
 *
 * To compile on x86:
 * clang -O2 -U __GNUC__ -I${RTE_SDK}/${RTE_TARGET}/include \
 * -target bpf -Wno-int-to-void-pointer-cast -c t3.c
 *
 * To compile on ARM:
 * clang -O2 -I/usr/include/aarch64-linux-gnu \
 * -I${RTE_SDK}/${RTE_TARGET}/include -target bpf \
 * -Wno-int-to-void-pointer-cast -c t3.c
 */

#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <net/ethernet.h>
#include <rte_config.h>
#include "mbuf.h"
#include <rte_hexdump.h>
#include <arpa/inet.h>

extern void rte_hexdump(FILE * f, const char * title, const void * buf, unsigned int len);

uint64_t
entry(const void *pkt)
{
	rte_hexdump(stdout, "meta-dump", pkt, 64);

	return 1;
}
