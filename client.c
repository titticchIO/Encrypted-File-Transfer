// client.c
#include "functions_client.c"
#include "header_client.h"

#define SERVER_IP "127.0.0.1"
#define PORT 12345
#define BUF_SIZE 1024

int main(int argc, char *argv[])
{
    // argomenti
    int sockfd;
    struct sockaddr_in server_addr;
    int p = 5;
    argv[1] = "aaaaaaaaaaaaaaaaaaaaaaaaaaaa";
    // argomenti

    char key_s[9] = "\0\0\0\0\0\0\0\0"; // 8 caratteri + terminatore null
    strncpy(key_s, argv[1], 8);
    char *text = read_file("mess_lungo.txt");
    int L = strlen(text);
    text_buffer = malloc(L + 1);
    text_buffer[L] = '\0';
    key = string_to_bits(key_s);

    // Blocca i segnali SIGINT, SIGALRM, SIGUSR1, SIGUSR2, SIGTERM
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGINT);
    sigaddset(&set, SIGALRM);
    sigaddset(&set, SIGUSR1);
    sigaddset(&set, SIGUSR2);
    sigaddset(&set, SIGTERM);

    printf("Testo: %s\nTesto cifrato: %s\n", text, text_buffer);
    // Stampa un char alla volta con un for
    for (int i = 0; i < L; i++)
    {
        printf("%02x ", (unsigned char)text_buffer[i]);
    }
    printf("\n");

    sockfd = init_socket(PORT, SERVER_IP, &server_addr);

    // 3. Connessione
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("connect");
        close(sockfd);
        exit(1);
    }

    block_signals(set);
    divide_blocks(text, p, L);
    unblock_signals(set);

    // 4. Invia messaggio
    char *msg = make_msg(key, text_buffer);

    // Manda la chiave al server con send
    printf("[CLIENT] Inviando messaggio: %s\n", msg);
    send(sockfd, msg, strlen(msg), 0);
    free(msg);

    char buffer[4];
    // 5. Riceve risposta
    memset(buffer, 0, 4);
    recv(sockfd, buffer, 4, 0);
    printf("[CLIENT] ACK ricevuto\n");
    // 6. Chiudi
    close(sockfd);

    return 0;
}