
For MMAP failure in Primary-Secondary Models:-

        char *env = getenv("DUMMY_RUN");
        fprintf(stdout, " init env DUMMY_RUN: %s\n", env);
 
        if ((env == NULL) || (strcmp(env,"no") == 0))
        {
                ret = rte_eal_init(argc, argv);
                if (ret < 0)
                        rte_exit(EXIT_FAILURE, "Invalid EAL arguments\n");
 
                ret = setenv("DUMMY_RUN", "yes", 1);
 
                fprintf(stdout, " --- get info DUMMY init --- \n");
 
                struct rte_config *data = rte_eal_get_configuration ();
                fprintf(stdout, " master_lcore: %u\n", data->master_lcore);
                fprintf(stdout, " lcore_count: %u\n", data->lcore_count);
                for (ret = 0; ret < RTE_MAX_LCORE; ret++) {
                        if (data->lcore_role[ret] == ROLE_RTE)
                                fprintf(stdout, " -- lcore id: %d \n", ret);
                }
                fprintf(stdout, " process_type: %u\n", data->process_type);
                struct rte_memseg *mem = rte_eal_get_physmem_layout ();
                fprintf(stdout, " socket: %u\n", mem->socket_id);
                fprintf(stdout, " addr: %p\n", mem->addr);
                char baseaddr[100] = {0};
                sprintf(baseaddr, "--base-virtaddr=0x%"PRIx64, mem->addr);
                fprintf(stdout, "\n\n .... baseaddr: %s\n\n", baseaddr);
                char *args[]={"/home/test/Downloads/adk-tiger/dpdk-17.05/examples/l2fwd/build/l2fwd", "--no-pci", "--log-level=8", "-l 2,4,5", baseaddr, NULL};
                execvp(args[0], args);
 
                return ret;
        }
 
        /* init EAL */
        ret = setenv(" reset DUMMY_RUN", "no", 1);
 
        ret = rte_eal_init(argc, argv);
        if (ret < 0)
                rte_exit(EXIT_FAILURE, "Invalid EAL arguments\n");
