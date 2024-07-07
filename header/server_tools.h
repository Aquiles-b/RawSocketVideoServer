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

#define DATA_SIZE 63000 

unsigned int get_num_files_dir(DIR *dir);

unsigned long int get_file_size(const char *file_path);

/* (Aloca memoria) Adiciona um vetor de nomes de arquivos em movies. */
void list_files_in_dir(char *dir_path, movies_t *movies);

/* Retorna 1 se o envio foi bem sucedido e 0 se nao foi. */
int send_movies_list(int sockfd, movies_t *movies);

/* Retorna 1 se todos os pacotes foram enviados com sucesso e 0 se nao foram. */
int send_seg_packets(unsigned char **packets, int sockfd);

/* Retorna 1 se o arquivo foi enviado com sucesso e 0 se nao foi. */
int send_file(int sockfd, char *file_name);

unsigned long int get_file_index(const unsigned char *packet);

/* (Aloca memoria) Retorna NULL em caso de falha. */
unsigned char *create_packet_file_desc(int sockfd, char *file_name);

/* Retorna 1 se o descritor do arquivo foi enviado com sucesso e 0 se nao foi. */
int send_file_desc(int sockfd, char *file_name);

#endif // _SERVER_TOOLS_H
