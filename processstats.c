#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/sysinfo.h>

int main (int argc, char *argv[])
{
    int res = 0;
    char buf[1000];
    char *stat_param[5] = {"utime", "stime", "cutime", "cstime", "starttime"};
    char *stat_result[5] = {0};

    struct sysinfo info = {0};

    if ( argc != 2 ) {
        fprintf(stdout, " usage: %s ProcessId \n", __FILE__);
        return 1;
    }

    fprintf(stdout, "Process to fetch stat: %s\n", argv[1]);

    if (sysinfo(&info) == 0) {
        fprintf(stdout, "sysinfo \n");

#if 0
unsigned long totalhigh; /* Total high memory size */
unsigned long freehigh;  /* Available high memory size */
unsigned int mem_unit;   /* Memory unit size in bytes */
char _f[20-2*sizeof(long)-sizeof(int)]; /* Padding to 64 bytes */
#endif

        fprintf(stdout, "uptime: %lu\n", info.uptime);
        fprintf(stdout, "loads: 1min (%lu) 5min (%lu) 15min (%lu)\n", info.loads[0], info.loads[1], info.loads[2]);
        fprintf(stdout, "RAM: free (%lu) shared (%lu) buffer (%lu)\n", info.freeram, info.sharedram, info.bufferram);
        fprintf(stdout, "swap: total (%lu) free (%lu)\n", info.totalswap, info.freeswap);
        fprintf(stdout, "procs: %d\n", info.procs);

    }

    sprintf(buf, "cat /proc/%s/stat | awk \'{print $14 \",\" $15 \",\" $16 \",\" $17 \",\" $22}\'", argv[1]);
    fp = popen(buf, "r");
    if (fp) {
        char *parse = fgets (buf, 999, fp);
        char *p = strtok (parse, ",");

        res = 0;
        while (p) {
            stat_result[res++] = p;
            p = strtok (NULL, ",");
        }

        fprintf(stdout, "%s: %s\n", stat_param[0], stat_result[0]);
        fprintf(stdout, "%s: %s\n", stat_param[1], stat_result[1]);
        fprintf(stdout, "%s: %s\n", stat_param[2], stat_result[2]);
        fprintf(stdout, "%s: %s\n", stat_param[3], stat_result[3]);
        fprintf(stdout, "%s: %s\n", stat_param[4], stat_result[4]);

        fprintf(stdout, " --- Calculation --- \n");
        unsigned long int hertz = sysconf(_SC_CLK_TCK);
        unsigned long int total_time = atol(stat_result[0]) + atol(stat_result[1]) + atol(stat_result[2]) + atol(stat_result[3]);
        unsigned long int sec = info.uptime - (atol(stat_result[4])/ hertz);
        unsigned long int cpu_usage = (100 * total_time) / (sec *hertz);

        fprintf(stdout, "Hertz: %lu \n", hertz);
        fprintf(stdout, "total time (%lu)\n", total_time);
        fprintf(stdout, "sec (%lu)\n", sec);
        fprintf(stdout, "cpu_usgae (%lu)\n", cpu_usage);
    }

    return 0;
}
