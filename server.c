#include "header_server.h"
// #include "functions_server.c"

sem_t available_connections;
int p;
int server_fd;
char *s;
pthread_t main_thread_id;

int main(int argc, char *argv[])
{
    // argomenti da terminale
    int l;
    read_args(argc, argv, &p, &s, &l);
    // Assegna il pid del thread principale
    main_thread_id = pthread_self();

    // Inizializza il semaforo per le connessioni attive
    if (sem_init(&available_connections, 0, l) == -1)
    {
        perror("sem_init");
        exit(EXIT_FAILURE);
    }

    setup_signal_handlers();

    // Inizializza il socket del server
    init_socket(PORT, &server_fd);

    // Inizia la gestione delle connessioni
    manage_connections();
    return 0;
}
