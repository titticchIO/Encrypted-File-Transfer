// #include "functions_client.c"
#include "header_client.h"

int main(int argc, char *argv[])
{
    struct sockaddr_in server_addr;
    int sockfd, port, p;
    char *filename, *key_s, *ip;
    char *text;
    size_t orig_l, l;

    // Lettura e validazione argomenti
    read_args(argc, argv, &filename, &key_s, &p, &ip, &port);

    // Lettura file
    text = read_file(filename);
    if (!text)
    {
        fprintf(stderr, "Error reading file %s\n", filename);
        exit(1);
    }
    orig_l = strlen(text);

    // Converte una stringa chiave in formato numerico
    key = string_to_bits(key_s);

    // Connessione al server
    sockfd = connect_to_server(port, ip, &server_addr);

    // Cifratura del testo
    l = encrypt_msg(&text, p, orig_l);

    // Invio messaggio
    send_message_to_server(sockfd, key, text_buffer, l);

    // Ricezione ACK
    receive_ack(sockfd);

    // Cleanup
    close(sockfd);
    free(text);
    if (text_buffer)
    {
        free(text_buffer);
        text_buffer = NULL;
    }
}