#include <signal.h>
#include <inttypes.h>

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <sys/queue.h>

#include <rte_common.h>
#include <rte_memory.h>
#include <rte_memzone.h>
#include <rte_launch.h>
#include <rte_eal.h>
#include <rte_per_lcore.h>
#include <rte_lcore.h>
#include <rte_cycles.h>
#include <rte_timer.h>
#include <rte_debug.h>

#define TIMER_RESOLUTION_CYCLES 20000000ULL /* around 10ms at 2 Ghz */

//#define TIMERS 10000
#define TIMERS 1

static struct rte_timer timer0[TIMERS];
static struct rte_timer timer1[TIMERS];

static uint64_t count0[TIMERS];
static uint64_t count1[TIMERS];

/* timer0 callback */
static void
timer0_cb(__attribute__((unused)) struct rte_timer *tim,
	  __attribute__((unused)) void *arg)
{
	unsigned lcore_id = rte_lcore_id();

	//int32_t id = *(int32_t *)arg;
	//printf("%s()%d on lcore %u timer: expire %"PRIu64 " period: %d status - state %d owner %d\n", __func__, 
		//id, lcore_id, tim->expire, tim->period, tim->status.state, tim->status.owner);

	count0[*(int16_t *)arg]++;

	//if (rte_timer_reset(tim, (1 * rte_get_timer_hz())/10000000L, SINGLE, lcore_id, timer0_cb, arg) != 0)
	if (rte_timer_reset(tim, 1, SINGLE, lcore_id, timer0_cb, arg) != 0)
	{
		rte_panic("%s failed for timer %d!!!\n", __func__, *(uint16_t *)arg);
	}
}

/* timer1 callback */
static void
timer1_cb(__attribute__((unused)) struct rte_timer *tim,
	  __attribute__((unused)) void *arg)
{
	unsigned lcore_id = rte_lcore_id();

	//int32_t id = *(int16_t *)arg;
	//printf("%s()%d on lcore %u timer: expire %"PRIu64 " period: %d status - state %d owner %d\n", __func__, 
			//id, lcore_id, tim->expire, tim->period, tim->status.state, tim->status.owner);

	count1[*(int16_t *)arg]++;

	//if (rte_timer_reset(tim, (1000000 * rte_get_timer_hz())/10000000L, SINGLE, lcore_id, timer1_cb, arg) != 0)
	//if (rte_timer_reset(tim, 999999, SINGLE, lcore_id, timer1_cb, arg) != 0)
	if (rte_timer_reset(tim, 2199916, SINGLE, lcore_id, timer1_cb, arg) != 0)
	{
		rte_panic("%s failed for timer %d!!!\n", __func__, *(uint16_t *)arg);
	}
}


void displayStats (int sig)
{
	unsigned i = 0;
	//printf("\033[2J");
	for (i = 0; i < TIMERS; i++) {
		printf("\n count-id %d 0 %d 1 %d ", i, count0[i], count1[i]);
	}
}

static __attribute__((noreturn)) int
lcore_mainloop(__attribute__((unused)) void *arg)
{
	uint64_t prev_tsc = 0, cur_tsc, diff_tsc;

	while (1) {
		rte_timer_manage();

		/*job process delay*/
		rte_delay_us_block (1000);
	}
}

int
main(int argc, char **argv)
{
	int ret;
	uint64_t hz;
	unsigned lcore_id;

	int32_t i, callId[TIMERS];

	signal(SIGUSR1, displayStats);

	/* init EAL */
	ret = rte_eal_init(argc, argv);
	if (ret < 0)
		rte_panic("Cannot init EAL\n");

	/* init RTE timer library */
	rte_timer_subsystem_init();

	/* load timer0, every second, on master lcore, reloaded automatically */
	hz = rte_get_timer_hz();
	lcore_id = rte_lcore_id();
	printf("\n hz %"PRIu64" current core id %d\n", hz, lcore_id);

	/* init timer structures */
	for (i = 0; i<TIMERS; i++) {
		callId[i] = i;
		rte_timer_init(&(timer0[i]));
		rte_timer_init(&(timer1[i]));
		//rte_timer_reset(&timer0[i], (1 * rte_get_timer_hz())/10000000L, SINGLE, lcore_id, timer0_cb, &callId[i]);
		//rte_timer_reset(&timer1[i], (1000000 * rte_get_timer_hz())/10000000L, SINGLE, lcore_id, timer1_cb, &callId[i]);
		rte_timer_reset(&timer0[i], 1, SINGLE, lcore_id, timer0_cb, &callId[i]);
		//rte_timer_reset(&timer1[i], 999999, SINGLE, lcore_id, timer1_cb, &callId[i]);
		rte_timer_reset(&timer1[i], 2199916, SINGLE, lcore_id, timer1_cb, &callId[i]);
	}

	/* call it on master lcore too */
	(void) lcore_mainloop(NULL);

	return 0;
}
