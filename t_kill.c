#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <signal.h>
#include <string.h>
#include <errno.h>

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        fprintf(stderr, "Usage: <signo> <tid>\n");
        exit(EXIT_FAILURE);
    }

    // int sig = atoi(argv[1]);
    // pthread_t tid = atol(argv[2]);

    int sig = 15;
    pthread_t tid = 0;

    int s = pthread_kill(tid, sig);
    if (s != 0)
    {
        fprintf(stderr, "pthread_kill: %s\n", strerror(s));
        exit(EXIT_FAILURE);
    }

    return 0;
}