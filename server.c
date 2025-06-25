#include "header_server.h"
#include "functions_server.c"
#include <semaphore.h>

sem_t available_connections;
int p;
int server_fd;
char *prefix;

int main()
{
    // ARGOMENTI DA TERMINALE
    prefix = "file_test_";
    int l = 3;
    p = 3;
    // ARGOMENTI DA TERMINALE
    sem_init(&available_connections, 0, l); // Inizializza il semaforo per le connessioni attive

    struct sigaction sa;
    sa.sa_handler = termination_handler;
    sigemptyset(&sa.sa_mask);

    if (sigaction(SIGINT, &sa, NULL) == -1)
    {
        perror("sigaction");
        exit(1);
    }
    if (sigaction(SIGTERM, &sa, NULL) == -1)
    {
        perror("sigterm");
        exit(1);
    }

    init_socket(PORT, &server_fd);
    manage_connections();

    return 0;
}
