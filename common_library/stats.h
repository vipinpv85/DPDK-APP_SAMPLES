#ifndef __STATS_H_
#define __STATS_H_

#include <inttypes.h>
#include <signal.h>

#include "config.h"
#include "node.h"

#include <rte_timer.h>
#include <rte_cycles.h>

#define MAX_FIFO 32

#define STATS_CLR_SCREEN   printf("\033[2J")
#define STATS_ROW(x)       "\033[x;1H"
#define STATS_ROW_COL(x,y) "\033[x;yH"
#define STATS_POS_OFFSET   2

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

typedef struct pkt_stats_s {
  uint64_t port_rxPkts[16];
  uint64_t port_txPkts[16];
  uint64_t port_rxBytes[16];
  uint64_t port_txBytes[16];
  uint64_t port_rxMissed[16];
  uint64_t port_rxErr[16];
  uint64_t port_txErr[16];
  uint64_t port_rxNoMbuff[16];

  uint64_t rx_ipv4[16];
  uint64_t rx_ipv6[16];
  uint64_t non_ip[16];
  uint64_t ipFrag[16];
  uint64_t ipCsumErr[16];
  uint64_t dropped[16];

  void *extraStats;
} pkt_stats_t; /* per interface */

typedef struct wrk_stats_s {
  uint64_t wrk_rxPkts[16];
  uint64_t wrk_txPkts[16];
  uint64_t wrk_rxBytes[16];
  uint64_t wrk_txBytes[16];
  uint64_t wrk_rxErr[16];
  uint64_t wrk_txErr[16];
  uint64_t wrk_drop[16];

  void *extraStats;

  char name[16][25];
} wrk_stats_t; /* per interface */

void get_link_stats(__attribute__((unused)) struct rte_timer *t, 
                    __attribute__((unused)) void *arg);
void get_process_stats(__attribute__((unused)) struct rte_timer *t, 
                       __attribute__((unused)) void *arg);

void show_static_prt_display(void);
void show_static_wrk_display(void);

void get_process_wrk_stats(void);

void set_stats_timer(void);
void sigExtraStats(int signo);
void sigDetails(int signo);

#endif /* __STATS_H__ */
