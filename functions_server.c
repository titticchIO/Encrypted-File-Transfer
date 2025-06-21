#include "header_server.h"

void init_socket(int port, int *server_fd, int l)
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
    // while (1) {
    // 	pthread_t tid;
    // 	int *client_socket = malloc(sizeof(int));
    // 	*client_socket = accept(server_socket, ...);
    // 	pthread_create(&tid, NULL, client_handler, client_socket);
    // 	pthread_detach(tid);
    // }

    // // 5. Accept
    // *client_fd = accept(*server_fd, (struct sockaddr *)&addr, &addr_len);
    // if (*client_fd < 0)
    // {
    //     perror("accept");
    //     close(*server_fd);
    //     exit(1);
    // }
}

void manage_threads(int *server_fd)
{
    struct sockaddr_in addr;
    socklen_t addr_len = sizeof(addr);

    while (1)
    {
        sem_wait(&available_connections); // Attende che ci sia una connessione disponibile

        int client_fd = accept(*server_fd, (struct sockaddr *)&addr, &addr_len);
        if (client_fd < 0)
        {
            perror("accept");
            continue;
        }

        // Qui puoi creare un thread per gestire la connessione
        pthread_t tid;
        pthread_create(&tid, NULL, manage_client_message, &client_fd);
        pthread_detach(tid);
    }
}

void manage_client_message()
{
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

void get_key_and_text(char *msg, size_t *text_len, char **text, unsigned long long *key)
{
    char *sep1 = strchr(msg, SEPARATOR);
    if (!sep1)
    {
        *key = 0;
        *text = NULL;
        return;
    }
    // key|len|text
    // Estrai la chiave
    char *sep2 = strchr(sep1 + 1, SEPARATOR);
    if (!sep2)
    {
        *text = NULL;
        return;
    }
    *sep1 = '\0';
    *sep2 = '\0';
    *key = strtoull(msg, NULL, 10);
    *text_len = strtoull(sep1 + 1, NULL, 10);

    // Trova il secondo separatore

    *text = malloc(*text_len + 1);
    if (!*text)
        return;
    memcpy(*text, sep2 + 1, *text_len);
    (*text)[*text_len] = '\0';
}

char *decypher_msg(char *msg, unsigned long long key)
{
    return NULL;
}

char *decypher_block(char *block, unsigned long long key)
{
    // unsigned long long block_bytes = string_to_bits(block);
    // unsigned long long decypher_bytes = block_bytes ^ key;
    // return bits_to_string(decypher_bytes);
    return NULL;
}