#ifndef __DPDK_COMMON__
#define __DPDK_COMMON__

#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <sys/queue.h>
#include <getopt.h>
#include <signal.h>
#include <dirent.h>
#include <unistd.h>

#include <rte_common.h>
#include <rte_byteorder.h>
#include <rte_log.h>
#include <rte_memory.h>
#include <rte_memcpy.h>
#include <rte_memzone.h>
#include <rte_eal.h>
#include <rte_per_lcore.h>
#include <rte_launch.h>
#include <rte_atomic.h>
#include <rte_cycles.h>
#include <rte_prefetch.h>
#include <rte_lcore.h>
#include <rte_per_lcore.h>
#include <rte_branch_prediction.h>
#include <rte_interrupts.h>
#include <rte_pci.h>
#include <rte_random.h>
#include <rte_debug.h>
#include <rte_mempool.h>
#include <rte_mbuf.h>
#include <rte_ether.h>
#include <rte_ethdev.h>
#include <rte_ip.h>
#include <rte_tcp.h>
#include <rte_udp.h>
#include <rte_string_fns.h>
#include <rte_acl.h>
#include <rte_version.h>
#include <rte_tailq.h>
#include <rte_cfgfile.h>
#include <rte_string_fns.h>
#include <rte_bpf.h>
#include <rte_hexdump.h>
#include <rte_malloc.h>
#include <rte_eventdev.h>
#include <rte_event_eth_rx_adapter.h>
#include <rte_event_eth_tx_adapter.h>
#include <rte_service.h>
#include <rte_service_component.h>

#define RTE_LOGTYPE_APP RTE_LOGTYPE_USER1
#define DPDK_MAXARGS 32
#define TRACE_MAXEVENTS 16
#define BURST_SIZE 128

#define RESET       "\033[0m"
#define BLACK       "\033[30m"        /* Black */
#define RED         "\033[31m"        /* Red */
#define GREEN       "\033[32m"        /* Green */
#define YELLOW      "\033[33m"        /* Yellow */
#define BLUE        "\033[34m"        /* Blue */
#define MAGENTA     "\033[35m"        /* Magenta */
#define CYAN        "\033[36m"        /* Cyan */
#define WHITE       "\033[37m"        /* White */
#define BOLDBLACK   "\033[1m\033[30m" /* Bold Black */
#define BOLDRED     "\033[1m\033[31m" /* Bold Red */
#define BOLDGREEN   "\033[1m\033[32m" /* Bold Green */
#define BOLDYELLOW  "\033[1m\033[33m" /* Bold Yellow */
#define BOLDBLUE    "\033[1m\033[34m" /* Bold Blue */
#define BOLDMAGENTA "\033[1m\033[35m" /* Bold Magenta */
#define BOLDCYAN    "\033[1m\033[36m" /* Bold Cyan */
#define BOLDWHITE   "\033[1m\033[37m" /* Bold White */

#define BORDER1 "========================================================================="
#define BORDER2 "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"
#define BORDER3 "-------------------------------------------------------------------------"

struct element {
	struct rte_bpf *bpf;
	FILE *file;
	uint8_t enabled;
	char traceName[64];
	char fileName[64];
	char functionName[64];
	uint16_t LineNumber;
	uint64_t count;
};

void stopApp(void);
void setTraceInfo(uint8_t loc, const char *funcName, const char *fileName, uint32_t line);
void executeTrace(uint8_t loc, uint16_t request, void *bufs[], uint16_t nb);
void executeVtune(uint8_t loc, uint16_t request, uint16_t nb);
void executeMetaVtune(uint8_t loc, uint16_t request, uint16_t nb, const uint64_t *rc, int len);
void *ParseStackConfig(const char *CfgFile);
void display(void);
int CreateTracePoints(uint8_t count);

#endif /*__COMMON__*/
