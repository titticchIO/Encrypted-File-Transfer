#include "header_server.h"

char *receive_msg(int client_fd)
{
    char buffer[BUF_SIZE];
    memset(buffer, 0, sizeof(buffer));

    char *msg = NULL;
    int total_received = 0;
    do
    {
        int n = recv(client_fd, buffer, BUF_SIZE, 0);
        if (n <= 0)
            break;

        // Alloca o rialloca msg
        char *new_msg = realloc(msg, total_received + n + 1); // +1 per terminatore
        if (!new_msg)
        {
            free(msg);
            perror("realloc");
            break;
        }
        msg = new_msg;

        // Copia i nuovi dati
        memcpy(msg + total_received, buffer, n);
        total_received += n;

    } while (msg[total_received - 1] != SEPARATOR);
    return msg;
}

void get_key_and_text(char *msg, char **text, unsigned long long *key)
{
    char *sep1 = strchr(msg, SEPARATOR);
    if (!sep1)
    {
        *key = 0;
        *text = NULL;
        return;
    }
    // Estrai la chiave
    *sep1 = '\0';
    *key = strtoull(msg, NULL, 10);

    // Trova il secondo separatore
    char *sep2 = strchr(sep1 + 1, SEPARATOR);
    if (!sep2)
    {
        *text = NULL;
        return;
    }
    size_t text_len = sep2 - (sep1 + 1);
    *text = malloc(text_len + 1);
    if (!*text)
        return;
    memcpy(*text, sep1 + 1, text_len);
    (*text)[text_len] = '\0';
}

char *decypher_msg(char *msg, unsigned long long key)
{
    return NULL;
}

char *decypher_block(char *block, unsigned long long key)
{
    // unsigned long long block_bytes = string_to_bits(block);
    // unsigned long long decypher_bytes = block_bytes ^ key;
    // return bits_to_string(decypher_bytes);
    return NULL;
}