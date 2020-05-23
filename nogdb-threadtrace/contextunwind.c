#ifdef DUMPSTACK

#include "contextunwind.h"

uint8_t g_threads = 0;
pthread_t tid[64] = {0};
char exeName[100] = {0};

int unwind_thread_callstack (void);
void get_therads(void);
static int getFileAndLine (unw_word_t addr, char *file, size_t flen, int *line);

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

        printf("\n\n----------------- PS INFO BEGIN -----------------\n");
        sprintf (cmd, "ps -mo THREAD -p %d", temp);
        system (cmd);
        printf("----------------- PS INFO DONE -----------------\n");
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
        fflush(stdout);

        do {sleep(2);} while(0);
    }
        
    printf("----------------- LOCAL unwind_thread_callstack -----------------\n");
    unwind_thread_callstack();
    fflush(stdout);
    do {sleep(10);} while(0);
    printf("^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n");


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
                    else
                        do {sleep(10);} while(0);
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

static int getFileAndLine (unw_word_t addr, char *file, size_t flen, int *line)
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

        unw_get_reg (&cursor, UNW_X86_64_RIP, &ip);
        unw_get_reg (&cursor, UNW_X86_64_RSP, &sp);
        unw_get_reg (&cursor, UNW_X86_64_RBP, &bp);

        unw_get_reg (&cursor, UNW_X86_64_RAX,   &val[0]); 
        unw_get_reg (&cursor, UNW_X86_64_RDX,   &val[1]);
        unw_get_reg (&cursor, UNW_X86_64_RCX,   &val[2]);
        unw_get_reg (&cursor, UNW_X86_64_RBX,   &val[3]);
        unw_get_reg (&cursor, UNW_X86_64_RSI,   &val[4]);
        unw_get_reg (&cursor, UNW_X86_64_RDI,   &val[5]);
        unw_get_reg (&cursor, UNW_X86_64_RBP,   &val[6]);
        unw_get_reg (&cursor, UNW_X86_64_RSP,   &val[7]);
        unw_get_reg (&cursor, UNW_X86_64_R8,    &val[8]);
        unw_get_reg (&cursor, UNW_X86_64_R9,    &val[9]);
        unw_get_reg (&cursor, UNW_X86_64_R10,   &val[10]);
        unw_get_reg (&cursor, UNW_X86_64_R11,   &val[11]);
        unw_get_reg (&cursor, UNW_X86_64_R12,   &val[12]);
        unw_get_reg (&cursor, UNW_X86_64_R13,   &val[13]);
        unw_get_reg (&cursor, UNW_X86_64_R14,   &val[14]);
        unw_get_reg (&cursor, UNW_X86_64_R15,   &val[15]);
        unw_get_reg (&cursor, UNW_X86_64_RIP,   &val[16]);

        if (depth == 0)
            sp_end = sp;

        unw_get_proc_info (&cursor, &pi);
        
        depth += 1;

#if 0
        getFileAndLine((long)ip, file, 256, &line);
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
    fflush(stdout);
    

#ifdef DUMPSTACK_EXTRASTACK
    printf("\n --- STACK from 0x%"PRIx64" to 0x%"PRIx64"--- \n", sp_begin, sp_end);
    if ((sp_begin - sp_end) > 0) {
        uint8_t count = 4;
        while (sp_begin != sp_end) {
            if (count == 4) {
                count = 0;
                printf("\n %"PRIx64": ", sp_begin);
            }

            count += 1;
            void *p = sp_begin;
            printf("0x%016"PRIx64" | ", *(uint64_t *)p);
            sp_begin -= 8;
        }
    }
    printf("\n --- --- --- --- --- --- --- --- \n");
    fflush(stdout);
#endif

    return 0;
}


#endif

