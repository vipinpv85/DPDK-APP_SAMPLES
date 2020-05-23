#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <stdarg.h>
#include <inttypes.h>
#include <sys/queue.h>
#include <stdlib.h>
#include <getopt.h>

#include <sys/time.h>

#include <rte_debug.h>
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
#include <rte_hash_crc.h>
#include <rte_tailq.h>
#include <rte_malloc.h>
#include <rte_mempool.h>
#include <rte_branch_prediction.h>
#include <rte_jhash.h>
#include <rte_alarm.h>

#define MAX_MONITOR (20)

#ifdef CRCHASH
#define MYHASH rte_hash_crc 
#else
#define MYHASH rte_jhash
#endif

static FILE *fptr = NULL;

int ret;
uint8_t quit = 1, i = 0;
char region[100] = {0};
const struct rte_memzone *mzPtr = NULL;
char usrMenu = '\0';
unsigned lcore_id = 0;

static uint8_t console = 1;
static uint8_t monitor = 1;
static uint8_t displaymenu = 0;

static int8_t count = MAX_MONITOR;
static uint32_t *crcvalues_ptr[MAX_MONITOR];
static uint32_t crcvalues_index[MAX_MONITOR];
static struct rte_timer timer[MAX_MONITOR];

typedef struct 
{
	uint8_t index;
	const struct rte_memzone *mzPtr;
	int32_t avgLen;
	uint32_t spillLen;
	uint32_t temp;
} timerArgs_t;
static timerArgs_t timerArgs[MAX_MONITOR];


void usrEnter (void);
void displayMenu (void); 
static int lcore_main (__attribute__((unused))void *arg);
static void monitor_timer_cb (__attribute__((unused)) struct rte_timer *tim, void *arg);
static void (alarm_callback) (__attribute__((unused)) void *arg);

static void alarm_callback (__attribute__((unused)) void *arg)
{
	fflush(stdout);
	printf("\033[2J");
	printf("\033[2J");

	displayMenu ();
	while(quit) 
	{
		usrMenu = getchar();

		switch (usrMenu) {
			case '1':
				displaymenu = 0;
				printf("\033[2J");
				fprintf(stdout, "\n -------- registered monitor --------\n");
				for (i = MAX_MONITOR - 1; 1>= 0; i--) {
					if (timerArgs[i].mzPtr == NULL)
						break;
					fprintf(stdout, " - %d - %s\n", (MAX_MONITOR - i), timerArgs[count].mzPtr->name);
				}
				fprintf(stdout, " ------------------------------------\n\n");
				rte_memzone_dump (stdout);
				//rte_dump_tailq (stdout);
				//rte_malloc_dump_stats (stdout, NULL);
				//rte_mempool_list_dump (stdout);
				rte_dump_physmem_layout (stdout);
				printf(" Press Key to continue -----\n");
				getchar();
				getchar();
				printf("\033[2J");
				displaymenu = 1;
				displayMenu ();
				break;

			case '2':
				displaymenu = 0;
				printf("\033[2J");
				printf("\nEnter the region to monitor:");
				getchar();
				scanf("%[^\n]%*c", region);
				//fgets (region, 100, stdin);
				region[99] = '\0';
				
				if (count >= 0) {
					mzPtr =  rte_memzone_lookup ((const char*)region);
					if (mzPtr) {
						count -= 1;
						timerArgs[count].index = count;
						timerArgs[count].mzPtr = mzPtr;
						
						rte_timer_reset(&timer[count], rte_get_timer_hz()/10000, PERIODICAL, lcore_id, monitor_timer_cb, &timerArgs[count]);
						
						printf("Added Memzone region : %s\n ", region);
					}
					else {
						printf(" Memzone details not found......!!!\n");
					}
					printf(" Press Key to continue -----\n");
				}
				else {
					printf(" There are no slots for monitor, Press Key to continue -----\n");
				}
				getchar();
				printf("\033[2J");
				displaymenu = 1;
				displayMenu ();
				break;

			case '3':
				monitor = 0;
				printf("\033[10:15H                                        ");
				break;

			case '4':
				monitor = 1;
				printf("\033[10:15H                                        ");
				break;

			case '5':
				console = (console == 1)?0:1;
				printf("\033[2J");
				printf("\n Toggle console and file, press to continue!!");
				getchar();
				getchar();
				printf("\033[2J");
				displayMenu ();
				break;

			case '6':
				quit = 0;
				printf("\033[2J");
				for (i = 0; i < MAX_MONITOR; i++) {
					if (timer[i].f != NULL) {
						printf("\nstopping timer in index %d\n", i);
						rte_timer_stop (&timer[i]);
					}
				}
				fclose(fptr); 
				printf("\033[2J");
				printf("\033[5;1H -------------------------");
				printf("\033[6;4H Stoppping all monitor and exiting!!");
				printf("\033[7;1H -------------------------");
				printf("\n\n");
				exit (1);
		}
	}

	rte_eal_alarm_set(rte_get_timer_hz()/10000, alarm_callback, NULL);
}

static void monitor_timer_cb(__attribute__((unused)) struct rte_timer *tim, void *arg)
{
	timerArgs_t *timerArgs = (timerArgs_t *) arg;
	struct timeval tv;
	uint32_t tempcrc = 0x0;
	int i = 0;
	uint64_t *dataptr = NULL;


	if (monitor) {
		gettimeofday( &tv, NULL );

		if (timerArgs->mzPtr) {

			if (unlikely(crcvalues_ptr[timerArgs->index] == NULL)) {
				timerArgs->avgLen = timerArgs->mzPtr->len / RTE_CACHE_LINE_SIZE;
				timerArgs->spillLen = (timerArgs->mzPtr->len - (RTE_CACHE_LINE_SIZE * timerArgs->avgLen));
				timerArgs->temp = timerArgs->avgLen;

				timerArgs->temp |= timerArgs->temp >>1;
				timerArgs->temp |= timerArgs->temp >>2;
				timerArgs->temp |= timerArgs->temp >>4;
				timerArgs->temp |= timerArgs->temp >>8;
				timerArgs->temp |= timerArgs->temp >>16;
				timerArgs->temp += 1;

				crcvalues_ptr[timerArgs->index] = (uint32_t *) calloc(timerArgs->temp, RTE_CACHE_LINE_SIZE);
				crcvalues_index[timerArgs->index] = timerArgs->temp;

				i = 0;
				dataptr = (uint64_t *)timerArgs->mzPtr->addr;
				for (; i < timerArgs->avgLen; dataptr += 8, i++) {
					*(crcvalues_ptr[timerArgs->index] + i) = MYHASH((unsigned char *)dataptr, RTE_CACHE_LINE_SIZE, 0xBADA);
					
				}

				/* allow check for spill over bytes */
				if (timerArgs->spillLen) {
					*(crcvalues_ptr[timerArgs->index] + i) = MYHASH((unsigned char *)dataptr, timerArgs->spillLen, 0xBADA);
				}

				return;
			}

			i = 0;
			dataptr = (uint64_t *)timerArgs->mzPtr->addr;
			for (; i < timerArgs->avgLen; dataptr = dataptr + 8, i++) {
				tempcrc = MYHASH((unsigned char *)dataptr, RTE_CACHE_LINE_SIZE, 0xBADA);
				if (tempcrc != *(crcvalues_ptr[timerArgs->index] + i)) {
					if (console) {
						if (displaymenu) {
							fprintf(stdout, "\033[%d;2H%d - (%s) of len (%zd) at offset (%u) with data (%"PRIx64",%"PRIx64",%"PRIx64",%"PRIx64",%"PRIx64",%"PRIx64",%"PRIx64",%"PRIx64")",
							timerArgs->index + 11, timerArgs->index,
							timerArgs->mzPtr->name, timerArgs->mzPtr->len,
							i * RTE_CACHE_LINE_SIZE,
							*(dataptr + 0), *(dataptr + 1), *(dataptr + 2), *(dataptr + 3), 
							*(dataptr + 4), *(dataptr + 5), *(dataptr + 6), *(dataptr + 7));
						}
					} else
						fprintf(fptr, "%"PRIu64" %d - (%s) of len (%zd) at offset (%u) with data (%"PRIx64",%"PRIx64",%"PRIx64",%"PRIx64",%"PRIx64",%"PRIx64",%"PRIx64",%"PRIx64")\n",
							rte_get_tsc_cycles(),
							timerArgs->index, timerArgs->mzPtr->name, timerArgs->mzPtr->len,
							i * RTE_CACHE_LINE_SIZE,
							*(dataptr + 0), *(dataptr + 1), *(dataptr + 2), *(dataptr + 3), 
							*(dataptr + 4), *(dataptr + 5), *(dataptr + 6), *(dataptr + 7));
				}
			}

			if (timerArgs->spillLen) {
				tempcrc = MYHASH((unsigned char *)dataptr, timerArgs->spillLen, 0xBADA);
				if (tempcrc != *(crcvalues_ptr[timerArgs->index] + i)) {
					if (console) {
						if (displaymenu) {
							fprintf(stdout, "\033[%d;2H%d - (%s) of len (%zd) at offset (%u) with data (%"PRIx64",%"PRIx64",%"PRIx64",%"PRIx64",%"PRIx64",%"PRIx64",%"PRIx64",%"PRIx64")",
							timerArgs->index + 11, timerArgs->index,
							timerArgs->mzPtr->name, timerArgs->mzPtr->len,
							i * RTE_CACHE_LINE_SIZE,
							*(dataptr + 0), *(dataptr + 1), *(dataptr + 2), *(dataptr + 3), 
							*(dataptr + 4), *(dataptr + 5), *(dataptr + 6), *(dataptr + 7));
						}
					} else
						fprintf(fptr, "%"PRIu64" %d - (%s) of len (%zd) at offset (%u) with data (%"PRIx64",%"PRIx64",%"PRIx64",%"PRIx64",%"PRIx64",%"PRIx64",%"PRIx64",%"PRIx64")\n",
							rte_get_tsc_cycles(),
							timerArgs->index, timerArgs->mzPtr->name, timerArgs->mzPtr->len,
							i * RTE_CACHE_LINE_SIZE,
							*(dataptr + 0), *(dataptr + 1), *(dataptr + 2), *(dataptr + 3), 
							*(dataptr + 4), *(dataptr + 5), *(dataptr + 6), *(dataptr + 7));
	
				}
			}
		}
	}
	fflush(stdout);
}

static int lcore_main (__attribute__((unused))void *arg)
{
	fflush(stdout);

	while (1)
	{
		if (monitor)
			rte_timer_manage();
	}

	return 0;
}

void displayMenu (void) 
{
	printf("\033[2;2H MONITOR options : 1) DISPLAY, 2) SELECT, 3) STOP, 4) START, 5) to file (monitor.log) or console, 6) quit");
	printf("\033[3;2HEnter Choice:");
}

void usrEnter(void)
{
	printf(" \033[78;1H ENTER CHOICE:");
	return;
}


int main(void)
{
	char appname[] = "memory-monitor";
	char c_flag[] = "--lcores";
	char n_flag[] = "(1-2)@(10)";
	char mp_flag[] = "--proc-type=secondary";
	char nopci_flag[] = "--no-pci";
	char nohpet_flag[] = "--no-hpet";
	char *argp[6];

	argp[0] = appname;
	argp[1] = c_flag;
	argp[2] = n_flag;
	argp[3] = mp_flag;
	argp[4] = nopci_flag;
	argp[5] = nohpet_flag;

	memset(timer, 0, sizeof(timer));
	memset(timerArgs, 0, sizeof(timerArgs));

	fptr = fopen("monitor.log", "w");
	if (fptr == NULL)
		rte_exit(EXIT_FAILURE, "Failed to open file!!!\n");


	ret = rte_eal_init(6, argp);
	if (ret < 0)
		rte_panic("Cannot init EAL\n");

	if (!rte_eal_primary_proc_alive(NULL))
		rte_exit(EXIT_FAILURE, "No primary DPDK process is running.\n");

	rte_timer_subsystem_init();
#ifdef CRCHASH
	rte_hash_crc_set_alg (CRC32_SSE42|CRC32_SSE42_x64);
#endif

	for (i = 0; i <MAX_MONITOR; i++)
		rte_timer_init(&(timer[i]));

	/*worker*/
	lcore_id = rte_get_next_lcore(rte_lcore_id(), 0, 0);
	rte_eal_remote_launch((lcore_function_t *)lcore_main, NULL, lcore_id),

	rte_eal_alarm_set(rte_get_timer_hz()/10000, alarm_callback, NULL);
	while (1);

	return 0;
}
