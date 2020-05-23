#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <sys/queue.h>
#include <signal.h>

#include <rte_memory.h>
#include <rte_launch.h>
#include <rte_eal.h>
#include <rte_per_lcore.h>
#include <rte_lcore.h>
#include <rte_debug.h>
#include <rte_memzone.h>
#include <rte_errno.h>
#include <rte_cycles.h>
#include <rte_memcpy.h>

static void
signal_handler(int signum)
{
	if (signum == SIGUSR2) {
		printf("\n\nSignal %d received\n", signum);
		const struct rte_memzone *ptr1 = rte_memzone_lookup("LKP-TBL");
		uint64_t *ptr = (uint64_t *) ptr1->addr;
		*ptr += 1;
	}
}


int
main(int argc, char **argv)
{
	int ret;
	const struct rte_memzone *ptr = NULL;

	signal(SIGUSR2, signal_handler);

	ret = rte_eal_init(argc, argv);
	if (ret < 0)
		rte_panic("Cannot init EAL\n");

	if (rte_eal_process_type())
		rte_exit(EXIT_FAILURE, " This should be primary\n");

	/* create direct lookup table */
	ptr = rte_memzone_reserve("LKP-TBL", 1000 * 128, SOCKET_ID_ANY, RTE_MEMZONE_1GB | RTE_MEMZONE_SIZE_HINT_ONLY);
	if (ptr == NULL) {
		fprintf(stderr, "failed to create Lookup table, cause (%s)!\n", rte_strerror(rte_errno));
		return -1;
	}

	/* create indirect direct lookup table */
	ptr = rte_memzone_reserve("LKP-TBL-INDIRECT", 1000 * 8, SOCKET_ID_ANY, RTE_MEMZONE_2MB | RTE_MEMZONE_SIZE_HINT_ONLY);
	if (ptr == NULL) {
		fprintf(stderr, "failed to create Lookup indirect table, cause (%s)!\n", rte_strerror(rte_errno));
		return -1;
	}

	/* create direct lookup+result table */
	ptr = rte_memzone_reserve("LKPRST-TBL", 1000 * (128 + 64), SOCKET_ID_ANY, RTE_MEMZONE_1GB | RTE_MEMZONE_SIZE_HINT_ONLY);
	if (ptr == NULL) {
		fprintf(stderr, "failed to create Lookup-Result table, cause (%s)!\n", rte_strerror(rte_errno));
		return -1;
	}

	/* create indirect lookup+result table */
	ptr = rte_memzone_reserve("LKPRST-TBL-INDIRECT", 1000 * 8, SOCKET_ID_ANY, RTE_MEMZONE_2MB | RTE_MEMZONE_SIZE_HINT_ONLY);
	if (ptr == NULL) {
		fprintf(stderr, "failed to create Lookup-RESULT indirect table, cause (%s)!\n", rte_strerror(rte_errno));
		return -1;
	}

	/* create direct counter table */
	ptr = rte_memzone_reserve("COUNTER", 256 * 8, SOCKET_ID_ANY, RTE_MEMZONE_2MB | RTE_MEMZONE_SIZE_HINT_ONLY);
	if (ptr == NULL) {
		fprintf(stderr, "failed to counter, cause (%s)!\n", rte_strerror(rte_errno));
		return -1;
	}

	/* create indirect counter table */
	ptr = rte_memzone_reserve("counter-INDIRECT", 256 * 1, SOCKET_ID_ANY, RTE_MEMZONE_2MB | RTE_MEMZONE_SIZE_HINT_ONLY);
	if (ptr == NULL) {
		fprintf(stderr, "failed to counter indirect, cause (%s)!\n", rte_strerror(rte_errno));
		return -1;
	}

	/* create direct lookup+result+counter table */
	ptr = rte_memzone_reserve("LKPRSTCNT-INDIRECT", 1000 * (128 + 64 + 8), SOCKET_ID_ANY, RTE_MEMZONE_1GB | RTE_MEMZONE_SIZE_HINT_ONLY);
	if (ptr == NULL) {
		fprintf(stderr, "failed to create Lookup-Result-Counter table, cause (%s)!\n", rte_strerror(rte_errno));
		return -1;
	}

	rte_memzone_dump(stdout);

	while(1);

	return 0;
}

