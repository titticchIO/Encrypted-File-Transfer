// client.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "functions_client.c"
#include "header_client.h"

#define SERVER_IP "127.0.0.1"
#define PORT 12345
#define BUF_SIZE 1024

// int main(int argc, char *argv[])
// {
//     char *text = read_file("prova.txt");
//     argv[1] = "questaèunastringamoltolunga";
//     char key_s[9] = "\0\0\0\0\0\0\0\0"; // 8 caratteri + terminatore null
//     strncpy(key_s, argv[1], 8);

//     unsigned long long key = string_to_bits(key_s);

//     int sockfd;
//     struct sockaddr_in server_addr;
//     char buffer[BUF_SIZE];

//     // 1. Crea socket
//     sockfd = socket(AF_INET, SOCK_STREAM, 0);
//     if (sockfd < 0)
//     {
//         perror("socket");
//         exit(1);
//     }

//     // 2. Imposta indirizzo del server
//     server_addr.sin_family = AF_INET;
//     server_addr.sin_port = htons(PORT);
//     inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr);

//     // 3. Connessione
//     if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
//     {
//         perror("connect");
//         close(sockfd);
//         exit(1);
//     }

//     // 4. Invia messaggio
//     message msg;
//     msg.key = key;
//     msg.text = text;
//     // Manda la chiave al server con send
//     send(sockfd, &msg, sizeof(msg), 0);
//     // 5. Riceve risposta
//     memset(buffer, 0, BUF_SIZE);
//     recv(sockfd, buffer, BUF_SIZE, 0);
//     printf("[CLIENT] Risposta: %s\n", buffer);

//     // 6. Chiudi
//     close(sockfd);

//     return 0;
// }
int main()
{
    char *text = "abcdefgh";
    unsigned long long key = string_to_bits("emiliano");
    char *cypered_text = cypher_block(text, key); // Esempio di chiave

    printf("Testo cifrato (hex): ");
    for (int i = 0; i < 8; i++)
    {
        printf("%02x ", (unsigned char)cypered_text[i]);
    }
    printf("\n");

    free(cypered_text);
    return 0;
}