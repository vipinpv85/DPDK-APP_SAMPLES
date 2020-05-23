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

static struct rte_timer timer0;
static struct rte_timer timer1;

/* timer0 callback */
static void
timer0_cb(__attribute__((unused)) struct rte_timer *tim,
          __attribute__((unused)) void *arg)
{
        static unsigned counter = 0;
        unsigned lcore_id = rte_lcore_id();

        printf("%s() count: %d on lcore %u timer: expire %"PRIu64 " period: %d status - state %d owner %d\n",
                __func__, counter++, lcore_id,
                tim->expire, tim->period, tim->status.state, tim->status.owner);

        if (rte_timer_reset(tim, 1, SINGLE, lcore_id, timer0_cb, NULL) != 0)
        {
                rte_timer_stop(tim);
                rte_panic("%s failed!!!\n", __func__);
        }
}

/* timer1 callback */
static void
timer1_cb(__attribute__((unused)) struct rte_timer *tim,
          __attribute__((unused)) void *arg)
{
        static unsigned counter = 0;
        unsigned lcore_id = rte_lcore_id();

        printf("%s() count: %d on lcore %u timer: expire %"PRIu64 " period: %d status - state %d owner %d\n",
                __func__, counter++, lcore_id,
                tim->expire, tim->period, tim->status.state, tim->status.owner);

        if (rte_timer_reset(tim, 2, SINGLE, lcore_id, timer1_cb, NULL) != 0)
        {
                rte_timer_stop(tim);
                rte_panic("%s failed!!!\n", __func__);
        }
}

static __attribute__((noreturn)) int
lcore_mainloop(__attribute__((unused)) void *arg)
{
        uint64_t prev_tsc = 0, cur_tsc, diff_tsc;
        unsigned lcore_id;

        lcore_id = rte_lcore_id();
        printf("Starting mainloop on core %u\n", lcore_id);

        while (1) {
                rte_timer_manage();
                //rte_delay_us_block (1);
        }
}

int
main(int argc, char **argv)
{
        int ret;
        uint64_t hz;
        unsigned lcore_id;

        /* init EAL */
        ret = rte_eal_init(argc, argv);
        if (ret < 0)
                rte_panic("Cannot init EAL\n");

        /* init RTE timer library */
        rte_timer_subsystem_init();

        /* init timer structures */
        rte_timer_init(&timer0);
        rte_timer_init(&timer1);

        /* load timer0, every second, on master lcore, reloaded automatically */
        hz = rte_get_timer_hz();
        lcore_id = rte_lcore_id();

        printf("\n hz %"PRIu64" current core id %d\n", hz, lcore_id);

        rte_timer_reset(&timer0, 1, SINGLE, lcore_id, timer0_cb, NULL);
        rte_timer_reset(&timer1, 2, SINGLE, lcore_id, timer1_cb, NULL);
        //rte_timer_reset(&timer0, 1, PERIODICAL, lcore_id, timer0_cb, NULL);
        //rte_timer_reset(&timer1, 1, PERIODICAL, lcore_id, timer1_cb, NULL);
        /* call it on master lcore too */
        (void) lcore_mainloop(NULL);

        return 0;
}
