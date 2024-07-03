#ifndef PROFILE_HELPER_H
#define PROFILE_HELPER_H

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

int StartProfile (void)
{
        char pd[50] = {'\0'};
        char name[50] = {'\0'};
        snprintf (pd, 49, "%d", getpid());
        snprintf (name, 49, "%d-profile.data", getpid());

        pid_t pid = fork();
        if (pid == 0) {
            auto fd=open("/dev/null",O_RDWR);
            dup2(fd,1);
            dup2(fd,2);
            exit(execl("/usr/bin/perf","perf","record","-g","-o",name,"-p",pd,nullptr));
        }

        return pid;
}

void StopProfile (pid_t pid)
{
        // Kill profiler
        kill(pid,SIGINT);
        waitpid(pid,nullptr,0);
}

#endif
