#define _POSIX_C_SOURCE 200112L
#ifndef HEADER_H
#define HEADER_H

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <malloc.h>
#include <arpa/inet.h>
#include <math.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>

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
extern char *text_buffer;

// Funzioni dichiarate in functions_client.c
char *read_file(char *filename);
void block_signals(sigset_t set);
void unblock_signals(sigset_t set);
size_t divide_blocks(char *text, int p, size_t l);
void manage_threads(char *text, int blocks_per_thread, int blocks_last_thread, int p);
void *cypher_partial(void *void_args);
void cypher_block(const char *block, int offset);
unsigned long long string_to_bits(const char *str);
char *bits_to_string(unsigned long long bits);
int init_socket(int port, const char *server_ip, struct sockaddr_in *server_addr);
char *make_msg(unsigned long long key, char *text, size_t l);

#endif