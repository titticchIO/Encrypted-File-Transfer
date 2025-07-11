#include "header_client.h"

unsigned long long key = 0;
char *text_buffer = NULL;

// Legge e valida gli argomenti da linea di comando
void read_args(int argc, char **argv, char **filename, char **key_s, int *p, char **ip, int *port)
{
    char *endptr;

    // controlli sul numero di argomenti del client
    if (argc != 6)
    {
        fprintf(stderr, "ERROR ARGUMENTS\nThe arguments must be:\n1) Input file name\n2) Key (8 characters)\n3) Parallelism degree for encryption\n4) Server IP\n5) Server port\n");
        exit(1);
    }

    // controlli sull'argomento filename
    *filename = argv[1]; // file input
    if (strlen(*filename) == 0)
    {
        fprintf(stderr, "Error: The file name cannot be empty.\n");
        exit(1);
    }

    // controlli sull'argomento key_s
    if (strlen(argv[2]) != 8)
    {
        fprintf(stderr, "Errore: The key ('%s') must be exactly 8 characters long.\n", argv[2]);
        exit(1);
    }
    *key_s = argv[2]; // chiave di cifratura

    // controlli sull'argomento p
    *p = (int)strtol(argv[3], &endptr, 10); // grado di parallelismo nella cifratura
    if (*endptr != '\0' || *p <= 0)
    {
        fprintf(stderr, "Error: The parallelism degree ('%s') must be a positive integer.\n", argv[3]);
        exit(1);
    }

    // controlli sull'argomento ip
    *ip = argv[4]; // ip del server
    if (strlen(*ip) == 0)
    {
        fprintf(stderr, "Error: The server IP address cannot be empty.\n");
        exit(1);
    }

    // controlli sull'argomento port
    *port = (int)strtol(argv[5], &endptr, 10); // porta del server
    if (*endptr != '\0' || *port <= 0 || *port > 65535)
    {
        fprintf(stderr, "Error: The server port ('%s') must be an integer between 1 and 65535.\n", argv[5]);
        exit(1);
    }
}

// Legge il contenuto di un file e lo restituisce come stringa
char *read_file(char *filename)
{
    long dimension;
    int read;
    char *buffer;
    FILE *file = fopen(filename, "r");
    if (!file)
    {
        fprintf(stderr, "Error opening file '%s'.\n", filename);
        return NULL;
    }

    if (fseek(file, 0, SEEK_END) != 0)
    {
        perror("fseek");
        fclose(file);
        return NULL;
    }
    dimension = ftell(file);
    if (dimension == -1L)
    {
        perror("ftell");
        fclose(file);
        return NULL;
    }
    rewind(file);

    buffer = malloc(dimension + 1);
    if (!buffer)
    {
        perror("Error allocating memory");
        fclose(file);
        return NULL;
    }

    read = fread(buffer, sizeof(char), dimension, file);
    buffer[read] = '\0';

    if (fclose(file) == EOF)
    {
        perror("fclose");
        free(buffer);
        return NULL;
    }

    return buffer;
}
// Gestisce la connessione al server
int connect_to_server(int port, const char *ip, struct sockaddr_in *server_addr)
{
    int sockfd = init_socket(port, ip, server_addr);

    printf("[CLIENT] Connecting to server %s:%d...\n", ip, port);
    if (connect(sockfd, (struct sockaddr *)server_addr, sizeof(*server_addr)) < 0)
    {
        perror("connect");
        close(sockfd);
        return -1;
    }

    return sockfd;
}

// Inizializza e restituisce un socket TCP
int init_socket(int port, const char *server_ip, struct sockaddr_in *server_addr)
{
    int sockfd;

    // 1. Crea socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        perror("socket");
        exit(1);
    }

    // 2. Imposta indirizzo del server
    memset(server_addr, 0, sizeof(*server_addr));
    server_addr->sin_family = AF_INET;
    server_addr->sin_port = htons(port);
    if (inet_pton(AF_INET, server_ip, &server_addr->sin_addr) <= 0)
    {
        perror("inet_pton");
        close(sockfd);
        exit(1);
    }
    return sockfd;
}

// Gestisce il processo di cifratura
size_t encrypt_msg(char **text, int p, size_t orig_l)
{
    size_t l = divide_blocks(text, p, orig_l);
    printf("[CLIENT] Encryption completed.\n");
    return l;
}

// Divide il testo in blocchi da cifrare e gestisce il padding
size_t divide_blocks(char **text, int p, size_t L)
{
    int padding_len, blocks_num, blocks_per_thread, blocks_last_thread;
    padding_len = 8 - (L % 8);
    if (padding_len < 8)
    {
        char *new_text = realloc(*text, L + padding_len + 1);
        if (!new_text)
        {
            perror("realloc");
            free(*text);
            exit(1);
        }
        *text = new_text;
        memset(*text + L, EOT, padding_len);
        (*text)[L + padding_len] = '\0';
        L += padding_len;
    }
    blocks_num = L / 8;
    blocks_per_thread = blocks_num / p;  // Calcola quanti blocchi deve cifrare ciascun thread
    blocks_last_thread = blocks_num % p; // Calcola quanti blocchi restano all’ultimo thread

    // Alloca il buffer globale per il testo cifrato
    if (text_buffer)
        free(text_buffer);
    text_buffer = malloc(L + 1);
    if (!text_buffer)
    {
        perror("malloc for text_buffer");
        exit(1);
    }
    memset(text_buffer, 0, L + 1);
    manage_threads(*text, blocks_per_thread, blocks_last_thread, p);
    return L;
}

// Gestisce la creazione e sincronizzazione dei thread di cifratura
void manage_threads(char *text, int blocks_per_thread, int blocks_last_thread, int p)
{
    int blocks, start;
    char *partial;
    thread_args *args;
    pthread_t *tids = malloc(sizeof(pthread_t) * p);
    if (!tids)
    {
        perror("malloc for tids");
        exit(1);
    }

    for (int i = 0; i < p; i++)
    {
        blocks = blocks_per_thread;
        if (i == p - 1 && blocks_last_thread > 0) // Se e' l'ultimo thread deve cifrare anche i blocchi in eccesso se presenti
            blocks += blocks_last_thread;

        start = i * blocks_per_thread * 8;
        partial = malloc(blocks * 8 + 1);

        strncpy(partial, text + start, blocks * 8);
        partial[blocks * 8] = '\0';

        args = malloc(sizeof(thread_args));
        args->partial = partial;
        args->offset = start;

        pthread_create(&tids[i], NULL, cypher_partial, args);
    }
    // Attende la terminazione di tutti i thread
    for (int i = 0; i < p; i++)
    {
        pthread_join(tids[i], NULL);
    }
    free(tids);
}

// Funzione eseguita da ciascun thread per cifrare una porzione di testo
void *cypher_partial(void *void_args)
{
    thread_args *args = (thread_args *)void_args;
    char *partial = args->partial;
    int offset = args->offset;
    int blocks_num = strlen(partial) / 8;
    char block[9];

    for (int i = 0; i < blocks_num; i++)
    {
        strncpy(block, partial + (i * 8), 8);
        block[8] = '\0';
        cypher_block(block, offset + (i * 8));
    }
    free(partial);
    free(args);
    return NULL;
}

// Cifra un blocco di testo e lo scrive nel buffer globale
void cypher_block(const char *block, int offset)
{

    unsigned long long block_bytes = string_to_bits(block);
    unsigned long long cyphered_bytes = block_bytes ^ key;
    char *cyphered_block = bits_to_string(cyphered_bytes);
    memcpy(text_buffer + offset, cyphered_block, 8);
    free(cyphered_block);
}

// Converte una stringa di 8 caratteri in un intero a 64 bit
unsigned long long string_to_bits(const char *str)
{
    unsigned long long result = 0;
    for (size_t i = 0; i < 8; i++)
    {
        result <<= 8;
        result |= (unsigned char)str[i];
    }
    return result;
}

// Converte un intero a 64 bit in una stringa di 8 caratteri
char *bits_to_string(unsigned long long bits)
{
    char *str = malloc(9);
    for (int i = 7; i >= 0; i--)
    {
        str[i] = (char)(bits & 0xFF);
        bits >>= 8;
    }
    str[8] = '\0';
    return str;
}

// Gestisce l'invio del messaggio al server
void send_message_to_server(int sockfd, unsigned long long key, char *text_buffer, size_t l)
{
    size_t msg_len;
    int n;
    char *msg = make_msg(key, text_buffer, l, &msg_len);

    printf("[CLIENT] Sending message...\n");
    n = send(sockfd, msg, msg_len, 0);
    if (n < 0)
    {
        perror("send");
        free(msg);
        exit(1);
    }

    free(msg);
}

// Crea il messaggio da inviare al server secondo il protocollo
char *make_msg(unsigned long long key, char *text, size_t l, size_t *msg_len)
{
    char *msg;
    int offset = 0;
    // Calcola la lunghezza delle stringhe rappresentanti key e l
    int key_len = snprintf(NULL, 0, "%llu", key);
    int l_len = snprintf(NULL, 0, "%zu", l);

    // Dimensione totale: key + sep + l + sep + testo + sep + terminatore
    *msg_len = key_len + 1 + l_len + 1 + l + 1;
    msg = malloc(*msg_len);
    if (!msg)
    {
        perror("malloc for make_msg");
        return NULL;
    }

    // Scrive key
    offset += snprintf(msg + offset, *msg_len - offset, "%llu", key);
    msg[offset++] = SEPARATOR;
    // Scrive la lunghezza del testo
    offset += snprintf(msg + offset, *msg_len - offset, "%zu", l);
    msg[offset++] = SEPARATOR;
    // Scrive il testo (può contenere byte nulli, quindi usa memcpy)
    memcpy(msg + offset, text, l);
    offset += l;
    msg[offset] = SEPARATOR;

    return msg;
}

// Gestisce la ricezione dell'ACK dal server
void receive_ack(int sockfd)
{
    char buffer[5];
    int n;

    memset(buffer, 0, 5);
    n = recv(sockfd, buffer, 5, 0);
    if (n < 0)
    {
        perror("recv");
        exit(1);
    }

    if (strcmp(buffer, "ACK") == 0)
    {
        printf("[CLIENT] ACK received\n");
    }
    else if (strcmp(buffer, "BUSY") == 0)
    {
        printf("[CLIENT] Connection refused. Server busy.\n");
    }
    else
    {
        printf("[CLIENT] Connection closed, no response from server.\n");
    }
}

// Restituisce un set di segnali da gestire
sigset_t get_set()
{
    sigset_t set;
    if (sigemptyset(&set) != 0)
    {
        perror("sigemptyset");
        exit(EXIT_FAILURE);
    }
    if (sigaddset(&set, SIGINT) != 0)
    {
        perror("sigaddset SIGINT");
        exit(EXIT_FAILURE);
    }
    if (sigaddset(&set, SIGALRM) != 0)
    {
        perror("sigaddset SIGALRM");
        exit(EXIT_FAILURE);
    }
    if (sigaddset(&set, SIGUSR1) != 0)
    {
        perror("sigaddset SIGUSR1");
        exit(EXIT_FAILURE);
    }
    if (sigaddset(&set, SIGUSR2) != 0)
    {
        perror("sigaddset SIGUSR2");
        exit(EXIT_FAILURE);
    }
    if (sigaddset(&set, SIGTERM) != 0)
    {
        perror("sigaddset SIGTERM");
        exit(EXIT_FAILURE);
    }
    return set;
}

// Blocca i segnali specificati
void block_signals(sigset_t set)
{

    if (pthread_sigmask(SIG_BLOCK, &set, NULL) != 0)
    {
        perror("Signal block error");
        exit(EXIT_FAILURE);
    }
}

// Sblocca i segnali specificati
void unblock_signals(sigset_t set)
{

    if (pthread_sigmask(SIG_UNBLOCK, &set, NULL) != 0)
    {
        perror("Signal unblock error");
        exit(EXIT_FAILURE);
    }
}
