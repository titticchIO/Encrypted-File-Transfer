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
    argv[1] = "emiliano";
    // argomenti
    // fprintf(stderr, "%s", buffer);
    if (strlen(argv[1]) != 8)
    { // controllo lunghezza chiave
        fprintf(stderr, "%s", "La chiave deve avere lunghezza 8\n");
        exit(1);
    }
    char *key_s = calloc(9, sizeof(char));
    strncpy(key_s, argv[1], 8);
    key_s[8] = '\0';

    printf("Key: %s\n", key_s);
    // char *text = read_file("mess_lungo.txt");
    char *text = read_file("prova.txt");
    size_t orig_l = strlen(text);
    // text_buffer = malloc(l + 1);
    // text_buffer[l] = '\0';
    key = string_to_bits(key_s);

    // Blocca i segnali SIGINT, SIGALRM, SIGUSR1, SIGUSR2, SIGTERM
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGINT);
    sigaddset(&set, SIGALRM);
    sigaddset(&set, SIGUSR1);
    sigaddset(&set, SIGUSR2);
    sigaddset(&set, SIGTERM);

    // printf("Testo: %s\nTesto cifrato: %s\n", text, text_buffer);
    // Stampa un char alla volta con un for
    // for (int i = 0; i < l; i++)
    // {
    //     printf("%02x ", (unsigned char)text_buffer[i]);
    // }
    // printf("\n");

    sockfd = init_socket(PORT, SERVER_IP, &server_addr);

    // 3. Connessione
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("connect");
        close(sockfd);
        exit(1);
    }

    block_signals(set);
    size_t l = divide_blocks(text, p, orig_l);
    unblock_signals(set);

    // 4. Invia messaggio
    size_t msg_len;
    char *msg = make_msg(key, text_buffer, l, &msg_len);
    // printf("Lunghezza messaggio: %d",msg)
    // Manda la chiave al server con send
    printf("[CLIENT] Inviando messaggio: %s\n", msg);
    int n = send(sockfd, msg, msg_len, 0);
    free(msg);
    printf("Lunghezza mes: %d", n);
    char buffer[4];
    // 5. Riceve risposta
    memset(buffer, 0, 4);
    recv(sockfd, buffer, 4, 0);
    printf("[CLIENT] ACK ricevuto\n");
    // 6. Chiudi
    close(sockfd);

    return 0;
}