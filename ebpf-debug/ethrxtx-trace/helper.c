#include "dpdk-common.h"

extern uint8_t runApp;

uint16_t argument_count = 1;
char argument[DPDK_MAXARGS][32] = {{"./ebpf-trace"}, {""}};
char usrMenu = '\0';
int8_t forceExit = 0;

struct rte_bpf_xsym bpf_xsym[] = {
	{
		.name = RTE_STR(stdout),
		.type = RTE_BPF_XTYPE_VAR,
		.var = {
			.val = &stdout,
			.desc = {
				.type = RTE_BPF_ARG_PTR,
				.size = sizeof(stdout),
			},
		},
	},
	{
		.name = RTE_STR(rte_pktmbuf_dump),
		.type = RTE_BPF_XTYPE_FUNC,
		.func = {
			.val = (void *)rte_pktmbuf_dump,
			.nb_args = 3,
			.args = {
				[0] = {
					.type = RTE_BPF_ARG_RAW,
					.size = sizeof(uintptr_t),
				},
				[1] = {
					.type = RTE_BPF_ARG_PTR_MBUF,
					.size = sizeof(struct rte_mbuf),
				},
				[2] = {
					.type = RTE_BPF_ARG_RAW,
					.size = sizeof(uint32_t),
				},
			},
		},
	},
		{
			.name = RTE_STR(rte_hexdump),
			.type = RTE_BPF_XTYPE_FUNC,
			.func = {
				.val = (void *)rte_hexdump,
				.nb_args = 4,
				.args = {
					[0] = {
						.type = RTE_BPF_ARG_RAW,
						.size = sizeof(uintptr_t),
					},
					[1] = {
						.type = RTE_BPF_ARG_RAW,
						.size = sizeof(uintptr_t),
					},
					[2] = {
						.type = RTE_BPF_ARG_RAW,
						.size = sizeof(uintptr_t),
					},
					[3] = {
						.type = RTE_BPF_ARG_RAW,
						.size = sizeof(uint32_t),
					},
				},
			},
		},
};


int CreateTracePoints(uint8_t count)
{
	if (count == 0) {
		RTE_LOG(ERR, APP, " failed to create no entries");
		return -1;
	}

	if (rte_memzone_lookup ((const char *)"BPF_REGIONS") != NULL) {
		RTE_LOG(ERR, APP, "BPF_REGIONS not expected to be created!");
		return -2;
	}

	const struct rte_memzone *mptr = rte_memzone_reserve_aligned(
				(const char *)"BPF_REGIONS",
				sizeof(struct rte_fbarray),
				SOCKET_ID_ANY,
				0,
				RTE_CACHE_LINE_SIZE);
	if (mptr == NULL) {
		RTE_LOG(ERR, APP, "BPF_REGIONS not created; err(%d)=%s!\n", rte_errno, strerror(rte_errno));
		return -2;
	}

	RTE_LOG(DEBUG, APP, "BPF_REGIONS %p addr %p", mptr, mptr->addr);

	if (rte_fbarray_init(
			(struct rte_fbarray *)mptr->addr,
			"TRACE_ALLOC_REG",
			count,
			sizeof(struct element)) < 0) {
		rte_panic("error rte_fbarray_init; err(%d)=%s!\n", rte_errno, strerror(rte_errno));
		return -2;
	}

	return 0;
}

void *ParseStackConfig(const char *CfgFile)
{
	struct rte_cfgfile *file = rte_cfgfile_load(CfgFile, 0);
	if (file == NULL) {
		RTE_LOG(ERR, APP, " failed to get file (%s)", CfgFile);
		return file;
	}

	/* get section name EAL */
	if (rte_cfgfile_has_section(file, "EAL")) {
		RTE_LOG(DEBUG, APP, " section (EAL); count %d\n", rte_cfgfile_num_sections(file, "EAL", sizeof("EAL") - 1));
		RTE_LOG(DEBUG, APP, " section (EAL) has entries %d\n", rte_cfgfile_section_num_entries(file, "EAL"));

		int n_entries = rte_cfgfile_section_num_entries(file, "EAL");
		struct rte_cfgfile_entry entries[n_entries];

		if (rte_cfgfile_section_entries(file, "EAL", entries, n_entries) != -1) {
			for (int i = 0; i < n_entries; i++) {

				if (strlen(entries[i].name)) {
					RTE_LOG(DEBUG, APP, " - entries[i].name: (%s)\n", entries[i].name);
					memcpy(argument[i * 2 + 1], entries[i].name, strlen(entries[i].name));
					argument_count += 1;
				}

				if (strlen(entries[i].value)) {
					RTE_LOG(DEBUG, APP, " - entries[i].value: (%s)\n", entries[i].value);
					memcpy(argument[i * 2 + 2], entries[i].value, strlen(entries[i].value));
					argument_count += 1;
				}
			}
		}

		for (int i = 0; i < argument_count; i++)
			RTE_LOG(DEBUG, APP, " - argument: (%s)\n", argument[i]);
	}

	/* get section name PORT-X */
	for (int i = 0; i < RTE_MAX_ETHPORTS; i++) {
		char port_section_name[15] = {""};

		sprintf(port_section_name, "%s%d", "PORT-", i);
		if (rte_cfgfile_has_section(file, port_section_name)) {
			int n_port_entries = rte_cfgfile_section_num_entries(file, port_section_name);

			RTE_LOG(INFO, APP, " %s", port_section_name);
			RTE_LOG(INFO, APP, " section (PORT) has %d entries\n", n_port_entries);

			struct rte_cfgfile_entry entries[n_port_entries];
			if (rte_cfgfile_section_entries(file, port_section_name, entries, n_port_entries) != -1) {

				for (int j = 0; j < n_port_entries; j++) {
					RTE_LOG(INFO, APP, " %s name: (%s) value: (%s)\n", port_section_name, entries[j].name, entries[j].value);

#if 0
					if (strcasecmp("rx-queues", entries[j].name) == 0)
						dpdk_ports[i].rxq_count = atoi(entries[j].value);
					else if (strcasecmp("tx-queues", entries[j].name) == 0)
						dpdk_ports[i].txq_count = atoi(entries[j].value);
					else if (strcasecmp("mtu", entries[j].name) == 0)
						dpdk_ports[i].mtu = atoi(entries[j].value);
					else if (strcasecmp("rss-tuple", entries[j].name) == 0)
						dpdk_ports[i].rss_tuple = atoi(entries[j].value);
					else if (strcasecmp("jumbo", entries[j].name) == 0)
						dpdk_ports[i].jumbo = (strcasecmp(entries[j].value, "yes") == 0) ? 1 : 0;
					else if (strcasecmp("core", entries[j].name) == 0)
						dpdk_ports[i].lcore_index = atoi(entries[j].value);
#endif
				}
			}
		}
	}

	/* get section name MEMPOOL-PORT */
	if (rte_cfgfile_has_section(file, "MEMPOOL-PORT")) {
		RTE_LOG(INFO, APP, " section (MEMPOOL-PORT); count %d\n", rte_cfgfile_num_sections(file, "MEMPOOL-PORT", sizeof("MEMPOOL-PORT") - 1));
		RTE_LOG(INFO, APP, " section (MEMPOOL-PORT) has entries %d\n", rte_cfgfile_section_num_entries(file, "MEMPOOL-PORT"));

		int n_entries = rte_cfgfile_section_num_entries(file, "MEMPOOL-PORT");
		struct rte_cfgfile_entry entries[n_entries];

		if (rte_cfgfile_section_entries(file, "MEMPOOL-PORT", entries, n_entries) != -1) {
			for (int j = 0; j < n_entries; j++) {
				RTE_LOG(INFO, APP, " - entries[i] name: (%s) value: (%s)\n", entries[j].name, entries[j].value);

#if 0
				if (strcasecmp("name", entries[j].name) == 0)
					rte_memcpy(dpdk_mempool_config.name, entries[j].value, sizeof(entries[j].value));
				if (strcasecmp("n", entries[j].name) == 0)
					dpdk_mempool_config.n = atoi(entries[j].value);
				if (strcasecmp("elt_size", entries[j].name) == 0)
					dpdk_mempool_config.elt_size = atoi(entries[j].value);
				if (strcasecmp("private_data_size", entries[j].name) == 0)
					dpdk_mempool_config.private_data_size = atoi(entries[j].value);
				if (strcasecmp("socket_id", entries[j].name) == 0)
					dpdk_mempool_config.private_data_size = atoi(entries[j].value);
#endif
			}
		}
	}

	rte_cfgfile_close(file);
	return file;
}

static struct rte_bpf*
ebpfTraceCreate(const char *fname)
{
	struct rte_bpf_prm prm;
	struct rte_bpf_jit jit;
	const char *sname = ".text";

	char ebpfFileName[250];
	snprintf(ebpfFileName, 250, "bpf/%s", fname);

	memset(&prm, 0, sizeof(prm));
	memset(&jit, 0, sizeof(jit));

	prm.xsym = bpf_xsym;
	prm.nb_xsym = RTE_DIM(bpf_xsym);
	prm.prog_arg.type = /*RTE_BPF_ARG_PTR */ RTE_BPF_ARG_PTR_MBUF;
	prm.prog_arg.size = RTE_MBUF_DEFAULT_BUF_SIZE;
	prm.prog_arg.buf_size = RTE_MBUF_DEFAULT_BUF_SIZE;

	//bpf = rte_bpf_load(&prm);
	struct rte_bpf *bpf = rte_bpf_elf_load(&prm, ebpfFileName, sname);
	if (bpf == NULL) {
		printf("%s@%d: failed to load bpf code, error=%d(%s);\n",
				__func__, __LINE__, rte_errno, strerror(rte_errno));
		return NULL;
	}

	rte_bpf_get_jit(bpf, &jit);
	printf("\n bpf (%p) with function (%p)\n",
			bpf, jit.func);

	return bpf;
}

void display(
		struct element *e_ptr, uint8_t numTrace)
{
	struct rte_bpf_xsym *ptr = bpf_xsym;
	uint8_t numElem = RTE_DIM(bpf_xsym);
	int i = 0, offset = 0, bpf_count = 1, usrVal = 0;
	char name[10][256];

	DIR *d = opendir("./bpf");
	if (d) {
		struct dirent *dir;
		while ((dir = readdir(d)) != NULL && bpf_count < 10)
		{
			printf("\033[%d;0H %d. %s",
				++offset, bpf_count, dir->d_name);

			snprintf(name[bpf_count - 1], 256, "%s", dir->d_name);
			bpf_count += 1;
		}

		closedir(d);
	}

	fflush(stdout);
	printf("\033[2J");

	printf("\033[2;2H --- eBPF options ---");
	printf(BLUE "\033[3;3H%.*s" RESET, 40, BORDER3);
	printf("\033[4;1H 1)DISPLAY\n 2)SELECT\n q)QUIT");
	printf(BOLDBLACK"\033[72;2H Enter Choice:"RESET);

	while (!forceExit)
	{
		usrMenu = getchar();

		switch (usrMenu) {
			case '1':
				do {
					offset = 2;
					printf("\033[2J");
					printf(BOLDBLACK "\033[%d;2H--------[display]--------" RESET, offset++);
					printf("\033[%d;1Ha)BPF Points", offset++);
					printf("\033[%d;1Hb)Trace Points", offset++);
					printf("\033[%d;1Hc)BPF files", offset++);
					printf(BOLDBLACK"\033[72;2H Enter Choice [M to main Menu]:"RESET);

					usrMenu = getchar();
					usrMenu = getchar();
					printf("\033[2J");
					offset = 2;

					if (usrMenu == 'a') {
						printf("\033[%d;2H Total: %-25d ", offset++, numElem);
						printf("\033[%d;0H %5s | %-25s | %-5s",
							++offset, "Index", "Name", "type");
						printf(BOLDBLUE "\033[%d;2H%.*s" RESET, ++offset, 60, BORDER1); 
	
						if (ptr) {
							for (i = 0, offset++; i < numElem; i++, offset++)
								printf("\033[%d;0H %5d | %-25s | %5s",
										offset, i, ptr[i].name,
										(ptr[i].type == RTE_BPF_XTYPE_FUNC) ? "FUNC":
										(ptr[i].type == RTE_BPF_XTYPE_VAR) ? "VAR":
										(ptr[i].type == RTE_BPF_XTYPE_NUM) ? "NUM":
										"unknown");
						}
						break;
					}
					else if (usrMenu == 'b') {
						printf("\033[%d;2H Total: %-25d ", offset++, numTrace);
						printf("\033[%d;0H %5s | %-10s | %-25s | %-25s | %-10s | %10s | %10s",
							++offset, "Index", "Name", "File", "Function", "Line", "BPF", "COUNT");
						printf(BOLDBLUE "\033[%d;2H%.*s" RESET, ++offset, 80, BORDER1);
	
						if (ptr) {
							for (i = 0, offset++; i < numTrace; i++, offset++)
								printf("\033[%d;0H %5d | %-10s | %-25s | %-25s | %10d | %10p | %"PRIu64,
										offset, i,
										e_ptr[i].traceName,
										e_ptr[i].fileName,
										e_ptr[i].functionName,
										e_ptr[i].LineNumber,
										(e_ptr[i].enabled) ? e_ptr[i].bpf : NULL,
										e_ptr[i].count);
						}
						break;
					}
					else if (usrMenu == 'c') {
						printf("\033[%d;0H %5s | %-10s ",
							++offset, "Index", "BPF File");
						printf(BOLDBLUE "\033[%d;2H%.*s" RESET, ++offset, 80, BORDER1);
						for (i = 0; i < bpf_count; i++)
							printf("\033[%d;0H %5d | %-10s",
								++offset, i,
								name[i]);
						break;
					}
					else if (usrMenu == 'd') {
						const struct rte_memzone *mptr = rte_memzone_lookup("BPF_REGIONS");
						if (mptr == NULL) {
							printf("\033[%d;2H failed to fetch BPF_REGIONS ", offset++);
							break;
						}

						if (rte_eal_process_type() == RTE_PROC_SECONDARY) {
							if (mptr) {
								int ret = rte_fbarray_attach((struct rte_fbarray *)mptr->addr);
								if (ret < 0) {
									printf("\033[%d;2H failed to attach fbarray for BPF_REGIONS ", offset++);
									break;
								}
							}
						}

						break;
					}
					else if (usrMenu == 'M') {
						break;
					}
			} while(usrMenu != 'a' || usrMenu != 'b' || usrMenu != 'c' || usrMenu != 'M');

				printf(BOLDBLACK "\033[%d;2H ---[press key]---" RESET, ++offset);
				getchar();
				getchar();
				printf("\033[2J");
				break;

			case '2':
				do {
					offset =2;
					printf("\033[2J");
					printf(BOLDBLACK "\033[%d;2H--------[trace]--------" RESET, offset++);
					printf("\033[%d;1Ha)Enable", offset++);
					printf("\033[%d;1Hb)Disbale", offset++);
					printf(BOLDBLACK"\033[72;2H Enter Choice [M to main menu]:"RESET);

					usrMenu = getchar();
					usrMenu = getchar();
					printf("\033[2J");
					offset = 2;

					if (usrMenu == 'a') {
						printf("\033[%d;2H Select BPF: ", offset++);
						usrMenu = getchar();
						usrMenu = getchar();
						usrVal = usrMenu - '0';
						if (usrVal > bpf_count) {
							printf("\033[%d;2H Invalid BPF selection (%d) ", offset++, usrVal);
							break;
						}
						printf("\033[%d;2H BPF (%s)", offset++, name[usrVal]);
						int8_t ebpfFileIndex = usrVal;

						printf("\033[%d;2H Select Trace: ", offset++);
						usrMenu = getchar();
						usrMenu = getchar();
						usrVal = usrMenu - '0';
						if ((usrVal > numTrace) || (e_ptr[usrVal].LineNumber = 0)) {
							printf("\033[%d;2H Invalid Trace selection (%d) ", offset++, usrVal);
							break;
						}
						printf("\033[%d;2H Trace: (%s) in (%s) on (%s) at (%u)", offset++,
							e_ptr[usrVal].traceName, e_ptr[usrVal].fileName,
							e_ptr[usrVal].functionName, e_ptr[usrVal].LineNumber);

						printf("\033[%d;2H Activating", offset++);
						e_ptr[usrVal].bpf = ebpfTraceCreate(name[ebpfFileIndex]);
						if (e_ptr[usrVal].bpf == NULL)
							e_ptr[usrVal].enabled = 0;
						else
							e_ptr[usrVal].enabled = 1;
						break;
					}
					else if (usrMenu == 'b') {
						printf("\033[%d;2H Select Trace: ", offset++);
						usrMenu = getchar();
						usrMenu = getchar();
						usrVal = usrMenu - '0';
						if ((usrVal > numTrace) || (e_ptr[usrVal].enabled == 0x00)){
							printf("\033[%d;2H Invalid Trace selection (%d) ", offset++, usrVal);
							break;
						}
						printf("\033[%d;2H Trace: (%s) in (%s) on (%s) at (%u)", offset++,
							e_ptr[usrVal].traceName, e_ptr[usrVal].fileName,
							e_ptr[usrVal].functionName, e_ptr[usrVal].LineNumber);

						printf("\033[%d;2H Deactivating", offset++);
						e_ptr[usrVal].enabled = 0;
						struct rte_bpf *bpf = e_ptr[usrVal].bpf;
						e_ptr[usrVal].bpf = NULL;
						rte_bpf_destroy(bpf);

						break;
					}
					else if (usrMenu == 'M') {
						break;
					}
				} while(usrMenu != 'a' || usrMenu != 'b' || usrMenu != 'M');

				printf(BOLDBLACK "\033[%d;2H ---[press key]---" RESET, ++offset);

				getchar();
				getchar();
				printf("\033[2J");
				break;

			case 'q':
				offset = 5;
				printf("\033[2J");
				printf(BOLDBLACK "\033[%d;2H--------[quit]--------\f" RESET, offset);
				forceExit = 1;
				runApp = 0;

				getchar();
				getchar();
				printf("\033[2J");
				break;

			default:
				fflush(stdout);
				printf("\033[2J");
				break;
		}

		if (likely(!forceExit)) {
			printf("\033[2J");
			printf("\033[2;2H --- eBPF options ---");
			printf(BLUE "\033[3;3H%.*s" RESET, 40, BORDER3);
			printf("\033[4;1H 1)DISPLAY\n 2)SELECT\n q)QUIT");
			printf(BOLDBLACK"\033[72;2H Enter Choice:"RESET);
		}

	}

}
