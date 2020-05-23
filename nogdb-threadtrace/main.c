/*-
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
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>
#include <sys/types.h>
#include <sys/queue.h>
#include <netinet/in.h>
#include <setjmp.h>
#include <stdarg.h>
#include <ctype.h>
#include <errno.h>
#include <getopt.h>
#include <signal.h>
#include <stdbool.h>

#include <rte_common.h>
#include <rte_log.h>
#include <rte_malloc.h>
#include <rte_memory.h>
#include <rte_memcpy.h>
#include <rte_memzone.h>
#include <rte_eal.h>
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
#include <rte_ether.h>
#include <rte_ethdev.h>
#include <rte_mempool.h>
#include <rte_mbuf.h>
#include <rte_pdump.h>

static volatile bool force_quit;

/* MAC updating enabled by default */
static int mac_updating = 1;

#define RTE_LOGTYPE_L2FWD RTE_LOGTYPE_USER1

#define NB_MBUF   8192

#define MAX_PKT_BURST 32
#define BURST_TX_DRAIN_US 100 /* TX drain every ~100us */
#define MEMPOOL_CACHE_SIZE 256

/*
 * Configurable number of RX/TX ring descriptors
 */
#define RTE_TEST_RX_DESC_DEFAULT 128
#define RTE_TEST_TX_DESC_DEFAULT 512
static uint16_t nb_rxd = RTE_TEST_RX_DESC_DEFAULT;
static uint16_t nb_txd = RTE_TEST_TX_DESC_DEFAULT;

/* ethernet addresses of ports */
static struct ether_addr l2fwd_ports_eth_addr[RTE_MAX_ETHPORTS];

/* mask of enabled ports */
static uint32_t l2fwd_enabled_port_mask = 0;

/* list of enabled ports */
static uint32_t l2fwd_dst_ports[RTE_MAX_ETHPORTS];

static unsigned int l2fwd_rx_queue_per_lcore = 1;

#define MAX_RX_QUEUE_PER_LCORE 16
#define MAX_TX_QUEUE_PER_PORT 16
struct lcore_queue_conf {
	unsigned n_rx_port;
	unsigned rx_port_list[MAX_RX_QUEUE_PER_LCORE];
} __rte_cache_aligned;
struct lcore_queue_conf lcore_queue_conf[RTE_MAX_LCORE];

static struct rte_eth_dev_tx_buffer *tx_buffer[RTE_MAX_ETHPORTS];

static const struct rte_eth_conf port_conf = {
	.rxmode = {
        .max_rx_pkt_len = 0x2600,
		.split_hdr_size = 0,
		.header_split   = 0, /**< Header Split disabled */
		.hw_ip_checksum = 0, /**< IP checksum offload disabled */
		.hw_vlan_filter = 0, /**< VLAN filtering disabled */
		.jumbo_frame    = 1, /**< Jumbo Frame Support disabled */
		.hw_strip_crc   = 1, /**< CRC stripped by hardware */
	},
	.txmode = {
		.mq_mode = ETH_MQ_TX_NONE,
	},
};

struct rte_mempool * l2fwd_pktmbuf_pool = NULL;

/* Per-port statistics struct */
struct l2fwd_port_statistics {
	uint64_t tx;
	uint64_t rx;
	uint64_t dropped;
} __rte_cache_aligned;
struct l2fwd_port_statistics port_statistics[RTE_MAX_ETHPORTS];

#define MAX_TIMER_PERIOD 86400 /* 1 day max */
/* A tsc-based timer responsible for triggering statistics printout */
static uint64_t timer_period = 10; /* default period is 10 seconds */


#ifdef DUMPSTACK
#include <rte_version.h>

#define UNW_LOCAL_ONLY 
#include <libunwind.h>
#include <dirent.h>
#include <sys/ptrace.h>
#include <sys/user.h>

uint8_t g_threads = 0;
pthread_t tid[64] = {0};
char exeName[100] = {0};

int unwind_thread_callstack (void);
void sig_pthreadhandler(__rte_unused int signo, __rte_unused siginfo_t *info, __rte_unused void *dummy);
void sig_handler(int signo);
void get_therads(void);

__attribute__ ((inline))
void get_therads(void)
{
    char dirname[100];

    DIR *proc_dir;
    {
        snprintf(dirname, sizeof dirname, "/proc/%d/task", getpid());
        fprintf (stdout, "getpid (%d)\n", getpid());
        proc_dir = opendir(dirname);
    }

    if (proc_dir)
    {
        /* /proc available, iterate through tasks... */
        struct dirent *entry;
        while ((entry = readdir(proc_dir)) != NULL)
        {
            if(entry->d_name[0] == '.')
                continue;

            int tid = atoi(entry->d_name);
            fprintf (stdout, " ---------- tid (%d)\n", tid);

            // compare for "pdump-thread"
            sprintf (dirname, "cat /proc/%d/task/%d/comm", rte_sys_gettid(), tid);
            system (dirname);
        }

        fflush(stdout);
        closedir(proc_dir);
    }
    else
    {
        /* /proc not available, act accordingly */
    }
    return;
}

__attribute__((noinline))
void sig_pthreadhandler(__rte_unused int signo, __rte_unused siginfo_t *info, __rte_unused void *dummy)
{
    printf(" Inside sig_pthreadhandler %lu for signal %u\n", pthread_self(), signo);
    unwind_thread_callstack ();
    fflush(stdout);

#if 0
    sigset_t mask;
    sigfillset(&mask);
    sigdelset(&mask, signo);
    sigaddset (&mask, SIGUSR2);
    sigsuspend(&mask);
#endif

    return;
}
__attribute__((noinline))
void sig_handler(int signo)
{
    sigset_t mask;
    pthread_t temp = rte_sys_gettid();

    char cmd[30] = {0};
    struct rte_config *config = NULL;

    sprintf (exeName, "/proc/%d/cmdline", temp);
    FILE *f = fopen(exeName, "r");
    if (f) {
        fgets(exeName, sizeof(exeName) / sizeof(*exeName), f);
        fclose(f);
        fprintf(stdout, " Executable Name: %s\n", exeName);
    }


    fprintf(stdout, "\n *** Processing sig_handler %lu for signal %d\n", pthread_self(), signo);

    if (rte_sys_gettid() == rte_gettid()) {

//        printf("rte_sys_gettid %d rte_gettid %d rte_sys_gettid %d pthread_self %d\n",
  //              rte_sys_gettid(), rte_gettid(), rte_sys_gettid(), pthread_self());


#ifdef DUMPSTACK_EXTRA
        printf("\n\n----------------- MMAP BEGIN -----------------\n");
        sprintf (cmd, "pmap -x %d", temp);
        system (cmd);
        printf("----------------- MMAP DONE -----------------\n");

#if 0
        printf("\n\n----------------- PS INFO BEGIN -----------------\n");
        sprintf (cmd, "ps -mo THREAD -p %d", temp);
        system (cmd);
        rte_delay_ms(100);
        printf("----------------- PS INFO DONE -----------------\n");
#endif
#endif

        printf("\n\n----------------- THREAD NAME BEGIN -----------------\n");
        sprintf (cmd, "ls /proc/%d/task/*/comm", temp);
        system (cmd);
        sprintf (cmd, "cat /proc/%d/task/*/comm", temp);
        system (cmd);
        printf("----------------- THREAD NAME DONE -----------------\n");

        printf(" DPDK Version 0x%x\n", RTE_VERSION);
        config = rte_eal_get_configuration();
        printf(" Config: msater %u lcore count %u process %d\n", config->master_lcore, config->lcore_count, config->process_type);
        printf(" rte_sys_gettid %d \n", temp);

        //get_therads ();
        rte_delay_ms(10);
        fflush(stdout);
    }
        
    printf("----------------- LOCAL unwind_thread_callstack -----------------\n");
    unwind_thread_callstack();
    rte_delay_ms(2000);
    printf("^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n");
    fflush(stdout);

    switch (signo) {
        case SIGSEGV:
            //signal(signo, SIG_DFL);

            printf(" ---------------- Stack Unwind BEGIN ----------------------\n");
            for (int i = 0; i < g_threads; i++) {
                temp = tid[i];
                if (temp) {
                    tid[i] = 0;
                    printf(" ++++++++++++++++++++++++ Thread index %d ID %lu ++++++++++++++++++++++++\n", i, temp);
                    if (pthread_kill(temp, SIGUSR2) != 0) 
                        continue;
                    rte_delay_ms(2000);
                    printf("^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n");
                }
            }
            printf(" ---------------- Stack Unwind DONE ----------------------\n");

            fflush(stdout);
            exit(2);
        
        default:
            printf(" unknown signal %d", signo);
        break;
    }

    fflush(stdout);
    return;
}

__attribute__((noinline))
int getFileAndLine (unw_word_t addr, char *file, size_t flen, int *line)
{
    static char buf[256];
    char *p;

    sprintf (buf, "/usr/bin/addr2line -C -e %s -f -i %lx", exeName, addr);
    FILE* f = popen (buf, "r");

    if (f == NULL)
    {
        perror (buf);
        return 0;
    }

    // get function name
    fgets (buf, 256, f);

    // get file and line
    fgets (buf, 256, f);

    if (buf[0] != '?')
    {
        int l;
        char *p = buf;

        // file name is until ':'
        while (*p != ':')
        {
            p++;
        }

        *p++ = 0;
        // after file name follows line number
        strcpy (file , buf);
        sscanf (p,"%d", line);
    }
    else
    {
        strcpy (file,"unkown");
        *line = 0;
    }

    pclose(f);
    return 0;
}

__attribute__((noinline))
int unwind_thread_callstack (void)
{
    unw_cursor_t cursor; unw_context_t uc;
    unw_word_t ip, sp, bp, offp;
    unw_word_t sp_begin = 0x0, sp_end = 0x0;
    unw_word_t val[20];
    unw_proc_info_t pi;

    char name[256];
    int depth = 0;
    char file[256];
    int line = 0;

    unw_getcontext (&uc);
    unw_init_local (&cursor, &uc);

    while (unw_step(&cursor) > 0)
    {
        name[0] = '\0';

        unw_get_proc_name (&cursor, name, 256, &offp);
        getFileAndLine((long)ip, file, 256, &line);

        unw_get_reg (&cursor, UNW_X86_64_RIP, &ip);
        unw_get_reg (&cursor, UNW_X86_64_RSP, &sp);
        unw_get_reg (&cursor, UNW_X86_64_RBP, &bp);

        unw_get_reg (&cursor, UNW_X86_64_RAX, &val[0]); 
        unw_get_reg (&cursor, UNW_X86_64_RDX, &val[1]);
        unw_get_reg (&cursor, UNW_X86_64_RCX, &val[2]);
        unw_get_reg (&cursor, UNW_X86_64_RBX, &val[3]);
        unw_get_reg (&cursor, UNW_X86_64_RSI, &val[4]);
        unw_get_reg (&cursor, UNW_X86_64_RDI, &val[5]);
        unw_get_reg (&cursor, UNW_X86_64_RBP, &val[6]);
        unw_get_reg (&cursor, UNW_X86_64_RSP, &val[7]);
        unw_get_reg (&cursor, UNW_X86_64_R8,  &val[8]);
        unw_get_reg (&cursor, UNW_X86_64_R9,  &val[9]);
        unw_get_reg (&cursor, UNW_X86_64_R10, &val[10]);
        unw_get_reg (&cursor, UNW_X86_64_R11, &val[11]);
        unw_get_reg (&cursor, UNW_X86_64_R12, &val[12]);
        unw_get_reg (&cursor, UNW_X86_64_R13, &val[13]);
        unw_get_reg (&cursor, UNW_X86_64_R14, &val[14]);
        unw_get_reg (&cursor, UNW_X86_64_R15, &val[15]);
        unw_get_reg (&cursor, UNW_X86_64_RIP, &val[16]);

        if (depth == 0)
            sp_end = sp;

        unw_get_proc_info (&cursor, &pi);
        
        depth += 1;

#if 0
        fprintf (stdout, "%03d) ip {0x%016lx} {%30s + 0x%lx} - file %s at line %d - proc {0x%016lx-%016lx}\n",
                    depth, (long) ip, name, offp, file, line,
                    (long) pi.start_ip, (long) pi.end_ip);
#else
        fprintf (stdout, "%03d) (ip {0x%016lx}: %30s + 0x%lx) - proc {0x%016lx-%016lx}\n",
                depth, (long) ip, name, offp, (long) pi.start_ip, (long) pi.end_ip);
#endif

#ifdef DUMPSTACK_EXTRAREG
        fprintf(stdout, " ---Register Values--- \n"); 
        fprintf(stdout, "\tRAX{0x%016lx} RDX{0x%016lx} \n", (long)val[0], (long)val[1]);
        fprintf(stdout, "\tRCX{0x%016lx} RBX{0x%016lx} \n", (long)val[2], (long)val[3]);
        fprintf(stdout, "\tRSI{0x%016lx} RDI{0x%016lx} \n", (long)val[4], (long)val[5]);
        fprintf(stdout, "\tRBP{0x%016lx} RSP{0x%016lx} \n", (long)val[6], (long)val[7]);
        fprintf(stdout, "\tRIP{0x%016lx}\n", (long)val[16]);
        fprintf(stdout, "\t R8{0x%016lx}  R9{0x%016lx} R10{0x%016lx} R11{0x%016lx} \n", (long)val[8], (long)val[9], (long)val[10],(long) val[11]);
        fprintf(stdout, "\tR12{0x%016lx} R13{0x%016lx} R14{0x%016lx} R15{0x%016lx} \n", (long)val[12], (long)val[13], (long)val[14], (long)val[15]);
#if 0
        printf("Register Values: \n\tRAX{0x%016lx} RDX{0x%016lx} \n\tRCX{0x%016lx} RBX{0x%016lx} \n\tRSI{0x%016lx} RDI{0x%016lx} \n\tRBP{0x%016lx} RSP{0x%016lx} \n\t R8{0x%016lx} R9{0x%016lx} R10{0x%016lx} R11{0x%016lx} \n\tR12{0x%016lx} R13{0x%016lx} R14{0x%016lx} R15{0x%016lx} \n\tRIP{0x%016lx}\n",
            (long)val[0], (long)val[1], (long)val[2], (long) val[3],  (long) val[4],  (long)val[5],  (long)val[6],  (long)val[7], 
            (long)val[8], (long)val[9], (long)val[10],(long) val[11], (long) val[12], (long)val[13], (long)val[14], (long)val[15], (long)val[16]);
#endif

#endif

        if (depth > 128) {
            printf (" --- too many unwind!! \n'''");
            break;
        }
            
        sp_begin = sp;
    }
    

#ifdef DUMPSTACK_EXTRASTACK
    fprintf(stdout, "\n --- STACK from 0x%"PRIx64" to 0x%"PRIx64"--- \n", sp_begin, sp_end);
    if ((sp_begin - sp_end) > 0) {
        uint8_t count = 4;
        while (sp_begin != sp_end) {
            if (count == 4) {
                count = 0;
                fprintf(stdout, "\n %"PRIx64": ", sp_begin);
            }

            count += 1;
            void *p = sp_begin;
            fprintf(stdout, "0x%016"PRIx64" | ", *(uint64_t *)p);
            sp_begin -= 8;
        }
    }
    printf("\n --- --- --- --- --- --- --- --- \n");
#endif

    fflush(stdout);
    return 0;
}


#endif

/* Print out statistics on packets dropped */
static void
print_stats(void)
{
	uint64_t total_packets_dropped, total_packets_tx, total_packets_rx;
	unsigned portid;

	total_packets_dropped = 0;
	total_packets_tx = 0;
	total_packets_rx = 0;

	const char clr[] = { 27, '[', '2', 'J', '\0' };
	const char topLeft[] = { 27, '[', '1', ';', '1', 'H','\0' };

		/* Clear screen and move to top left */
	printf("%s%s", clr, topLeft);

	printf("\nPort statistics ====================================");

	for (portid = 0; portid < RTE_MAX_ETHPORTS; portid++) {
		/* skip disabled ports */
		if ((l2fwd_enabled_port_mask & (1 << portid)) == 0)
			continue;
		printf("\nStatistics for port %u ------------------------------"
			   "\nPackets sent: %24"PRIu64
			   "\nPackets received: %20"PRIu64
			   "\nPackets dropped: %21"PRIu64,
			   portid,
			   port_statistics[portid].tx,
			   port_statistics[portid].rx,
			   port_statistics[portid].dropped);

		total_packets_dropped += port_statistics[portid].dropped;
		total_packets_tx += port_statistics[portid].tx;
		total_packets_rx += port_statistics[portid].rx;
	}
	printf("\nAggregate statistics ==============================="
		   "\nTotal packets sent: %18"PRIu64
		   "\nTotal packets received: %14"PRIu64
		   "\nTotal packets dropped: %15"PRIu64,
		   total_packets_tx,
		   total_packets_rx,
		   total_packets_dropped);
	printf("\n====================================================\n");
}

static void
l2fwd_mac_updating(struct rte_mbuf *m, unsigned dest_portid)
{
	struct ether_hdr *eth;
	void *tmp;

	eth = rte_pktmbuf_mtod(m, struct ether_hdr *);

	/* 02:00:00:00:00:xx */
	tmp = &eth->d_addr.addr_bytes[0];
	*((uint64_t *)tmp) = 0x000000000002 + ((uint64_t)dest_portid << 40);

	/* src addr */
	ether_addr_copy(&l2fwd_ports_eth_addr[dest_portid], &eth->s_addr);
}

static void
l2fwd_simple_forward(struct rte_mbuf *m, unsigned portid)
{
	unsigned dst_port;
	int sent;
	struct rte_eth_dev_tx_buffer *buffer;

	dst_port = l2fwd_dst_ports[portid];

	if (mac_updating)
		l2fwd_mac_updating(m, dst_port);

	buffer = tx_buffer[dst_port];
	sent = rte_eth_tx_buffer(dst_port, 0, buffer, m);
	if (sent)
		port_statistics[dst_port].tx += sent;
#if 0
    else {
        rte_pktmbuf_free (m);
        return;
    }
#endif

#if 0
    if (m->nb_segs > 1)
        //printf(" m->nb_segs %u, m->next %u, m->pkt_len %u, m->data_len %u\n", m->nb_segs, m->next, m->pkt_len, m->data_len);
        //rte_pktmbuf_dump(stdout, m, 5000);
        printf(" refcnt %u\n", m->refcnt);
#endif
}

/* main processing loop */
static void
l2fwd_main_loop(void)
{
	struct rte_mbuf *pkts_burst[MAX_PKT_BURST];
	struct rte_mbuf *m;
	int sent;
	unsigned lcore_id;
	uint64_t prev_tsc, diff_tsc, cur_tsc, timer_tsc;
	unsigned i, j, portid, nb_rx;
	struct lcore_queue_conf *qconf;
	const uint64_t drain_tsc = (rte_get_tsc_hz() + US_PER_S - 1) / US_PER_S *
			BURST_TX_DRAIN_US;
	struct rte_eth_dev_tx_buffer *buffer;

	prev_tsc = 0;
	timer_tsc = 0;

	lcore_id = rte_lcore_id();
	qconf = &lcore_queue_conf[lcore_id];

	if (qconf->n_rx_port == 0) {
		RTE_LOG(INFO, L2FWD, "lcore %u has nothing to do\n", lcore_id);
		return;
	}

	RTE_LOG(INFO, L2FWD, "entering main loop on lcore %u\n", lcore_id);

	for (i = 0; i < qconf->n_rx_port; i++) {

		portid = qconf->rx_port_list[i];
		RTE_LOG(INFO, L2FWD, " -- lcoreid=%u portid=%u\n", lcore_id,
			portid);

	}

	while (!force_quit) {

		cur_tsc = rte_rdtsc();

		/*
		 * TX burst queue drain
		 */
		diff_tsc = cur_tsc - prev_tsc;
		if (unlikely(diff_tsc > drain_tsc)) {

			for (i = 0; i < qconf->n_rx_port; i++) {

				portid = l2fwd_dst_ports[qconf->rx_port_list[i]];
				buffer = tx_buffer[portid];

				sent = rte_eth_tx_buffer_flush(portid, 0, buffer);
				if (sent)
					port_statistics[portid].tx += sent;

			}

			/* if timer is enabled */
			if (timer_period > 0) {

				/* advance the timer */
				timer_tsc += diff_tsc;

				/* if timer has reached its timeout */
				if (unlikely(timer_tsc >= timer_period)) {

					/* do this only on master core */
					if (lcore_id == rte_get_master_lcore()) {
						print_stats();
						/* reset the timer */
						timer_tsc = 0;
					}
				}
			}

			prev_tsc = cur_tsc;
		}

		/*
		 * Read packet from RX queues
		 */
		for (i = 0; i < qconf->n_rx_port; i++) {

			portid = qconf->rx_port_list[i];
			nb_rx = rte_eth_rx_burst((uint8_t) portid, 0,
						 pkts_burst, MAX_PKT_BURST);

			port_statistics[portid].rx += nb_rx;

			for (j = 0; j < nb_rx; j++) {
				m = pkts_burst[j];
				rte_prefetch0(rte_pktmbuf_mtod(m, void *));
				l2fwd_simple_forward(m, portid);
			}
		}
	}
}

static int
l2fwd_launch_one_lcore(__attribute__((unused)) void *dummy)
{
#ifdef DUMPSTACK
    struct sigaction sa;
    sigfillset(&sa.sa_mask);
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = sig_pthreadhandler;
    sigaction(SIGUSR2, &sa, NULL); 

    tid[g_threads] = pthread_self();
    g_threads += 1;
#endif

	l2fwd_main_loop();
	return 0;
}

/* display usage */
static void
l2fwd_usage(const char *prgname)
{
	printf("%s [EAL options] -- -p PORTMASK [-q NQ]\n"
	       "  -p PORTMASK: hexadecimal bitmask of ports to configure\n"
	       "  -q NQ: number of queue (=ports) per lcore (default is 1)\n"
		   "  -T PERIOD: statistics will be refreshed each PERIOD seconds (0 to disable, 10 default, 86400 maximum)\n"
		   "  --[no-]mac-updating: Enable or disable MAC addresses updating (enabled by default)\n"
		   "      When enabled:\n"
		   "       - The source MAC address is replaced by the TX port MAC address\n"
		   "       - The destination MAC address is replaced by 02:00:00:00:00:TX_PORT_ID\n",
	       prgname);
}

static int
l2fwd_parse_portmask(const char *portmask)
{
	char *end = NULL;
	unsigned long pm;

	/* parse hexadecimal string */
	pm = strtoul(portmask, &end, 16);
	if ((portmask[0] == '\0') || (end == NULL) || (*end != '\0'))
		return -1;

	if (pm == 0)
		return -1;

	return pm;
}

static unsigned int
l2fwd_parse_nqueue(const char *q_arg)
{
	char *end = NULL;
	unsigned long n;

	/* parse hexadecimal string */
	n = strtoul(q_arg, &end, 10);
	if ((q_arg[0] == '\0') || (end == NULL) || (*end != '\0'))
		return 0;
	if (n == 0)
		return 0;
	if (n >= MAX_RX_QUEUE_PER_LCORE)
		return 0;

	return n;
}

static int
l2fwd_parse_timer_period(const char *q_arg)
{
	char *end = NULL;
	int n;

	/* parse number string */
	n = strtol(q_arg, &end, 10);
	if ((q_arg[0] == '\0') || (end == NULL) || (*end != '\0'))
		return -1;
	if (n >= MAX_TIMER_PERIOD)
		return -1;

	return n;
}

static const char short_options[] =
	"p:"  /* portmask */
	"q:"  /* number of queues */
	"T:"  /* timer period */
	;

#define CMD_LINE_OPT_MAC_UPDATING "mac-updating"
#define CMD_LINE_OPT_NO_MAC_UPDATING "no-mac-updating"

enum {
	/* long options mapped to a short option */

	/* first long only option value must be >= 256, so that we won't
	 * conflict with short options */
	CMD_LINE_OPT_MIN_NUM = 256,
};

static const struct option lgopts[] = {
	{ CMD_LINE_OPT_MAC_UPDATING, no_argument, &mac_updating, 1},
	{ CMD_LINE_OPT_NO_MAC_UPDATING, no_argument, &mac_updating, 0},
	{NULL, 0, 0, 0}
};

/* Parse the argument given in the command line of the application */
static int
l2fwd_parse_args(int argc, char **argv)
{
	int opt, ret, timer_secs;
	char **argvopt;
	int option_index;
	char *prgname = argv[0];

	argvopt = argv;

	while ((opt = getopt_long(argc, argvopt, short_options,
				  lgopts, &option_index)) != EOF) {

		switch (opt) {
		/* portmask */
		case 'p':
			l2fwd_enabled_port_mask = l2fwd_parse_portmask(optarg);
			if (l2fwd_enabled_port_mask == 0) {
				printf("invalid portmask\n");
				l2fwd_usage(prgname);
				return -1;
			}
			break;

		/* nqueue */
		case 'q':
			l2fwd_rx_queue_per_lcore = l2fwd_parse_nqueue(optarg);
			if (l2fwd_rx_queue_per_lcore == 0) {
				printf("invalid queue number\n");
				l2fwd_usage(prgname);
				return -1;
			}
			break;

		/* timer period */
		case 'T':
			timer_secs = l2fwd_parse_timer_period(optarg);
			if (timer_secs < 0) {
				printf("invalid timer period\n");
				l2fwd_usage(prgname);
				return -1;
			}
			timer_period = timer_secs;
			break;

		/* long options */
		case 0:
			break;

		default:
			l2fwd_usage(prgname);
			return -1;
		}
	}

	if (optind >= 0)
		argv[optind-1] = prgname;

	ret = optind-1;
	optind = 1; /* reset getopt lib */
	return ret;
}

/* Check the link status of all ports in up to 9s, and print them finally */
static void
check_all_ports_link_status(uint8_t port_num, uint32_t port_mask)
{
#define CHECK_INTERVAL 100 /* 100ms */
#define MAX_CHECK_TIME 90 /* 9s (90 * 100ms) in total */
	uint8_t portid, count, all_ports_up, print_flag = 0;
	struct rte_eth_link link;

	printf("\nChecking link status");
	fflush(stdout);
	for (count = 0; count <= MAX_CHECK_TIME; count++) {
		if (force_quit)
			return;
		all_ports_up = 1;
		for (portid = 0; portid < port_num; portid++) {
			if (force_quit)
				return;
			if ((port_mask & (1 << portid)) == 0)
				continue;
			memset(&link, 0, sizeof(link));
			rte_eth_link_get_nowait(portid, &link);
			/* print link status if flag set */
			if (print_flag == 1) {
				if (link.link_status)
					printf("Port %d Link Up - speed %u "
						"Mbps - %s\n", (uint8_t)portid,
						(unsigned)link.link_speed,
				(link.link_duplex == ETH_LINK_FULL_DUPLEX) ?
					("full-duplex") : ("half-duplex\n"));
				else
					printf("Port %d Link Down\n",
						(uint8_t)portid);
				continue;
			}
			/* clear all_ports_up flag if any link down */
			if (link.link_status == ETH_LINK_DOWN) {
				all_ports_up = 0;
				break;
			}
		}
		/* after finally printing all link status, get out */
		if (print_flag == 1)
			break;

		if (all_ports_up == 0) {
			printf(".");
			fflush(stdout);
			rte_delay_ms(CHECK_INTERVAL);
		}

		/* set the print_flag if all ports up or timeout */
		if (all_ports_up == 1 || count == (MAX_CHECK_TIME - 1)) {
			print_flag = 1;
			printf("done\n");
		}
	}
}

static void
signal_handler(int signum)
{
	if (signum == SIGINT || signum == SIGTERM) {
		printf("\n\nSignal %d received, preparing to exit...\n",
				signum);
		force_quit = true;
	}
}

int
main(int argc, char **argv)
{
	struct lcore_queue_conf *qconf;
	struct rte_eth_dev_info dev_info;
	int ret;
	uint8_t nb_ports;
	uint8_t nb_ports_available;
	uint8_t portid, last_port;
	unsigned lcore_id, rx_lcore_id;
	unsigned nb_ports_in_mask = 0;

    uint16_t mtu = 0;

    struct rte_eth_rxconf *rxconf;
    struct rte_eth_txconf *txconf;

#ifdef DUMPSTACK
    //signal(SIGINT , sig_handler);
    //signal(SIGHUP , sig_handler);
    //signal(SIGINT , sig_handler);
    //signal(SIGQUIT, sig_handler);
    //signal(SIGILL , sig_handler);
    //signal(SIGABRT, sig_handler);
    //signal(SIGKILL, sig_handler);
    signal(SIGSEGV, sig_handler);
    //signal(SIGTERM, sig_handler);
    //signal(SIGSTOP, sig_handler);
    //signal(SIGTSTP, sig_handler);
    //signal(SIGBUS , sig_handler); 
    signal(SIGUSR1 , sig_handler); 
#else
	signal(SIGINT, signal_handler);
	signal(SIGTERM, signal_handler);
#endif

	/* init EAL */
	ret = rte_eal_init(argc, argv);
	if (ret < 0)
		rte_exit(EXIT_FAILURE, "Invalid EAL arguments\n");
	argc -= ret;
	argv += ret;

#ifdef DUMPSTACK
    rte_pdump_init(NULL);
    //get_therads ();
#endif


	force_quit = false;

	/* parse application arguments (after the EAL ones) */
	ret = l2fwd_parse_args(argc, argv);
	if (ret < 0)
		rte_exit(EXIT_FAILURE, "Invalid L2FWD arguments\n");

	printf("MAC updating %s\n", mac_updating ? "enabled" : "disabled");

	/* convert to number of cycles */
	timer_period *= rte_get_timer_hz();

	/* create the mbuf pool */
	l2fwd_pktmbuf_pool = rte_pktmbuf_pool_create("mbuf_pool", NB_MBUF,
		MEMPOOL_CACHE_SIZE, 0, 
        //RTE_MBUF_DEFAULT_BUF_SIZE,
        2100,
		rte_socket_id());
	if (l2fwd_pktmbuf_pool == NULL)
		rte_exit(EXIT_FAILURE, "Cannot init mbuf pool\n");

	nb_ports = rte_eth_dev_count();
	if (nb_ports == 0)
		rte_exit(EXIT_FAILURE, "No Ethernet ports - bye\n");

	/* reset l2fwd_dst_ports */
	for (portid = 0; portid < RTE_MAX_ETHPORTS; portid++)
		l2fwd_dst_ports[portid] = 0;
	last_port = 0;

	/*
	 * Each logical core is assigned a dedicated TX queue on each port.
	 */
	for (portid = 0; portid < nb_ports; portid++) {
		/* skip ports that are not enabled */
		if ((l2fwd_enabled_port_mask & (1 << portid)) == 0)
			continue;

		if (nb_ports_in_mask % 2) {
			l2fwd_dst_ports[portid] = last_port;
			l2fwd_dst_ports[last_port] = portid;
		}
		else
			last_port = portid;

		nb_ports_in_mask++;

		rte_eth_dev_info_get(portid, &dev_info);
	}
	if (nb_ports_in_mask % 2) {
		printf("Notice: odd number of ports in portmask.\n");
		l2fwd_dst_ports[last_port] = last_port;
	}

	rx_lcore_id = 0;
	qconf = NULL;

	/* Initialize the port/queue configuration of each logical core */
	for (portid = 0; portid < nb_ports; portid++) {
		/* skip ports that are not enabled */
		if ((l2fwd_enabled_port_mask & (1 << portid)) == 0)
			continue;

		/* get the lcore_id for this port */
		while (rte_lcore_is_enabled(rx_lcore_id) == 0 ||
		       lcore_queue_conf[rx_lcore_id].n_rx_port ==
		       l2fwd_rx_queue_per_lcore) {
			rx_lcore_id++;
			if (rx_lcore_id >= RTE_MAX_LCORE)
				rte_exit(EXIT_FAILURE, "Not enough cores\n");
		}

		if (qconf != &lcore_queue_conf[rx_lcore_id])
			/* Assigned a new logical core in the loop above. */
			qconf = &lcore_queue_conf[rx_lcore_id];

		qconf->rx_port_list[qconf->n_rx_port] = portid;
		qconf->n_rx_port++;
		printf("Lcore %u: RX port %u\n", rx_lcore_id, (unsigned) portid);
	}

	nb_ports_available = nb_ports;

	/* Initialise each port */
	for (portid = 0; portid < nb_ports; portid++) {
		/* skip ports that are not enabled */
		if ((l2fwd_enabled_port_mask & (1 << portid)) == 0) {
			printf("Skipping disabled port %u\n", (unsigned) portid);
			nb_ports_available--;
			continue;
		}
		/* init port */
		printf("Initializing port %u... ", (unsigned) portid);
		fflush(stdout);
		ret = rte_eth_dev_configure(portid, 1, 1, &port_conf);
		if (ret < 0)
			rte_exit(EXIT_FAILURE, "Cannot configure device: err=%d, port=%u\n",
				  ret, (unsigned) portid);

		ret = rte_eth_dev_adjust_nb_rx_tx_desc(portid, &nb_rxd,
						       &nb_txd);
		if (ret < 0)
			rte_exit(EXIT_FAILURE,
				 "Cannot adjust number of descriptors: err=%d, port=%u\n",
				 ret, (unsigned) portid);

		rte_eth_macaddr_get(portid,&l2fwd_ports_eth_addr[portid]);

        rte_eth_dev_info_get (portid, &dev_info);
        rxconf = &dev_info.default_rxconf;
        rxconf->rx_drop_en = 1;
        txconf = &dev_info.default_txconf;
        
        printf(" before rte_eth_txconf txq_flags 0x%x\n", txconf->txq_flags);
        txconf->txq_flags = (txconf->txq_flags & ETH_TXQ_FLAGS_NOMULTSEGS)?(txconf->txq_flags ^ ETH_TXQ_FLAGS_NOMULTSEGS):txconf->txq_flags;
        printf(" after rte_eth_txconf txq_flags 0x%x\n", txconf->txq_flags);

		/* init one RX queue */
		fflush(stdout);
		ret = rte_eth_rx_queue_setup(portid, 0, nb_rxd,
					     rte_eth_dev_socket_id(portid),
					     /*NULL*/ rxconf,
					     l2fwd_pktmbuf_pool);
		if (ret < 0)
			rte_exit(EXIT_FAILURE, "rte_eth_rx_queue_setup:err=%d, port=%u\n",
				  ret, (unsigned) portid);

		/* init one TX queue on each port */
		fflush(stdout);
		ret = rte_eth_tx_queue_setup(portid, 0, nb_txd,
				rte_eth_dev_socket_id(portid),
			    txconf);
		if (ret < 0)
			rte_exit(EXIT_FAILURE, "rte_eth_tx_queue_setup:err=%d, port=%u\n",
				ret, (unsigned) portid);

		/* Initialize TX buffers */
		tx_buffer[portid] = rte_zmalloc_socket("tx_buffer",
				RTE_ETH_TX_BUFFER_SIZE(MAX_PKT_BURST), 0,
				rte_eth_dev_socket_id(portid));
		if (tx_buffer[portid] == NULL)
			rte_exit(EXIT_FAILURE, "Cannot allocate buffer for tx on port %u\n",
					(unsigned) portid);

		rte_eth_tx_buffer_init(tx_buffer[portid], MAX_PKT_BURST);

		ret = rte_eth_tx_buffer_set_err_callback(tx_buffer[portid],
				rte_eth_tx_buffer_count_callback,
				&port_statistics[portid].dropped);
		if (ret < 0)
				rte_exit(EXIT_FAILURE, "Cannot set error callback for "
						"tx buffer on port %u\n", (unsigned) portid);

		/* Start device */
		ret = rte_eth_dev_start(portid);
		if (ret < 0)
			rte_exit(EXIT_FAILURE, "rte_eth_dev_start:err=%d, port=%u\n",
				  ret, (unsigned) portid);

		printf("done: \n");

		rte_eth_promiscuous_enable(portid);

		printf("Port %u, MAC address: %02X:%02X:%02X:%02X:%02X:%02X\n\n",
				(unsigned) portid,
				l2fwd_ports_eth_addr[portid].addr_bytes[0],
				l2fwd_ports_eth_addr[portid].addr_bytes[1],
				l2fwd_ports_eth_addr[portid].addr_bytes[2],
				l2fwd_ports_eth_addr[portid].addr_bytes[3],
				l2fwd_ports_eth_addr[portid].addr_bytes[4],
				l2fwd_ports_eth_addr[portid].addr_bytes[5]);

        if (rte_eth_dev_get_mtu (portid, &mtu) == 0) {
            printf("Port %d mtu %u\n", (uint8_t)portid, mtu);
#if 1
            rte_eth_dev_stop (portid);
            if (rte_eth_dev_set_mtu (portid, 9000) == 0) {
                if (rte_eth_dev_get_mtu (portid, &mtu) == 0) 
                    printf("Port %d mtu %u\n", (uint8_t)portid, mtu);
            }
            rte_eth_dev_start (portid);
#endif
        }

		/* initialize port stats */
		memset(&port_statistics, 0, sizeof(port_statistics));
	}

	if (!nb_ports_available) {
		rte_exit(EXIT_FAILURE,
			"All available ports are disabled. Please set portmask.\n");
	}

	check_all_ports_link_status(nb_ports, l2fwd_enabled_port_mask);

	ret = 0;
	/* launch per-lcore init on every lcore */
	rte_eal_mp_remote_launch(l2fwd_launch_one_lcore, NULL, CALL_MASTER);
	RTE_LCORE_FOREACH_SLAVE(lcore_id) {
		if (rte_eal_wait_lcore(lcore_id) < 0) {
			ret = -1;
			break;
		}
	}

	for (portid = 0; portid < nb_ports; portid++) {
		if ((l2fwd_enabled_port_mask & (1 << portid)) == 0)
			continue;
		printf("Closing port %d...", portid);
		rte_eth_dev_stop(portid);
		rte_eth_dev_close(portid);
		printf(" Done\n");
	}
	printf("Bye...\n");

	return ret;
}
