#include "header_server.h"
int delay = 0;

// Legge e valida gli argomenti da linea di comando
void read_args(int argc, char **argv, int *p, char **s, int *l)
{
    char *endptr;

    // controlli sul numero di argomenti del server
    if (argc != 4)
    {
        fprintf(stderr, "ERRORE ARGOMENTI\nGli argomenti devono essere:\n1) Grado parallelismo decifratura\n2) Prefisso file output\n3) Massimo connessioni concorrenti\n");
        exit(1);
    }

    // controlli sull'argomento p
    *p = (int)strtol(argv[1], &endptr, 10); // grado di parallelismo
    if (*endptr != '\0' || *p <= 0)
    {
        fprintf(stderr, "Errore: Il grado di parallelismo ('%s') deve essere un numero intero positivo.\n", argv[1]);
        exit(1);
    }

    // controlli sull'argomento s
    if (strlen(argv[2]) == 0)
    {
        fprintf(stderr, "Errore: Il prefisso del file non può essere vuoto.\n");
        exit(1);
    }
    if (strchr(argv[2], '/') != NULL)
    {
        fprintf(stderr, "Errore: Il prefisso del file ('%s') non può contenere il carattere '/'.\n", argv[2]);
        exit(1);
    }
    *s = argv[2]; // prefisso file di scrittura

    // controllo sull'argomento l
    *l = (int)strtol(argv[3], &endptr, 10); // numero massimo di connessioni concorrenti
    if (*endptr != '\0' || *l <= 0)
    {
        fprintf(stderr, "Errore: Il numero massimo di connessioni ('%s') deve essere un numero intero positivo.\n", argv[3]);
        exit(1);
    }
}

// Inizializza il socket del server
void init_socket(int port, int *server_fd)
{
    struct sockaddr_in addr;
    socklen_t addr_len = sizeof(addr);

    // 1. Crea socket
    *server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (*server_fd == -1)
    {
        perror("socket");
        exit(1);
    }

    // Permetti il riutilizzo immediato della porta
    int optval = 1;
    setsockopt(*server_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

    // 2. Imposta indirizzo locale

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY; // Ascolta su tutte le interfacce
    addr.sin_port = htons(port);

    // 3. Bind
    if (bind(*server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("bind");
        close(*server_fd);
        exit(1);
    }
    // 4. Listen
    if (listen(*server_fd, 1) < 0)
    {
        perror("listen");
        close(*server_fd);
        exit(1);
    }
    printf("[SERVER] Pronto ad accettare connessioni sulla porta %d...\n\n", port);
}

// Gestisce le nuove connessioni in ingresso
void manage_connections()
{
    struct sockaddr_in addr;
    socklen_t addr_len = sizeof(addr);
    int serial = 0;
    while (1)
    {
        int client_fd = accept(server_fd, (struct sockaddr *)&addr, &addr_len);
        if (client_fd < 0)
        {
            perror("accept");
            continue;
        }
        printf("[SERVER] Connessione accettata da %s:%d\n", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
        // Tenta di acquisire un semaforo senza bloccare
        if (sem_trywait(&available_connections) == -1)
        {
            // Se EAGAIN, il server è occupato
            if (errno == EAGAIN)
            {
                printf("Server occupato. Rifiuto connessione da %s:%d\n",
                       inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
                send(client_fd, "BUSY", 5, 0); // Invia un messaggio di "occupato"
                close(client_fd);
                continue; // Torna ad attendere la prossima connessione
            }
            else
            {
                // Altro errore con sem_trywait
                perror("sem_trywait");
                close(client_fd);
                continue;
            }
        }
        serial++;
        c_thread_args *args = malloc(sizeof(c_thread_args));
        args->client_fd = client_fd;
        args->serial = serial;
        pthread_t tid;
        pthread_create(&tid, NULL, *manage_client_message, args);
        pthread_detach(tid);
    }
}

// Gestisce la comunicazione con un singolo client (thread)
void *manage_client_message(void *arg)
{
    sleep(delay);
    c_thread_args *args = (c_thread_args *)arg;
    int client_fd = args->client_fd;
    int serial = args->serial;
    printf("[SERVER] Gestione client #%d (fd=%d) iniziata.\n", serial, client_fd);
    char *msg = receive_msg(client_fd);
    printf("[SERVER] Messaggio ricevuto dal client #%d.\n", serial);
    unsigned long long key = 0;
    size_t text_len = 0;
    char *text = NULL;
    get_key_and_text(msg, &key, &text_len, &text);
    sigset_t set = get_set();
    block_signals(set);
    char *decyphered_text = manage_threads(text, text_len, key);
    unblock_signals(set);
    printf("[SERVER] Decifratura completata.\n");
    char *eot = strchr(decyphered_text, EOT);
    *eot = '\0';

    const char *response = "ACK";
    send(client_fd, response, 4, 0);
    close(client_fd);

    int len_serial = snprintf(NULL, 0, "%d", serial);
    int filename_len = strlen(s) + len_serial + 5;
    char *filename = malloc(filename_len);
    snprintf(filename, filename_len, "%s%d.txt", s, serial);
    filename[filename_len] = '\0';
    printf("[SERVER] Scrivendo sul file: %s\n", filename);
    block_signals(set);
    write_file(decyphered_text, filename);
    unblock_signals(set);
    printf("[SERVER] Fine scrittura sul file: %s\n\n", filename);
    // Cleanup
    cleanup_client_resources(client_fd, msg, text, decyphered_text, filename);
    free(args);
    sem_post(&available_connections);
}

// Riceve un messaggio completo dal client
char *receive_msg(int client_fd)
{
    char buffer[BUF_SIZE];
    memset(buffer, 0, sizeof(buffer));

    char *msg = NULL;
    int total_received = 0;
    do
    {
        int n = recv(client_fd, buffer, BUF_SIZE, 0);
        if (n <= 0)
            break;

        // Alloca o rialloca msg
        char *new_msg = realloc(msg, total_received + n + 1); // +1 per terminatore
        if (!new_msg)
        {
            free(msg);
            perror("realloc");
            break;
        }
        msg = new_msg;

        // Copia i nuovi dati
        memcpy(msg + total_received, buffer, n);
        total_received += n;

    } while (msg[total_received - 1] != SEPARATOR);
    return msg;
}

// Estrae chiave e testo dal messaggio ricevuto
void get_key_and_text(char *msg, unsigned long long *key, size_t *text_len, char **text)
{
    // Formato messaggio: key SEPARATOR text_len SEPARATOR text SEPARATOR

    // Trova i separatori
    char *sep1 = strchr(msg, SEPARATOR);
    if (!sep1)
    {
        *key = 0;
        *text = NULL;
        *text_len = 0;
        return;
    }
    char *sep2 = strchr(sep1 + 1, SEPARATOR);
    if (!sep2)
    {
        *key = 0;
        *text = NULL;
        *text_len = 0;
        return;
    }
    // Estrai la chiave
    *sep1 = '\0';
    *key = strtoull(msg, NULL, 10);

    // Estrai la lunghezza del testo
    *sep2 = '\0';
    *text_len = strtoull(sep1 + 1, NULL, 10);

    // Alloca e copia il testo
    *text = malloc(*text_len + 1);
    if (!*text)
    {
        *text_len = 0;
        return;
    }
    memcpy(*text, sep2 + 1, *text_len);
    (*text)[*text_len] = '\0'; // Per sicurezza, anche se il testo può contenere '\0'
}

// Gestisce la decifratura parallela del testo
char *manage_threads(char *text, size_t text_len, unsigned long long key)
{
    char *text_buffer = malloc(text_len + 1);
    // calcola il numero totale di blocchi
    int blocks_total = text_len / 8; // 8 byte per blocco
    int blocks_per_thread, blocks_last_thread, thread_count;
    if (blocks_total <= p)
    {
        blocks_per_thread = 1;
        blocks_last_thread = 0;
        thread_count = blocks_total;
    }
    else
    {
        // Calcola il numero di blocchi per thread
        blocks_per_thread = text_len / (p * 8);      // 8 byte per blocco
        blocks_last_thread = text_len % (p * 8) / 8; // Resto per l'ultimo thread
        thread_count = p;
    }

    pthread_t *tids = malloc(sizeof(pthread_t) * thread_count);

    for (int i = 0; i < thread_count; i++)
    {
        int blocks = blocks_per_thread;
        if (i == p - 1 && blocks_last_thread > 0)
            blocks += blocks_last_thread;

        int start = i * blocks_per_thread * 8;
        char *partial = malloc(blocks * 8 + 1);
        // strncpy(partial, text + start, blocks * 8);
        memcpy(partial, text + start, blocks * 8);
        partial[blocks * 8] = '\0';

        d_thread_args *args = malloc(sizeof(d_thread_args));
        args->partial = partial;
        args->offset = start;
        args->key = key;
        args->text_buffer = text_buffer;
        args->blocks_num = blocks;
        pthread_create(&tids[i], NULL, *decypher_partial, args);
    }
    // Attende la terminazione di tutti i thread
    for (int i = 0; i < thread_count; i++)
    {
        pthread_join(tids[i], NULL);
    }
    free(tids);
    text_buffer[text_len] = '\0'; // Assicurati che il buffer sia terminato correttamente
    return text_buffer;           // Restituisce il buffer con il testo decifrato
}

// Funzione eseguita da ciascun thread per decifrare una porzione di testo
void *decypher_partial(void *arg)
{
    d_thread_args *args = (d_thread_args *)arg;
    char *partial = args->partial;
    int offset = args->offset;
    char *text_buffer = args->text_buffer;
    unsigned long long key = args->key;
    int blocks_num = args->blocks_num;
    char block[9];
    for (int i = 0; i < blocks_num; i++)
    {
        // strncpy(block, partial + (i * 8), 8);
        memcpy(block, partial + (i * 8), 8);
        block[8] = '\0';
        decypher_block(block, offset + (i * 8), key, text_buffer);
    }
    free(partial);
    free(args);
}

// Decifra un blocco di testo e lo scrive nel buffer
void decypher_block(char *block, int offset, unsigned long long key, char *text_buffer)
{
    unsigned long long block_bytes = string_to_bits(block);
    unsigned long long decyphered_bytes = block_bytes ^ key;
    char *decyphered_block = bits_to_string(decyphered_bytes);
    memcpy(text_buffer + offset, decyphered_block, 8);
    free(decyphered_block);
}

// Converte una stringa di 8 caratteri in un intero a 64 bit
unsigned long long string_to_bits(const char *str)
{
    unsigned long long result = 0;

    for (size_t i = 0; i < 8; i++)
    {
        result <<= 8;                    // lascia spazio per il prossimo carattere
        result |= (unsigned char)str[i]; // aggiungi i byte in fondo
    }

    // Stampa la rappresentazione binaria

    return result;
}

// Converte un intero a 64 bit in una stringa di 8 caratteri
char *bits_to_string(unsigned long long bits)
{
    char *str = malloc(9); // 8 byte + terminatore di stringa
    for (int i = 7; i >= 0; i--)
    {
        str[i] = (char)(bits & 0xFF);
        bits >>= 8;
    }
    str[8] = '\0'; // Terminatore di stringa
    return str;
}

// Scrive il testo decifrato su file
void write_file(char *text, char *pathfile)
{
    FILE *file = fopen(pathfile, "w");
    if (!file)
    {
        perror("Errore nell'apertura del file");
        return;
    }
    fwrite(text, sizeof(char), strlen(text), file);
    fclose(file);
}

// Pulisce tutte le risorse del client
void cleanup_client_resources(int client_fd, char *msg, char *text, char *decyphered_text, char *filename)
{
    if (client_fd >= 0)
    {
        close(client_fd);
    }
    free(msg);
    free(text);
    free(decyphered_text);
    free(filename);
}

// Imposta i gestori dei segnali per la terminazione sicura
void setup_signal_handlers()
{
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
    if (sigaction(SIGTSTP, &sa, NULL) == -1)
    {
        perror("sigtstp");
        exit(1);
    }
}

// Restituisce un set di segnali da gestire
sigset_t get_set()
{
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGINT);
    sigaddset(&set, SIGALRM);
    sigaddset(&set, SIGUSR1);
    sigaddset(&set, SIGUSR2);
    sigaddset(&set, SIGTERM);
    return set;
}

// Gestisce la terminazione del server su segnale
void termination_handler(int signum)
{
    printf("\n[SERVER] Terminazione richiesta. Chiudo il server...\n");
    sem_destroy(&available_connections);
    close(server_fd);
    exit(0);
}

// Blocca i segnali in set
void block_signals(sigset_t set)
{

    if (pthread_sigmask(SIG_BLOCK, &set, NULL) != 0)
    {
        perror("Errore nel blocco dei segnali");
        exit(EXIT_FAILURE);
    }
}

// Sblocca i segnali in set
void unblock_signals(sigset_t set)
{

    if (pthread_sigmask(SIG_UNBLOCK, &set, NULL) != 0)
    {
        perror("Errore nello sblocco dei segnali");
        exit(EXIT_FAILURE);
    }
}