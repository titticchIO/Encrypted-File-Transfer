#include "header_server.h"
#include "functions_server.c"

int main()
{
    int server_fd, client_fd;
    struct sockaddr_in addr;
    socklen_t addr_len = sizeof(addr);

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

    char *msg = receive_msg(client_fd);
    printf("[SERVER] Messaggio ricevuto: %s\n", msg);

    char *text = NULL;
    unsigned long long key = 0;
    // 6. Estrai chiave e testo dal messaggio
    get_key_and_text(msg, &text, &key);
    printf("[SERVER] Chiave: %llu, Testo: %s\n", key, text);

    // decypher_msg(msg, 0); // Placeholder per decifrare il messaggio
    // 7. Manda l'ACK al client
    const char *response = "ACK";
    send(client_fd, response, strlen(response), 0);

    // 8. Termina connessione
    close(client_fd);
    close(server_fd);
    printf("[SERVER] Connessione chiusa.\n");

    return 0;
}
