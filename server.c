#include "header_server.h"
#include "functions_server.c"

sem_t available_connections;
int p;
int server_fd;
char *s;

int main(int argc, char *argv[])
{
    // argomenti da terminale
    int l;
    read_args(argc, argv, &p, &s, &l);

    // Inizializza il semaforo per le connessioni attive
    sem_init(&available_connections, 0, l);

    setup_signal_handlers();

    // Inizializza il socket del server
    init_socket(PORT, &server_fd);

    // Inizia la gestione delle connessioni
    manage_connections();
    return 0;
}
