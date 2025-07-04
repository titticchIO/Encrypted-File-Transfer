#define _GNU_SOURCE
#define _POSIX_C_SOURCE 200809L
#ifndef HEADER_SERVER_H
#define HEADER_SERVER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <semaphore.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <signal.h>
#include <errno.h>
#include <sys/syscall.h>
#include <sys/types.h>

// Costanti per la configurazione del server
#define PORT 50000       // Porta su cui il server si mette in ascolto
#define BUF_SIZE 1024    // Dimensione del buffer per la ricezione dei dati
#define SEPARATOR '\x1F' // Carattere usato per separare i campi nel messaggio
#define EOT '\x03'       // Carattere usato per il padding

// Argomenti per i thread di decifratura
typedef struct
{
    char *partial;
    int offset;
    unsigned long long key;
    char *text_buffer;
    int blocks_num;
} d_thread_args;

// Argomenti per i thread che gestiscono le connessioni dei client
typedef struct
{
    int client_fd; // File descriptor del socket del client
    int serial;    // Numero seriale della connessione
} c_thread_args;

// Dichiarazione di variabili globali
extern pthread_t main_thread_id;    // ID del thread principale del server
extern sem_t available_connections; // Semaforo per limitare le connessioni concorrenti
extern int p;                       // Grado di parallelismo per la decifratura
extern int server_fd;               // File descriptor del socket del server
extern char *s;                     // Prefisso per il nome dei file di output

// Ritardi per testare il blocco dei segnali
extern int delay_unblocked;
extern int delay_decypher;
extern int delay_writing;

// Setup
void read_args(int argc, char **argv, int *p, char **s, int *l); // Legge e valida gli argomenti da linea di comando
void init_socket(int port, int *server_fd);                      // Inizializza il socket del server

// Gestione connessioni
void manage_connections();              // Gestisce le nuove connessioni in ingresso
void *manage_client_message(void *arg); // Gestisce la comunicazione con un singolo client (thread)

// Gestisce la comunicazione con un singolo client (thread)
char *receive_msg(int client_fd);                                                         // Riceve un messaggio completo dal client
void get_key_and_text(char *msg, unsigned long long *key, size_t *text_len, char **text); // Estrae chiave e testo dal messaggio ricevuto

// Decifratura
char *manage_threads(char *text, size_t text_len, unsigned long long key);               // Gestisce la decifratura parallela del testo
void *decypher_partial(void *arg);                                                       // Funzione eseguita da ciascun thread per decifrare una porzione di testo
void decypher_block(char *block, int offset, unsigned long long key, char *text_buffer); // Decifra un blocco di testo e lo scrive nel buffer
unsigned long long string_to_bits(const char *str);                                      // Converte una stringa di 8 caratteri in un intero a 64 bit
char *bits_to_string(unsigned long long bits);                                           // Converte un intero a 64 bit in una stringa di 8 caratteri

// Scrittura su file di output
void write_file(char *text, char *pathfile);                                                                // Scrive il testo decifrato su file
void cleanup_client_resources(int client_fd, char *msg, char *text, char *decyphered_text, char *filename); // Pulisce tutte le risorse del client

// Gestione segnali
void setup_signal_handlers();         // Imposta i gestori dei segnali per la terminazione sicura
sigset_t get_set();                   // Restituisce un set di segnali da gestire
void termination_handler(int signum); // Gestisce la terminazione del server su segnale
void block_signals(sigset_t set);     // Blocca i segnali specificati
void unblock_signals(sigset_t set);   // Sblocca i segnali specificati

#endif