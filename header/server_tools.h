#ifndef _SERVER_TOOLS_H
#define _SERVER_TOOLS_H

#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "../header/common_packets.h"
#include "../header/basic_for_packets.h"

typedef struct movies {
    char **movies;
    unsigned long int num_movies;
} movies_t;

<<<<<<< HEAD
#define DATA_SIZE 63000 
=======
typedef struct window_packet_t {
    unsigned char *packet;
    struct window_packet_t *next_packet;
} window_packet_t;

<<<<<<< HEAD
#define DATA_SIZE 63000
#define WINDOW_SIZE 5
>>>>>>> 66e7900 ([FIX] janela deslizante)
=======
typedef struct window_packet_head_t {
    window_packet_t *head;
    window_packet_t *tail;
    unsigned long int size;
} window_packet_head_t;
>>>>>>> d42b23a ([FIX] janela deslizante (falta algumas coisas))

unsigned int get_num_files_dir(DIR *dir);

unsigned long int get_file_size(const char *file_path);

/* (Aloca memoria) Adiciona um vetor de nomes de arquivos em movies. */
void list_files_in_dir(char *dir_path, movies_t *movies);

/* Retorna 1 se o envio foi bem sucedido e 0 se nao foi. */
int send_movies_list(int sockfd, movies_t *movies);

/* Retorna 1 se todos os pacotes foram enviados com sucesso e 0 se nao foram. */
int send_packets_in_window(int sockfd, FILE *file_to_send);

/* Retorna 1 se o arquivo foi enviado com sucesso e 0 se nao foi. */
int send_file(int sockfd, char *file_name);

long int get_movie_index(const unsigned char *packet);

/* (Aloca memoria) Retorna NULL em caso de falha. */
unsigned char *create_packet_file_desc(int sockfd, char *file_name);

/* Retorna 1 se o descritor do arquivo foi enviado com sucesso e 0 se nao foi. */
int send_file_desc(int sockfd, char *file_name);

void free_window_packet_list(window_packet_t *w_packet_head);

/* (Aloca memoria). Monta uma lista window_packet_t de pacotes a partir
 * de um vetor de dados. */
window_packet_head_t *segment_data_in_window_packets(unsigned char *data, 
                                        const unsigned long int size, 
                                        unsigned char last_packet_code,
                                        unsigned char *sequence);

#endif // _SERVER_TOOLS_H
