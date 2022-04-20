#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <sys/queue.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <assert.h>

#include <immintrin.h>

#include <rte_memory.h>
#include <rte_launch.h>
#include <rte_eal.h>
#include <rte_per_lcore.h>
#include <rte_lcore.h>
#include <rte_debug.h>
#include <rte_malloc.h>
#include <rte_memcpy.h>

char *src;
char *dst;
static uint32_t numMemGb = 4; /* in GB*/
static long long unsigned MemPerThread; /* in GB/therad */
long long unsigned total[10];
static long long unsigned tickCount[256][16] __attribute__ ((aligned (16)));

static void __attribute__ ((noinline)) v1_CopyCompare(const int offsetIndex)
{
        const int offIndex = offsetIndex;

        void *ptr = memcpy(dst + offIndex * MemPerThread,
                src + offIndex * MemPerThread,
                MemPerThread);

        assert(ptr == (dst + offIndex * MemPerThread));
}

static void __attribute__ ((noinline)) v2_CopyCompare(const int offsetIndex)
{
        const int offIndex = offsetIndex;
        const unsigned long long int pseudoIndex = offIndex * MemPerThread;

        const char *vsrc1 = (const char *)src + pseudoIndex;
        const char *vdst1 = (const char *)dst + pseudoIndex;
        const char *vsrc2 = (const char *)src + pseudoIndex + 16;
        const char *vdst2 = (const char *)dst + pseudoIndex + 16;
        const char *vsrc3 = (const char *)src + pseudoIndex + 32;
        const char *vdst3 = (const char *)dst + pseudoIndex + 32;
        const char *vsrc4 = (const char *)src + pseudoIndex + 48;
        const char *vdst4 = (const char *)dst + pseudoIndex + 48;

        //for (unsigned long long int i = 0; i < MemPerThread; i += 16)
        //for (unsigned long long int i = 0; i < MemPerThread; i += 32)
        for (unsigned long long int i = 0; i < MemPerThread; i += 64)
        {
                const __m128i a1 = _mm_stream_load_si128 ((__m128i const * )(vsrc1 + i));
                const __m128i a2 = _mm_stream_load_si128 ((__m128i const * )(vsrc1 + i + 16));
                const __m128i a3 = _mm_stream_load_si128 ((__m128i const * )(vsrc1 + i + 32));
                const __m128i a4 = _mm_stream_load_si128 ((__m128i const * )(vsrc1 + i + 48));
                _mm_stream_si128 ((__m128i *)(vdst1 + i), a1);
                _mm_stream_si128 ((__m128i *)(vdst1 + i + 16), a2);
                _mm_stream_si128 ((__m128i *)(vdst1 + i + 32), a3);
                _mm_stream_si128 ((__m128i *)(vdst1 + i + 48), a4);
        }
}

static void __attribute__ ((noinline)) v3_CopyCompare(const int offsetIndex)
{
        const int offIndex = offsetIndex;

        const unsigned long long int pseudoIndex = offIndex * MemPerThread;
        const char *vsrc1 = (const char *)src + pseudoIndex;
        const char *vdst1 = (const char *)dst + pseudoIndex;
        const char *vsrc2 = (const char *)src + pseudoIndex + 32;
        const char *vdst2 = (const char *)dst + pseudoIndex + 32;

        //for (unsigned long long int i = 0; i < MemPerThread; i += 32)
        for (unsigned long long int i = 0; i < MemPerThread; i += 64)
        {
                const __m256i a1 = _mm256_stream_load_si256 ((__m128i const * )(vsrc1 + i));
                const __m256i a2 = _mm256_stream_load_si256 ((__m128i const * )(vsrc1 + i + 32));
                _mm256_stream_si256 ((__m256i *)(vdst1 + i), a1);
                _mm256_stream_si256 ((__m256i *)(vdst1 + i + 32), a2);
        }
}

static void __attribute__ ((noinline)) v4_CopyCompare(const int offsetIndex)
{
        const int offIndex = offsetIndex;

        void *ptr = rte_memcpy(dst + offIndex * MemPerThread, src + offIndex * MemPerThread, MemPerThread);
        assert(ptr == (dst + offIndex * MemPerThread));
}

static void __attribute__ ((noinline)) v5_CopyCompare(const int offsetIndex)
{
        const int offIndex = offsetIndex;

        const unsigned long long int pseudoIndex = offIndex * MemPerThread;
        const char *vsrc1 = (const char *)src + pseudoIndex;
        const char *vsrc2 = (const char *)src + pseudoIndex + 32;
        const char *vdst1 = (const char *)dst + pseudoIndex;
        const char *vdst2 = (const char *)dst + pseudoIndex + 32;

        for (unsigned long long int i = 0; i < MemPerThread; i += 64)
        {
                const __m256i a1 = _mm256_loadu_si256((__m256i const * )(vsrc1 + i));
                const __m256i a2 = _mm256_loadu_si256((__m256i const * )(vsrc2 + i));
                _mm256_storeu_si256((__m256i *)(vdst1 + i), a1);
                _mm256_storeu_si256((__m256i *)(vdst2 + i), a2);
        }
}

/* Launch a function on lcore. 8< */
static int
lcore_hello(__rte_unused void *arg)
{
        unsigned lcore_id, lcore_index;
        unsigned long long start;

        lcore_id = rte_lcore_id();
        lcore_index = rte_lcore_index(lcore_id);
        printf("hello from core %u\n", lcore_id);

        do {
                /* v1 */
                start = rte_get_timer_cycles();
                v1_CopyCompare(lcore_index);
                tickCount[lcore_index][0] = rte_get_timer_cycles() - start;

                /* v2 */
                start = rte_get_timer_cycles();
                v2_CopyCompare(lcore_index);
                tickCount[lcore_index][1] = rte_get_timer_cycles() - start;

                /* v3 */
                start = rte_get_timer_cycles();
                v3_CopyCompare(lcore_index);
                tickCount[lcore_index][2] = rte_get_timer_cycles() - start;

                /* v4 */
                start = rte_get_timer_cycles();
                v4_CopyCompare(lcore_index);
                tickCount[lcore_index][3] = rte_get_timer_cycles() - start;

                /* v5 */
                start = rte_get_timer_cycles();
                v5_CopyCompare(lcore_index);
                tickCount[lcore_index][4] = rte_get_timer_cycles() - start;

        } while(0);

        return 0;
}
/* >8 End of launching function on lcore. */

void display(void)
{
        /* display stats */
        printf("\033[2J");
        printf("\033[1;1H ---- Summary in ticks -----");
        int i = 0;
        long long unsigned total[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        const numThreads = rte_lcore_count();

        for (; i < numThreads; i++) {
                printf("\033[%d;1H Thread-%03d: v1 (%10llu), v2 (%10llu), v3 (%10llu), v4 (%10llu), v5 (%10llu)",
                        i + 2, i, tickCount[i][0], tickCount[i][1], tickCount[i][2], tickCount[i][3], tickCount[i][4]);
                total[0] += tickCount[i][0];
                total[1] += tickCount[i][1];
                total[2] += tickCount[i][2];
                total[3] += tickCount[i][3];
                total[4] += tickCount[i][4];
        }
        printf("\033[%d;1H --- with Pstate ticks can be unreliable ---", i + 2);
        printf("\033[%d;1H Total cycles: %llu, %llu, %llu, %llu, %llu",
                i + 3, total[0], total[1], total[2], total[3], total[4]);
        printf("\033[%d;1H speedup %%: %f, %f, %f, %f, %f",
                i + 4,
                ((double)total[0] / (double)total[0]),
                ((double)total[0] / (double)total[1]),
                ((double)total[0] / (double)total[2]),
                ((double)total[0] / (double)total[3]),
                ((double)total[0] / (double)total[4])
                );
        printf("\033[%d;1H ---------------------------------", i + 5);
        printf("\n\n");
}

/* Initialization of Environment Abstraction Layer (EAL). 8< */
int
main(int argc, char **argv)
{
        int ret;
        unsigned lcore_id;

        ret = rte_eal_init(argc, argv);
        if (ret < 0)
                rte_panic("Cannot init EAL\n");
        /* >8 End of initialization of Environment Abstraction Layer */

        MemPerThread = (unsigned long long) (numMemGb * (1UL << 30)) / rte_lcore_count();
        printf("lcore count (%d) Memory per thread (%lluB)\n", rte_lcore_count(), MemPerThread);

        src = (char *) rte_zmalloc_socket("src", numMemGb * (1UL << 30), 0, rte_socket_id());
        dst = (char *) rte_zmalloc_socket("dst", numMemGb * (1UL << 30), 0, rte_socket_id());

        if ((src == NULL) || (dst == NULL)) {
                rte_panic("failed to get memory for src %p dst %p\n", src, dst);
        }

        /* fill src with random value */
        int rnd=open("/dev/urandom", O_RDONLY);
        int64_t readSize = 0, temp = 0;
        do {
                temp = read(rnd, src + readSize, (numMemGb * (1UL << 30)) - readSize);
                if (temp == -1) {
                        fprintf(stderr, "ERR: temp %lu readSize %lu \n", temp, readSize);
                }
                readSize += temp;
                //fprintf(stdout, " current read %lu temp %lu target %lu\n", readSize, temp, 1UL << 34);
        } while(readSize < (1UL << 30) * numMemGb);
        close(rnd);

        /* Launches the function on each lcore. 8< */
        RTE_LCORE_FOREACH_WORKER(lcore_id) {
                /* Simpler equivalent. 8< */
                rte_eal_remote_launch(lcore_hello, NULL, lcore_id);
                /* >8 End of simpler equivalent. */
        }

        /* call it on main lcore too */
        lcore_hello(NULL);
        /* >8 End of launching the function on each lcore. */

        rte_eal_mp_wait_lcore();

        /* clean up the EAL */
                ((double)total[0] / (double)total[4])
                );
        printf("\033[%d;1H ---------------------------------", i + 5);
        printf("\n\n");
}

/* Initialization of Environment Abstraction Layer (EAL). 8< */
int
main(int argc, char **argv)
{
        int ret;
        unsigned lcore_id;

        ret = rte_eal_init(argc, argv);
        if (ret < 0)
                rte_panic("Cannot init EAL\n");
        /* >8 End of initialization of Environment Abstraction Layer */

        MemPerThread = (unsigned long long) (numMemGb * (1UL << 30)) / rte_lcore_count();
        printf("lcore count (%d) Memory per thread (%lluB)\n", rte_lcore_count(), MemPerThread);

        src = (char *) rte_zmalloc_socket("src", numMemGb * (1UL << 30), 0, rte_socket_id());
        dst = (char *) rte_zmalloc_socket("dst", numMemGb * (1UL << 30), 0, rte_socket_id());

        if ((src == NULL) || (dst == NULL)) {
                rte_panic("failed to get memory for src %p dst %p\n", src, dst);
        }

        /* fill src with random value */
        int rnd=open("/dev/urandom", O_RDONLY);
        int64_t readSize = 0, temp = 0;
        do {
                temp = read(rnd, src + readSize, (numMemGb * (1UL << 30)) - readSize);
                if (temp == -1) {
                        fprintf(stderr, "ERR: temp %lu readSize %lu \n", temp, readSize);
                }
                readSize += temp;
                //fprintf(stdout, " current read %lu temp %lu target %lu\n", readSize, temp, 1UL << 34);
        } while(readSize < (1UL << 30) * numMemGb);
        close(rnd);

        /* Launches the function on each lcore. 8< */
        RTE_LCORE_FOREACH_WORKER(lcore_id) {
                /* Simpler equivalent. 8< */
                rte_eal_remote_launch(lcore_hello, NULL, lcore_id);
                /* >8 End of simpler equivalent. */
        }

        /* call it on main lcore too */
        lcore_hello(NULL);
        /* >8 End of launching the function on each lcore. */

        rte_eal_mp_wait_lcore();

        /* clean up the EAL */
        rte_eal_cleanup();

        display();

        return 0;
}
