#include "header_server.h"

int main()
{
    int server_fd, client_fd;
    struct sockaddr_in addr;
    socklen_t addr_len = sizeof(addr);
    char buffer[BUF_SIZE];

    // 1. Crea socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1)
    {
        perror("socket");
        exit(1);
    }

    // 2. Imposta indirizzo locale
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY; // Ascolta su tutte le interfacce
    addr.sin_port = htons(PORT);

    // 3. Bind
    if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("bind");
        close(server_fd);
        exit(1);
    }

    // 4. Listen
    if (listen(server_fd, 1) < 0)
    {
        perror("listen");
        close(server_fd);
        exit(1);
    }

    printf("[SERVER] In ascolto su porta %d...\n", PORT);

    // 5. Accept
    client_fd = accept(server_fd, (struct sockaddr *)&addr, &addr_len);
    if (client_fd < 0)
    {
        perror("accept");
        close(server_fd);
        exit(1);
    }

    char *text_size = malloc(20);
    char c;
    while (c != SEPARATOR)
    {
        recv(client_fd, &c, 1, 0);
        strncat(text_size, &c, 1);
    }
    text_size[strlen(text_size)] = '\0';
    int size = atoi(text_size);
    char *text = malloc(size + 1);
    recv(client_fd, text, size, 0);
    text[size] = '\0';
    char *key_s = malloc(19);
    recv(client_fd, &key_s, 19, 0);
    // key= key_s
    unsigned long long key;
    printf("[SERVER] Chiave: %lld\nRicevuto testo: %s\n", key, text);

    // 7. Risposta
    const char *response = "Ciao dal server!";
    send(client_fd, response, strlen(response), 0);

    // 8. Chiudi
    close(client_fd);
    close(server_fd);
    printf("[SERVER] Connessione chiusa.\n");

    return 0;
}
