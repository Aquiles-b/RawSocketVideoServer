#ifndef _SERVER_TOOLS_H
#define _SERVER_TOOLS_H

#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <stdlib.h>

#include "../header/common_packets.h"
#include "../header/basic_for_packets.h"

#define DATA_SIZE 63000 

unsigned int get_num_files_dir(DIR *dir);

unsigned long int get_file_size(const char *file_path);

/* (Aloca memoria) Retorna um vetor de nomes de arquivos no diretorio.
 * O ultimo elemento da lista eh NULL. */
char **list_files_in_dir(char *dir_path);

/* Retorna 1 se o envio foi bem sucedido e 0 se nao foi. */
int send_movies_list(int sockfd, char **movies);

/* Retorna 1 se todos os pacotes foram enviados com sucesso e 0 se nao foram. */
int send_seg_packets(unsigned char **packets, int sockfd);

/* Retorna 1 se o arquivo foi enviado com sucesso e 0 se nao foi. */
int send_file(int sockfd, char *file_name);

#endif // _SERVER_TOOLS_H
