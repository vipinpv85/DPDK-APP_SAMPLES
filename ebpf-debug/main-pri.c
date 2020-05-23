#include "dpdk-common.h"

extern uint16_t argument_count;
extern char argument[DPDK_MAXARGS][32];
char *args[DPDK_MAXARGS];

/* Parse the argument given in the command line of the application */
static int
stack_parse_args(int argc, char **argv)
{
	char *prgname = argv[0];
	char **argvopt;
	int option_index, opt, ret;

	static const char short_options[] =
        "f:"  /* cfg file */
        "T:"  /* timer period */
        ;

	argvopt = argv;

	RTE_LOG(INFO, APP, " prgname (%s)\n", prgname);

	while ((opt = getopt_long(argc, argvopt, short_options, NULL, &option_index)) != EOF) {
		switch (opt) {
		case 'f':
			RTE_LOG(INFO, APP, " file (%s) index (%d)\n", optarg, option_index);
			if (ParseStackConfig(optarg) == NULL)
				return -1;
			ret = 0;
			break;

		case 'T':
			ret = 0;
			break;

		default:
			return  -2;
		}
	}

	return ret;
}

int
main(int argc, char **argv)
{
	int ret = 0;

	ret = stack_parse_args(argc, argv);
	if (ret != 0) {
		RTE_LOG(ERR, APP, " fail to read config\n");
		return -1;
	}

	for (int j = 0; j < argument_count; j++)
		args[j] = argument[j];

	ret = rte_eal_init(argument_count, args);
	if (ret < 0)
		rte_panic("Cannot init EAL\n");

	rte_eal_mp_wait_lcore();
	return 0;
}
