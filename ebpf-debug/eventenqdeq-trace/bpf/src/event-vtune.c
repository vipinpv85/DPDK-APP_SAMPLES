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

int64_t rdtsc()
{
    int64_t    tstamp;

#if defined(__x86_64__)
    asm(    "rdtsc\n\t"
            "shlq   $32,%%rdx\n\t"
            "or     %%rax,%%rdx\n\t"
            "movq   %%rdx,%0\n\t"
            : "=g"(tstamp)
            :
            : "rax", "rdx" );
#elif defined(__i386__)
    asm( "rdtsc\n": "=A"(tstamp) );
#else
#error NYI
#endif
    return tstamp;
}

extern void rte_hexdump(FILE *f, const char * title, const void * buf, unsigned int len);
extern void rte_pktmbuf_dump(FILE *, const struct rte_mbuf *, unsigned int);

struct rte_event {
	union {
		uint64_t event;
		struct {
			uint32_t flow_id:20;
			uint32_t sub_event_type:8;
			uint32_t event_type:4;
			uint8_t op:2;
			uint8_t rsvd:4;
			uint8_t sched_type:2;
			uint8_t queue_id;
			uint8_t priority;
			uint8_t impl_opaque;
		};
	};

	union {
		uint64_t u64;
		void *event_ptr;
		struct rte_mbuf *mbuf;
	};
};

uint64_t
entry(void *pkt)
{
	struct rte_event *eptr = (struct rte_event *) pkt;

	if (eptr->queue_id >= 4)
		rte_hexdump(stdout, NULL, pkt, 32);

return 1;
}
