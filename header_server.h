#ifndef HEADER_H
#define HEADER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 12345
#define BUF_SIZE 1024
#define SEPARATOR '\x1F'

void init_socket(int port, int *server_fd, int *client_fd);
char *receive_msg(int client_fd);
void get_key_and_text(char *msg, size_t *text_len, char **text, unsigned long long *key);
char *decypher_msg(char *msg, unsigned long long key);
char *decypher_block(char *block, unsigned long long key);

#endif