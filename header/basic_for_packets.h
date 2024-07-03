#ifndef _BASE_PACKETS_H_
#define _BASE_PACKETS_H_

#define CRC_8_POLY 0xb

#define PACKET_SIZE 67

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

unsigned char calc_crc_8(const unsigned char *data, const short size);

/* Retorna 1 se for valido e 0 se nao for. */
int validate_crc_8(const unsigned char *data, const short size);

/* Retorna 1 se for valido e 0 se nao for. */
int validate_packet(const unsigned char *data, const short size);

/* (Aloca memoria). A ultima posicao do vetor eh NULL. */
unsigned char **segment_data_in_packets(unsigned char *data, 
                                        const unsigned long int size, 
                                        unsigned char last_packet_code);
void free_packets(unsigned char ***packets);

void clear_socket_buffer(int sockfd);

#endif // _BASE_PACKETS_H_
