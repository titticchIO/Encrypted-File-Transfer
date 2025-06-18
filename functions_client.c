#include "header_client.h"

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

char *bits_to_string(unsigned long long bits)
{
    char *str = malloc(9); // 8 byte + terminatore di stringa
    for (int i = 7; i >= 0; i--)
    {
        str[i] = (char)(bits & 0xFF);
        bits >>= 8;
    }
    str[8] = '\0'; // Terminatore di stringa
    return str;
}

char *make_msg(unsigned long long key, char *text)
{
    int msg_size = sizeof(int) + sizeof(char) + strlen(text) + sizeof(unsigned long long);
    char *msg = malloc(msg_size);
    sprintf(msg, "%ld%c%s%lld", strlen(text), SEPARATOR, text, key);
    printf("[CLIENT] Messaggio: %s\n", msg);
    return msg;
}

char *cypher_block(char *block, unsigned long long key)
{
    unsigned long long block_bytes = string_to_bits(block);
    unsigned long long cypher_bytes = block_bytes ^ key;
    return bits_to_string(cypher_bytes);
}

char *cypher_text(char *text, unsigned long long key)
{

    return text; // Placeholder for the actual ciphering logic
}
char *read_file(char *filename, unsigned long long key)
{
    FILE *file = fopen(filename, "r");
    if (!file)
    {
        perror("Errore nell'apertura del file");
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long dimensione = ftell(file);
    rewind(file);

    char *buffer = malloc(dimensione + 1);
    if (!buffer)
    {
        perror("Errore nell'allocazione di memoria");
        fclose(file);
        return NULL;
    }

    int letti = fread(buffer, sizeof(char), dimensione, file);
    buffer[letti] = '\0';
    fclose(file);
    return cypher_text(buffer, key);
}
