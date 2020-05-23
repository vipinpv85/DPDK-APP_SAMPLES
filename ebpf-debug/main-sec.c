#include "dpdk-common.h"

extern uint16_t argument_count;
extern char argument[DPDK_MAXARGS][32];
char *args[DPDK_MAXARGS];

int
main(void)
{
	int ret = 0;

	if (ParseStackConfig("trace-view.cfg") == NULL)
		rte_panic("failed to parse secodnary cfg file\n");

	for (int j = 0; j < argument_count; j++)
		args[j] = argument[j];

	ret = rte_eal_init(argument_count, args);
	if (ret < 0)
		rte_panic("Cannot init EAL\n");

	rte_eal_mp_wait_lcore();
	return 0;
}
