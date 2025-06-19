// client.c
#include "functions_client.c"
#include "header_client.h"

#define SERVER_IP "127.0.0.1"
#define PORT 12345
#define BUF_SIZE 1024

int main(int argc, char *argv[])
{
    argv[1] = "aaaaaaaaaaaaaaaaaaaaaaaaaaaa";
    char key_s[9] = "\0\0\0\0\0\0\0\0"; // 8 caratteri + terminatore null
    strncpy(key_s, argv[1], 8);

    unsigned long long key = string_to_bits(key_s);
    // printf("Chiave in bits: %llu\n", key);
    char *text = read_file("prova.txt", key);

    int sockfd;
    struct sockaddr_in server_addr;

    // 1. Crea socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        perror("socket");
        exit(1);
    }

    // 2. Imposta indirizzo del server
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr);

    // 3. Connessione
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("connect");
        close(sockfd);
        exit(1);
    }

    // 4. Invia messaggio
    char *msg = make_msg(key, text);

    // Manda la chiave al server con send
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

// malloc(): invalid next size
//  TEST CIFRATURA
//  int main()
//  {
//      char *text = "abcdefgh";
//      unsigned long long key = string_to_bits("emiliano");
//      char *cypered_text = cypher_block(text, key); // Esempio di chiave

//     printf("Testo cifrato (hex): ");
//     for (int i = 0; i < 8; i++)
//     {
//         printf("%02x ", (unsigned char)cypered_text[i]);
//     }
//     printf("\n");

//     free(cypered_text);
//     return 0;
// }