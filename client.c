// client.c
#include "functions_client.c"
#include "header_client.h"

#define SERVER_IP "127.0.0.1"
#define PORT 12345
#define BUF_SIZE 1024

int main(int argc, char *argv[])
{
    struct sockaddr_in server_addr;
    // argomenti
    int sockfd, port, p;
    char *filename, *key_s, *ip;
    read_args(argv, &filename, &key_s, &p, &ip, &port);

    // char *text = read_file("mess_lungo.txt");
    char *text = read_file(filename);
    size_t orig_l = strlen(text);
    // text_buffer = malloc(l + 1);
    // text_buffer[l] = '\0';
    key = string_to_bits(key_s);

    sockfd = init_socket(port, ip, &server_addr);

    // 3. Connessione
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("connect");
        close(sockfd);
        exit(1);
    }

    sigset_t set = get_set();
    block_signals(set);
    size_t l = divide_blocks(text, p, orig_l);
    unblock_signals(set);

    // 4. Invia messaggio
    size_t msg_len;
    char *msg = make_msg(key, text_buffer, l, &msg_len);

    // Manda la chiave al server con send
    printf("[CLIENT] Invio messaggio...\n");
    int n = send(sockfd, msg, msg_len, 0);
    free(msg);
    char buffer[4];
    // 5. Riceve risposta
    memset(buffer, 0, 4);
    recv(sockfd, buffer, 4, 0);
    if (strcmp(buffer, "ACK") == 0)
    {
        printf("[CLIENT] ACK ricevuto\n");
    }
    // 6. Chiudi
    close(sockfd);
    return 0;
}