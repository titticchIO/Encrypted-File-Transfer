// client.c
#include "functions_client.c"
#include "header_client.h"

#define SERVER_IP "127.0.0.1"
#define PORT 12345
#define BUF_SIZE 1024

int main(int argc, char *argv[])
{
    struct sockaddr_in server_addr;
    // argomenti da terminale
    int sockfd, port, p;
    char *filename, *key_s, *ip;
    read_args(argv, &filename, &key_s, &p, &ip, &port);

    int n;
    sigset_t set;
    size_t l, msg_len, orig_l;
    char buffer[4], *msg, *text;

    // char *text = read_file("mess_lungo.txt");
    text = read_file(filename);
    orig_l = strlen(text);
    // text_buffer = malloc(l + 1);
    // text_buffer[l] = '\0';
    key = string_to_bits(key_s);

    sockfd = init_socket(port, ip, &server_addr);

    // Connessione al server
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("connect");
        close(sockfd);
        exit(1);
    }

    // Marchera dei segnali
    set = get_set();
    block_signals(set);
    l = divide_blocks(text, p, orig_l);
    unblock_signals(set);

    // Composizione del messaggio
    msg = make_msg(key, text_buffer, l, &msg_len);

    // Invio del messaggio al server
    printf("[CLIENT] Invio messaggio...\n");
    n = send(sockfd, msg, msg_len, 0);

    // Ricezione ACK dal server
    memset(buffer, 0, 4);
    recv(sockfd, buffer, 4, 0);
    if (strcmp(buffer, "ACK") == 0)
    {
        printf("[CLIENT] ACK ricevuto\n");
    }

    // Chiusura della connessione
    close(sockfd);
    free(msg);
    return 0;
}