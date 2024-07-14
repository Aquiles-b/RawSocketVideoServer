#ifndef _CLIENT_TOOLS_H
#define _CLIENT_TOOLS_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../header/basic_for_packets.h"
#include "../header/common_packets.h"

/* Imprime a mensagem text e retorna a opcao escolhida pelo usuario.
 * A opcao nao pode ser 0! */
long int get_user_input(char *text);

int view_movies_list(int sockfd, unsigned char *packet_server);

/* Retorna 1 caso sucesso, 0 caso falha */
int recv_file(int sockfd, char *filename, unsigned long int file_size);

/* Cuida do recebimento de um pacote de descritor de arquivo.
 * Retorna 1 caso sucesso, 0 caso falha */
int handle_recv_file_desc_packet(int sockfd, unsigned char *packet_server);

void show_movie_date_size_packet(unsigned char *packet_server);

#endif // _CLIENT_TOOLS_H
