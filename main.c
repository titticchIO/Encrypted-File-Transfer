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
    char *text = "Hello, World!";
    unsigned long long bits = string_to_bits(text);
}
