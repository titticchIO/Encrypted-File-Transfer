#include "header_server.h"

void termination_handler(int signum)
{
    printf("\n[SERVER] Terminazione richiesta. Chiudo il server...\n");
    sem_destroy(&available_connections);
    close(server_fd);
    exit(0);
}

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

    printf("[SERVER] In ascolto su porta %d...\n", PORT);
}

void manage_connections()
{
    struct sockaddr_in addr;
    socklen_t addr_len = sizeof(addr);

    while (1)
    {
        sem_wait(&available_connections); // Attende che ci sia una connessione disponibile

        int client_fd = accept(server_fd, (struct sockaddr *)&addr, &addr_len);
        if (client_fd < 0)
        {
            perror("accept");
            continue;
        }

        // Qui puoi creare un thread per gestire la connessione
        pthread_t tid;
        pthread_create(&tid, NULL, *manage_client_message, &client_fd);
        pthread_detach(tid);
    }
}

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

    // Debug:
    // stampa il testo ricevuto come dati binari
    fwrite(*text, 1, *text_len, stdout);
    printf("\n");
}

void *manage_client_message(void *arg)
{
    int client_fd = *(int *)arg;
    char *msg = receive_msg(client_fd);
    unsigned long long key = 0;
    size_t text_len = 0;
    char *text = NULL;
    get_key_and_text(msg, &key, &text_len, &text);
    char *decyphered_text = manage_threads(text, text_len, key);
    printf("[SERVER] Messaggio decifrato: %s\n", decyphered_text);

    const char *response = "ACK";
    send(client_fd, response, strlen(response), 0);
    close(client_fd);
    // scrivi su output.txt
}

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

        thread_args *args = malloc(sizeof(thread_args));
        args->partial = partial;
        args->offset = start;
        args->key = key;
        args->text_buffer = text_buffer;
        args->blocks_num = blocks_per_thread;
        pthread_create(&tids[i], NULL, *decypher_partial, args);
    }
    // Attende la terminazione di tutti i thread
    for (int i = 0; i < thread_count; i++)
    {
        pthread_join(tids[i], NULL);
    }
    free(tids);
    free(text);                   // Libera il testo originale
    text_buffer[text_len] = '\0'; // Assicurati che il buffer sia terminato correttamente
    return text_buffer;           // Restituisce il buffer con il testo decifrato
}

void *decypher_partial(void *arg)
{
    thread_args *args = (thread_args *)arg;
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

void decypher_block(char *block, int offset, unsigned long long key, char *text_buffer)
{
    // char *key_s = bits_to_string(key);  //DEBUG
    // printf("Key: %s\n", key_s);          //DEBUG
    unsigned long long block_bytes = string_to_bits(block);
    printf("Blocco in binario: ");
    for (int i = 63; i >= 0; i--)
    {
        printf("%llu", (block_bytes >> i) & 1ULL);
    }
    printf("\n");
    unsigned long long decyphered_bytes = block_bytes ^ key;
    char *decyphered_block = bits_to_string(decyphered_bytes);
    memcpy(text_buffer + offset, decyphered_block, 8);
    free(decyphered_block);
}

unsigned long long string_to_bits(const char *str)
{
    unsigned long long result = 0;
    size_t len = strlen(str);

    if (len > 8)
        len = 8; // massimo 8 caratteri (64 bit)

    for (size_t i = 0; i < len; i++)
    {
        result <<= 8;                    // lascia spazio per il prossimo carattere
        result |= (unsigned char)str[i]; // aggiungi i byte in fondo
    }

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
