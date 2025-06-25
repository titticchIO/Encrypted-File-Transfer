#include "header_client.h"

unsigned long long key = 0;
char *text_buffer = NULL;

char *read_file(char *filename)
{
    FILE *file = fopen(filename, "r");
    if (!file)
    {
        perror("Errore nell'apertura del file");
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long dimension = ftell(file);
    rewind(file);

    char *buffer = malloc(dimension + 1);
    if (!buffer)
    {
        perror("Errore nell'allocazione di memoria");
        fclose(file);
        return NULL;
    }

    int read = fread(buffer, sizeof(char), dimension, file);
    buffer[read] = '\0';
    fclose(file);
    return buffer;
}

// Blocca i segnali SIGINT, SIGALRM, SIGUSR1, SIGUSR2, SIGTERM
void block_signals(sigset_t set)
{

    if (pthread_sigmask(SIG_BLOCK, &set, NULL) != 0)
    {
        perror("Errore nel blocco dei segnali");
        exit(EXIT_FAILURE);
    }
}

// Sblocca i segnali SIGINT, SIGALRM, SIGUSR1, SIGUSR2, SIGTERM
void unblock_signals(sigset_t set)
{

    if (pthread_sigmask(SIG_UNBLOCK, &set, NULL) != 0)
    {
        perror("Errore nello sblocco dei segnali");
        exit(EXIT_FAILURE);
    }
}
size_t divide_blocks(char *text, int p, size_t L)
{
    int padding_len = 8 - (L % 8);
    if (padding_len < 8)
    {
        text = realloc(text, L + padding_len + 1);
        memset(text + L, EOT, padding_len);
        text[L + padding_len] = '\0';
        L += padding_len;
    }
    int blocks_num = L / 8;
    int blocks_per_thread = blocks_num / p;
    int blocks_last_thread = blocks_num % p;

    // Alloca il buffer globale per il testo cifrato
    if (text_buffer)
        free(text_buffer);
    text_buffer = malloc(L + 1);
    memset(text_buffer, 0, L + 1);
    manage_threads(text, blocks_per_thread, blocks_last_thread, p);
    return L;
}

void manage_threads(char *text, int blocks_per_thread, int blocks_last_thread, int p)
{
    int total_blocks = (blocks_per_thread * p) + blocks_last_thread;
    int total_bytes = total_blocks * 8;
    pthread_t *tids = malloc(sizeof(pthread_t) * p);

    for (int i = 0; i < p; i++)
    {
        int blocks = blocks_per_thread;
        if (i == p - 1 && blocks_last_thread > 0)
            blocks += blocks_last_thread;

        int start = i * blocks_per_thread * 8;
        char *partial = malloc(blocks * 8 + 1);
        strncpy(partial, text + start, blocks * 8);
        partial[blocks * 8] = '\0';

        thread_args *args = malloc(sizeof(thread_args));
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
}

void cypher_block(const char *block, int offset)
{

    unsigned long long block_bytes = string_to_bits(block);
    unsigned long long cyphered_bytes = block_bytes ^ key; // XOR
    char *cyphered_block = bits_to_string(cyphered_bytes);
    memcpy(text_buffer + offset, cyphered_block, 8);
    free(cyphered_block);
}

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
    inet_pton(AF_INET, server_ip, &server_addr->sin_addr);
    return sockfd;
}

char *make_msg(unsigned long long key, char *text, size_t l, size_t *msg_len)
{
    // Calcola la lunghezza delle stringhe rappresentanti key e l
    int key_len = snprintf(NULL, 0, "%llu", key);
    int l_len = snprintf(NULL, 0, "%zu", l);

    // Dimensione totale: key + sep + l + sep + testo + sep + terminatore
    *msg_len = key_len + 1 + l_len + 1 + l + 1;
    char *msg = malloc(*msg_len);
    if (!msg)
        return NULL;

    int offset = 0;
    // Scrivi key
    offset += snprintf(msg + offset, *msg_len - offset, "%llu", key);
    msg[offset++] = SEPARATOR;
    // Scrivi la lunghezza del testo
    offset += snprintf(msg + offset, *msg_len - offset, "%zu", l);
    msg[offset++] = SEPARATOR;
    // Scrivi il testo (può contenere byte nulli, quindi usa memcpy)
    memcpy(msg + offset, text, l);
    offset += l;
    msg[offset] = SEPARATOR;

    return msg;
}