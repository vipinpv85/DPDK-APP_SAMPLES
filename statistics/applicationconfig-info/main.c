/*
 *   BSD LICENSE
 *
 *   Copyright(c) 2010-2016 Intel Corporation. All rights reserved.
 *   All rights reserved.
 *
 *   Redistribution and use in source and binary forms, with or without
 *   modification, are permitted provided that the following conditions
 *   are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in
 *       the documentation and/or other materials provided with the
 *       distribution.
 *     * Neither the name of Intel Corporation nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *   OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <stdarg.h>
#include <inttypes.h>
#include <sys/queue.h>
#include <stdlib.h>
#include <getopt.h>

#include <rte_eal.h>
#include <rte_common.h>
#include <rte_debug.h>
#include <rte_ethdev.h>
#include <rte_malloc.h>
#include <rte_memory.h>
#include <rte_memzone.h>
#include <rte_launch.h>
#include <rte_tailq.h>
#include <rte_per_lcore.h>
#include <rte_lcore.h>
#include <rte_debug.h>
#include <rte_log.h>
#include <rte_atomic.h>
#include <rte_branch_prediction.h>
#include <rte_string_fns.h>
#include <rte_cycles.h>
#include <rte_version.h>
#include <rte_eventdev.h>
#include <rte_cryptodev.h>

static const char *stats_border = "########################";

int main (int argc, char **argv)
{
	int ret;
	const struct rte_memseg* ptr = NULL;

	ret = rte_eal_init(argc, argv);
	if (ret < 0)
		rte_panic("Cannot init EAL\n");

	if (!rte_eal_primary_proc_alive(NULL))
		rte_exit(EXIT_FAILURE, "No primary DPDK process is running.\n");

	fprintf(stdout, "%s\n", stats_border);
	fprintf(stdout, " --- generic --- \n");
	fprintf(stdout, " current core id: %u", rte_lcore_id());
	fprintf(stdout, " role: %s\n", (rte_eal_lcore_role(rte_lcore_id()) == ROLE_RTE)?"ROLE_RTE":"ROLE_OFF");
	fprintf(stdout, " process: %s\n", (rte_eal_process_type() == RTE_PROC_SECONDARY)?"RTE_PROC_SECONDARY":"NA");
	fprintf(stdout, " huge Pages: %d\n", rte_eal_has_hugepages());
	fprintf(stdout, " syscall tid: %d", rte_sys_gettid());
	fprintf(stdout, " thread tid: %d\n", rte_gettid());
	fprintf(stdout, " version: %s\n", rte_version());
	fprintf(stdout, " dev count: %u\n", rte_eth_dev_count());
	fprintf(stdout, " event dev count: %u\n", rte_event_dev_count());
	fprintf(stdout, " crypto dev count: %u\n", rte_cryptodev_count());
	fprintf(stdout, "%s\n", stats_border);
	
	fprintf(stdout, " phy mem: %"PRIu64"\n", rte_eal_get_physmem_size());
	fprintf(stdout, " mem channel: %u\n", rte_memory_get_nchannel());
	fprintf(stdout, " mem rank: %u\n", rte_memory_get_nrank());
	fprintf(stdout, " check features for cpu on current vs compiled for: %d\n", rte_cpu_is_supported());

	{
		fprintf(stdout, "%s\n", stats_border);
		fprintf(stdout, " --- phy mem layout ---\n");
		rte_dump_physmem_layout(stdout);
		ptr = rte_eal_get_physmem_layout();
		if (ptr != NULL) {
			fprintf(stdout, " phys_addr: %"PRIu64"\n", ptr->phys_addr);
			fprintf(stdout, " len: %zu\n", ptr->len);
			fprintf(stdout, " huge page size: %"PRIu64"\n", ptr->hugepage_sz);
			fprintf(stdout, " socket id: %d\n", ptr->socket_id);
			fprintf(stdout, " nchannel: %d\n", ptr->nchannel);
			fprintf(stdout, " nrank: %d\n", ptr->nrank);
			fprintf(stdout, " addr: %p\n", ptr->addr);
			fprintf(stdout, " addr_64: %"PRIu64"\n", ptr->addr_64);
		}
		/* rte_mem_lock_page(virt) - for lookup tables*/

		fprintf(stdout, "%s \n", stats_border);
		fprintf(stdout, " --- memzone dump --- \n");
		rte_memzone_dump (stdout);

		fprintf(stdout, "%s \n", stats_border);
		fprintf(stdout, " --- malloc dump ---\n");
		rte_malloc_dump_stats (stdout, NULL);
		/* rte_malloc_set_limit	(NULL, max) */
	}

	fprintf(stdout, "%s \n", stats_border);
	fprintf(stdout, " -- reserve memory with memzone ---\n");
	struct rte_memzone *mzptr = rte_memzone_reserve("test1", ptr->len, 0, RTE_MEMZONE_2MB|RTE_MEMZONE_1GB);
	//struct rte_memzone *mzptr = rte_memzone_reserve_aligned("test1", ptr->len, 0, RTE_MEMZONE_2MB|RTE_MEMZONE_1GB);
	if (mzptr != NULL) {
		fprintf(stdout, " --- memzone dump --- \n");
		rte_memzone_dump (stdout);
	}

	fprintf(stdout, "%s \n", stats_border);
	fprintf(stdout, " -- reserve memory with memzone ---\n");
	void *cptr = rte_calloc ("test2", 1, ptr->len, 0);
	if (cptr != NULL) {
		fprintf(stdout, " --- alloc dump --- \n");
		rte_malloc_dump_stats(stdout, NULL);
	}

	return 0;
}

