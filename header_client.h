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
#define EOT '\x03'

typedef struct
{
    char *partial;
    int offset;
} thread_args;

extern unsigned long long key;
extern char *text_buffer;

// Funzioni dichiarate in functions_client.c
void read_args(char **argv, char **file_name, char **key_s, int *p, char **ip, int *port); // Legge e valida gli argomenti da linea di comando
char *read_file(char *filename);                                                           // Legge il contenuto di un file e lo restituisce come stringa
sigset_t get_set();                                                                        // Restituisce un set di segnali da gestire
void block_signals(sigset_t set);                                                          // Blocca i segnali specificati
void unblock_signals(sigset_t set);                                                        // Sblocca i segnali specificati
size_t divide_blocks(char *text, int p, size_t L);                                         // Divide il testo in blocchi da cifrare e gestisce il padding
void manage_threads(char *text, int blocks_per_thread, int blocks_last_thread, int p);     // Gestisce la creazione e sincronizzazione dei thread di cifratura
void *cypher_partial(void *void_args);                                                     // Funzione eseguita da ciascun thread per cifrare una porzione di testo
void cypher_block(const char *block, int offset);                                          // Cifra un blocco di testo e lo scrive nel buffer globale
unsigned long long string_to_bits(const char *str);                                        // Converte una stringa di 8 caratteri in un intero a 64 bit
char *bits_to_string(unsigned long long bits);                                             // Converte un intero a 64 bit in una stringa di 8 caratteri
int init_socket(int port, const char *server_ip, struct sockaddr_in *server_addr);         // Inizializza e restituisce un socket TCP
char *make_msg(unsigned long long key, char *text, size_t l, size_t *msg_len);             // Crea il messaggio da inviare al server secondo il protocollo

#endif