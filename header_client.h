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
#include <math.h>
#include <pthread.h>
#define PORT 12345
#define BUF_SIZE 1024
#define TEXT_SIZE 1024
#define SEPARATOR '\x1F'

typedef struct
{
    char *partial;
    int offset;
} thread_args;

extern unsigned long long key;
extern char *msg_buffer;

// Funzioni dichiarate in functions_client.c
int init_socket(int port, const char *server_ip);
void divide_blocks(char *text, int p, int L);
void manage_threads(char *text, int blocks_per_thread, int blocks_last_thread, int p);
void *cypher_partial(void *void_args);
void cypher_block(const char *block, int offset);
unsigned long long string_to_bits(const char *str);
char *bits_to_string(unsigned long long bits);
char *make_msg(unsigned long long key, char *text);
char *read_file(char *filename);

#endif