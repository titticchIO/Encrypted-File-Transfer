// client.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>

#define SERVER_IP "127.0.0.1"
#define PORT 12345
#define BUF_SIZE 1024

unsigned long long string_to_bits(const char *str)
{
    unsigned long long result = 0;
    size_t len = strlen(str);

    if (len > 8)
        len = 8; // massimo 8 caratteri (64 bit)

    for (size_t i = 0; i < len; i++)
    {
        result <<= 8;                    // lascia spazio per il prossimo carattere
        result |= (unsigned char)str[i]; // aggiungi i byte in fondo
    }

    return result;
}

int main()
{
    srand(time(NULL)); // inizializza il generatore con il tempo attuale

    char key_s[9];
    for (int i = 0; i < 8; i++)
    {
        key_s[i] = 'a' + rand() % 26;
    }
    key_s[8] = '\0'; // termina la stringa
    unsigned long long key = string_to_bits(key_s);

    int sockfd;
    struct sockaddr_in server_addr;
    char buffer[BUF_SIZE];

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
    const char *msg = "Ciao dal client!";
    // Manda la chiave al server con send
    send(sockfd, &key, sizeof(key), 0);

    // 5. Riceve risposta
    memset(buffer, 0, BUF_SIZE);
    recv(sockfd, buffer, BUF_SIZE, 0);
    printf("[CLIENT] Risposta: %s\n", buffer);

    // 6. Chiudi
    close(sockfd);

    return 0;
}
