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
#include <rte_memzone.h>
#include <rte_errno.h>
#include <rte_cycles.h>
#include <rte_memcpy.h>

typedef struct rule {
	unsigned char field[64];
	unsigned char mask[64];
} rule_t;

typedef struct rule_result {
	unsigned char field[64];
	unsigned char mask[64];
	unsigned char result[64];
} rule_result_t;

typedef struct rule_result_counter {
	unsigned char field[64];
	unsigned char mask[64];
	unsigned char result[64];
	unsigned long long int counter;
} rule_result_counter_t;

static void modify(void)
{
	const struct rte_memzone *ptr1 = rte_memzone_lookup("LKP-TBL");
	const struct rte_memzone *ptr2 = rte_memzone_lookup("LKP-TBL-INDIRECT");
	const struct rte_memzone *ptr3 = rte_memzone_lookup("LKPRST-TBL");
	const struct rte_memzone *ptr4 = rte_memzone_lookup("LKPRST-TBL-INDIRECT");
	const struct rte_memzone *ptr5 = rte_memzone_lookup("COUNTER");
	const struct rte_memzone *ptr6 = rte_memzone_lookup("counter-INDIRECT");
	const struct rte_memzone *ptr7 = rte_memzone_lookup("LKPRSTCNT-INDIRECT");

	rule_t *testrule = NULL;
	rule_result_t *testruleresult = NULL;
	void *testptr = NULL;
	uint64_t testcounter = NULL;
	rule_result_counter_t *testruleresultcounter = NULL;

	unsigned char *change = NULL;
	uint64_t count = 0;

	while (1) {
		if (ptr1) {
			testrule = ptr1->addr;
			testrule->field[0] = (unsigned char)count;
		}
		if (ptr2) {
			testptr = ptr2->addr;
			change = (unsigned char *) testptr;
			*change = count;
		}
		if (ptr3) {
			testruleresult = ptr3->addr;
			change = (unsigned char *) testruleresult;
			*change = count;
		}
		if (ptr4) {
			testptr = ptr4->addr;
		}
		if (ptr5) {
			testcounter = ptr5->addr;
		}
		if (ptr6) {
			unsigned char *temp = ptr6->addr;
		}
		if (ptr7) {
			testruleresultcounter = ptr7->addr;
		}

		count += 1;
	}
}

int
main(int argc, char **argv)
{
	int ret;
	unsigned lcore_id;
	const struct rte_memzone *ptr = NULL;

	ret = rte_eal_init(argc, argv);
	if (ret < 0)
		rte_panic("Cannot init EAL\n");

	if (rte_eal_process_type() == 0)
		rte_exit(EXIT_FAILURE, " This should be secondary\n");

	rte_memzone_dump(stdout);

	modify();

	return 0;
}

