#include "header_server.h"
#include "functions_server.c"
#include <semaphore.h>

sem_t available_connections;
int p;
int server_fd;
char *s;

int main(int argc, char *argv[])
{
    // ARGOMENTI DA TERMINALE
    int l;
    read_args(argv, &p, &s, &l);

    // ARGOMENTI DA TERMINALE
    sem_init(&available_connections, 0, l); // Inizializza il semaforo per le connessioni attive

    setup_signal_handlers();

    init_socket(PORT, &server_fd);
    manage_connections();

    return 0;
}
