#define _POSIX_C_SOURCE 200112L
#ifndef HEADER_H
#define HEADER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <semaphore.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <signal.h>

#define PORT 12345
#define BUF_SIZE 1024
#define SEPARATOR '\x1F'

extern sem_t available_connections;
extern int p;
extern int server_fd;

typedef struct
{
    char *partial;
    int offset;
    unsigned long long key;
    char *text_buffer;
    int blocks_num;
} thread_args;

// Funzioni di functions_server.c
void termination_handler(int signum);
void init_socket(int port, int *server_fd);
void manage_connections();
char *receive_msg(int client_fd);
void get_key_and_text(char *msg, unsigned long long *key, size_t *text_len, char **text);
void *manage_client_message(void *arg);
char *manage_threads(char *text, size_t text_len, unsigned long long key);
void *decypher_partial(void *arg);
void decypher_block(char *block, int offset, unsigned long long key, char *text_buffer);
unsigned long long string_to_bits(const char *str);
char *bits_to_string(unsigned long long bits);

#endif