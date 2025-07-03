/* tsig.c : tgkill wrapper  gcc tsig.c -o tsig */
#define _GNU_SOURCE
#include <signal.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char **argv)
{
    if (argc != 4)
    {
        fprintf(stderr, "uso: %s <tgid> <tid> <sig>\n", argv[0]);
        return 1;
    }
    pid_t tgid = (pid_t)atoi(argv[1]);
    pid_t tid = (pid_t)atoi(argv[2]);
    int sig = atoi(argv[3]); /* oppure usa str2sig */
    if (syscall(SYS_tgkill, tgid, tid, sig) == -1)
    {
        perror("tgkill");
        return 1;
    }
    return 0;
}
