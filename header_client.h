#ifndef HEADER_H
#define HEADER_H

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <malloc.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 12345
#define BUF_SIZE 1024
#define TEXT_SIZE 1024
#define SEPARATOR '\x1F'

unsigned long long string_to_bits(const char *str);
char *bits_to_string(unsigned long long bits);
char *cypher_block(char *block, unsigned long long key);
char *cypher_text(char *text, unsigned long long key);
char *read_file(char *filename, unsigned long long key);
char *make_msg(unsigned long long key, char *text);

#endif