/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2010-2014 Intel Corporation
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <sys/queue.h>

#include <rte_memory.h>
#include <rte_launch.h>
#include <rte_eal.h>
#include <rte_per_lcore.h>
#include <rte_lcore.h>
#include <rte_debug.h>

#include <rte_eal.h>
#include <rte_memzone.h>
#include <rte_malloc.h>
#include <rte_fbarray.h>
#include <rte_cycles.h>

struct rte_memzone *mptr = NULL;

struct elements {
	char name[256];
	void *ptr;
};

static int
lcore_hello(__attribute__((unused)) void *arg)
{
	unsigned lcore_id;
	lcore_id = rte_lcore_id();
	printf("hello from core %u\n", lcore_id);

	void *ptr = NULL;
	int count = 0;
	struct elements *e = NULL;
	char name[256] = {0};

	do {
		rte_delay_ms(1000);

		ptr = rte_malloc(NULL, 16, 0);
		if (ptr == NULL)
			break;

		printf("rte_malloc for 16 bytes!\n");

		snprintf(name, 256, "%d-%d", rte_lcore_index(rte_lcore_id()), count++);
		printf("name %s %p\n", name, ptr);

		int index = rte_fbarray_find_next_free((struct rte_fbarray *)mptr->addr, 0);
		if (index < 0) {
			printf("fail in rte_fbarray_find_next_free %d", index);
			break;
		}

		printf("index %d", index);
		e = (struct elements *) rte_fbarray_get((struct rte_fbarray *)mptr->addr,
				index);

		memcpy(e->name, name, strlen(name));
		e->ptr = ptr;

		rte_fbarray_set_used((struct rte_fbarray *)mptr->addr, index);
	} while(1);

	return 0;
}

int
main(int argc, char **argv)
{
	int ret;
	unsigned lcore_id;

	ret = rte_eal_init(argc, argv);
	if (ret < 0)
		rte_panic("Cannot init EAL\n");

	mptr = rte_memzone_lookup("ADK_MALLOC_REGIONS");

	if (rte_eal_process_type() == RTE_PROC_SECONDARY) {
		int count = 0;
		unsigned int index = 0;
		printf("Process is RTE_PROC_SECONDARY!\n");

		if (mptr == NULL)
			rte_panic("mptr NULL\n");
		else
			printf("ADK_MALLOC_REGIONS %p addr %p\n", mptr, mptr->addr);

	ret = rte_fbarray_attach((struct rte_fbarray *)mptr->addr);
	if (ret < 0)
		rte_panic("error rte_fbarray_attach\n");

	while (1) {
		count = rte_fbarray_find_next_n_used((struct rte_fbarray *)mptr->addr,
				index, 2);
		printf("element index %d, fetch 2, count %d\n", index, count);

		if (count < 0)
			break;

		for(int i = index; i < index + 2; i++) {
			ret = rte_fbarray_is_used((struct rte_fbarray *)mptr->addr, i);
			if (ret == 0)
				continue;

			struct elements *e = (struct elements *) rte_fbarray_get(
					(struct rte_fbarray *)mptr->addr, i);
			if (e)
				printf("name %s ptr %p\n", e->name , e->ptr);
		}
		index += 2;
	}

	rte_fbarray_dump_metadata((struct rte_fbarray *)mptr->addr, stdout);

		rte_eal_cleanup();
		return 0;
	}

	mptr = rte_memzone_reserve_aligned("ADK_MALLOC_REGIONS",
			sizeof(struct rte_fbarray), SOCKET_ID_ANY,
			RTE_MEMZONE_2MB, 8);
	printf("ADK_MALLOC_REGIONS %p addr %p", mptr, mptr->addr);

	ret = rte_fbarray_init((struct rte_fbarray *)mptr->addr, "ADK_ALLOC_REG",
			512, sizeof(struct elements));
	if (ret < 0)
		rte_panic("error rte_fbarray_init\n");

	/* call lcore_hello() on every slave lcore */
	RTE_LCORE_FOREACH_SLAVE(lcore_id) {
		rte_eal_remote_launch(lcore_hello, NULL, lcore_id);
	}

	/* call it on master lcore too */
	lcore_hello(NULL);

	rte_eal_mp_wait_lcore();
	while(1);
	return 0;
}
