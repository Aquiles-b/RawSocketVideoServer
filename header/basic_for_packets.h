#ifndef _BASE_PACKETS_H_
#define _BASE_PACKETS_H_

#define CRC_8_POLY 0xb

#define PACKET_MAX_SIZE 67
#define PACKET_MIN_SIZE 14

#define INIT_MARKER 0x7e

/* Codigo de cada tipo */
#define ACK_COD 0x00
#define NACK_COD 0x01
#define DATA_COD 0x12
#define END_DATA_COD 0x1e

/* CRC-8 pre calculados */
#define ACK_CRC 0x2d
#define NACK_CRC 0x3d

#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

unsigned char calc_crc_8(unsigned char *data, const short size);

/* Retorna 1 se for valido e 0 se nao for. */
short validate_crc_8(unsigned char *data, const short size);

/* Retorna o tamanho real do pacote se for valido e 0 se nao for. */
short validate_packet(unsigned char *data, const short size);

/* (Faz alocacao de memoria). A ultima posicao do vetor eh NULL. */
unsigned char **segment_data_in_packets(unsigned char *data, 
                                        const unsigned long int size);

void free_packets(unsigned char ***packets);

short send_packet(int sockfd, unsigned char *packet);

#endif // _BASE_PACKETS_H_
