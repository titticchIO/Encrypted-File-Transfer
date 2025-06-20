#include "header_server.h"
#include "functions_server.c"

int main()
{
    int server_fd, client_fd;
    init_socket(PORT, &server_fd, &client_fd);

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
