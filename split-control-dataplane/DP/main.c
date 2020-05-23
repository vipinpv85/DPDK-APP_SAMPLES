/* main.c */

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <signal.h>
#include <sys/queue.h>

#include <rte_memory.h>
#include <rte_memzone.h>
#include <rte_launch.h>
#include <rte_tailq.h>
#include <rte_eal.h>
#include <rte_per_lcore.h>
#include <rte_lcore.h>
#include <rte_debug.h>
#include <rte_power.h>

#include "dpcommon.h"
#include "config.h"
#include "version.h"

uint32_t masterCore = 0;
uint32_t slaveCore [64];
int32_t slaveCount = 0;

struct rte_ring *srb [16];

/* E X T E R N */
extern uint64_t rulesSet;
extern struct dp_resultTuple aclIpv4Result [1024];
extern struct dp_resultTuple aclIpv6Result [1024];


int main(int argc, char **argv)
{
    int32_t index = 0, indexCore = 0, ret = 0;
    char srbName [25];
    struct dp_ipv4Tuple params = {.flow = FLOWDIR_S2D, .proto = 17, .sPort = 0, .dPort = 0, .srcIp = 0, .dstIp = 0 };
    
    uint32_t userData = 10;

    MSG("DEBUG: check dependency ...\n");

    ret = rte_eal_has_hugepages();
    if (unlikely(ret < 0))
    {
        MSG(" ERROR: No hugepage configured%d ", ret);
        return -1;
    }

    MSG("DEBUG: RTE EAL INIT ...\n");

    MSG("%15s:%s", "Build Host", UNAME);
    MSG("%15s:%s", "GCC", GCC_VER);
    MSG("%15s:%s", "SDK", DPDK_SDK);
    MSG("%15s:%s", "TARGET", DPDK_TARGET);

    ret = rte_eal_init(argc, argv);
    if (unlikely(ret < 0))
        rte_panic("ERROR: Cannot init EAL\n");
#if 0
     if (rte_power_set_env(PM_ENV_ACPI_CPUFREQ) != 0)
        rte_panic("ERROR: Set Freq\n");
#else
    rte_power_init(2);
    rte_power_init(3);
#endif

    /* TODO: Load Ini Configuration */
    /* Create ACL for lookups */
    ret = tableLookupCreate();
    if (unlikely(ret < 0))
        rte_panic("ERROR: ACL lookup Creation\n");

    MSG("DEBUG: ACL Lookup table Created !!!");

    /* test dummy result with acl mapping */
    aclIpv4Result [userData -1].action   = ACTIONORDER_PASS;
    aclIpv4Result [userData -1].next     = 0x01;
    aclIpv4Result [userData -1].msgIndex = 0xBADA;
    aclIpv4Result [userData -1].counter  = 0xffff1111;

    ret = tableLookupAddIpv4Rule(&params, userData);
    if (ret < 0)
        rte_panic("ERROR: ACL Add Rules IPV4\n");

    /* TODO: To remove only for testing */
    rulesSet |= (1 << MDPOS_UDP4);

    ret = tableLookup_ipv4Rules_build();
    if (ret < 0)
        rte_panic("ERROR: ACL Build Rules IPV4\n");

    ret = tableLookup_ipv4Rules_lookup(&params);
    if (ret < 0)
        rte_panic("ERROR: ACL lookup IPV4\n");
    else
       MSG("DEBUG: ACL IPV4 Lookup Success");

    ret = ethDevSetup();
    if (unlikely(ret < 0))
        rte_panic("ERROR: Eth Dev Setup\n");

    initl2Functions();
    initl3Functions();
    //initl4Functions();
    initcounterFunctions();

    MSG("DEBUG: Registering Signal handlers for stats!!!");
    signal (SIGUSR1, sigHandler);
    signal (SIGUSR2, sigHandler);

    MSG("DEBUG: RTE MP WAIT LCORE ...\n");

    masterCore = rte_get_master_lcore();
    MSG("master core: %u", masterCore);

    slaveCount = rte_lcore_count () - 1;
    if (slaveCount < 3)
    {
        rte_panic("ERROR: Slave Core Count\n");
    }
    MSG("slave core count: %u", slaveCount);


    RTE_LCORE_FOREACH_SLAVE(index)
    {
        slaveCore [indexCore++] = index;
        MSG("slave index :%u", index);
    }

    MSG(" DEBUG: LOGIC - BEGIN \n");


    MSG(" DEBUG: invoke lcore threads  \n");
    /* create queue for RX frame parser to Suricata threads*/
    for (index = 0; index < (slaveCount - 1); index++)
    {
        sprintf(srbName, "%s%d", "RINGBUFF", index);
        MSG("Suricata Ring Buffer Name: %s", srbName);

        srb [index] = rte_ring_create(srbName, RTE_RING_SIZE, 
                             SOCKET_ID_ANY, RING_F_SP_ENQ);

        if (NULL == srb [index])
            rte_exit(EXIT_FAILURE, "ERROR: Cannot create output ring %s\n", srbName);
    }

    /* deploy on lcore */
    index = 0;
    rte_eal_remote_launch(frameParseSetup, NULL, slaveCore [index]);
    MSG("frame parser index: %u core: %u", index, slaveCore [index]);

    for (index = 1; index < slaveCount; index++)
    {
        rte_eal_remote_launch(userThread, NULL, slaveCore [index]);
        MSG("ether Parse index: %u core: %u", index, slaveCore [index]);
    }
    /* create pthread for SURICATA */


    MSG("\n DEBUG: start capture on interface  \n");
    ret = ethDevStart();
    if (unlikely(ret < 0))
    {
        rte_panic("ERROR: interface failed\n");
    }

    /* TODO: Implement Sighandlers */
    /* master core - CLI framework */
    MSG("Invoking CLI on master core");
    cli_main();
    /* wait for threads to fnish their work */
    RTE_LCORE_FOREACH_SLAVE(index) 
    {
        if (rte_eal_wait_lcore(index) < 0)
            return -1;
    }

    MSG(" DEBUG: LOGIC - END \n");

    rte_eal_mp_wait_lcore();

    /* Destroy ACL Tables */
    tableLookupDestroy();

    return 0;
}

