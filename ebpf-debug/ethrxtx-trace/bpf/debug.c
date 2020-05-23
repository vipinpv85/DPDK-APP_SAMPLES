/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2018 Intel Corporation
 */

/*
 * eBPF program sample.
 * Accepts pointer to first segment packet data as an input parameter.
 * analog of tcpdump -s 1 -d 'dst 1.2.3.4 && udp && dst port 5000'
 * (000) ldh      [12]
 * (001) jeq      #0x800           jt 2    jf 12
 * (002) ld       [30]
 * (003) jeq      #0x1020304       jt 4    jf 12
 * (004) ldb      [23]
 * (005) jeq      #0x11            jt 6    jf 12
 * (006) ldh      [20]
 * (007) jset     #0x1fff          jt 12   jf 8
 * (008) ldxb     4*([14]&0xf)
 * (009) ldh      [x + 16]
 * (010) jeq      #0x1388          jt 11   jf 12
 * (011) ret      #1
 * (012) ret      #0
 *
 * To compile on x86:
 * clang -O2 -U __GNUC__ -target bpf -c t1.c
 *
 * To compile on ARM:
 * clang -O2 -I/usr/include/aarch64-linux-gnu/ -target bpf -c t1.c
 */

#include <stdint.h>
#include <net/ethernet.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <arpa/inet.h>

#include <sys/ipc.h>
#include <sys/shm.h>

#include <rte_config.h>
#include "mbuf.h"
#include <rte_hexdump.h>

extern void rte_hexdump(FILE *f, const char * title, const void * buf, unsigned int len);
extern void rte_pktmbuf_dump(FILE *, const struct rte_mbuf *, unsigned int);

static int __attribute__((always_inline)) update(void *pkt)
{
	struct ether_header *ether_header = pkt;

	if (ether_header->ether_type != htons(0x0800)) {
		return 0;
	}

	return 1;
}

uint64_t
entry(void *pkt)
{
//	int shmid = shmget('D' + 'B' + 'G', sizeof(uint64_t), 0666|IPC_CREAT);
//	uint64_t *ptr = (uint64_t *) shmat(shmid,(void*)0,0);
	//size_t dbg_counter = *((int *) ptr + 0);
//	dbg_counter += 1;

	const struct rte_mbuf *mb = pkt;
	struct ether_header *eth = (struct ether_header *) rte_pktmbuf_mtod(mb, const struct ether_header *);

	if (update(pkt) == 0) {
		rte_hexdump(stdout, NULL, pkt, 32);
//		rte_pktmbuf_dump(stdout, mb, 64);
		return 0;
	}

	if (eth->ether_type == htons(ETHERTYPE_ARP) ||
			eth->ether_type == htons(ETHERTYPE_IP))
		rte_hexdump(stdout, NULL, eth, 32);
//		rte_pktmbuf_dump(stdout, mb, 64);
//		printf("data 1: %p %lx %lx\n", (uint64_t *) ptr + 0, *((uint64_t *) ptr + 0), dbg_counter);

	return 1;
}
