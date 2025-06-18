#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "header_client.h"
#include "functions_client.c"

int main()
{
    // chiave=1
    unsigned long long key = 0x0102030405060708; // 64 bit, 8 byte
    // converti key in stringa
    char *key_s = bits_to_string(key);
    printf("Chiave: %s\n%ld\n", key_s, strlen(key_s));
    return 0;
}
