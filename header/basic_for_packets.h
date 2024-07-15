#ifndef _BASE_PACKETS_H_
#define _BASE_PACKETS_H_

#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

#define CRC_8_POLY 0xb

#define PACKET_SIZE 67

#define INIT_MARKER 0x7e

/* Codigo de cada tipo */
#define ACK_COD 0x00
#define NACK_COD 0x01
#define DATA_COD 0x12
#define END_DATA_COD 0x1e
#define LIST_FILES_COD 0x0a
#define DOWNLOAD_FILE_COD 0x0b
#define SHOW_IN_SCREEN_COD 0x10
#define FILE_DESC_COD 0x11
#define ERROR_COD 0x1f

/* CRC-8 pre calculados */
#define ACK_CRC 0x2d
#define NACK_CRC 0x3d

unsigned char calc_crc_8(const unsigned char *data, const short size);

/* Retorna 1 se for valido e 0 se nao for. */
int validate_crc_8(const unsigned char *data, const short size);

/* Retorna 1 se for valido e 0 se nao for. */
int validate_packet(const unsigned char *data, const short size);

/* (Aloca memoria). Retorna NULL em caso de falha */
unsigned char *create_packet(const unsigned char *data, unsigned short size_data,
                        unsigned char seq, unsigned char code);

void clear_socket_buffer(int sockfd);

unsigned char get_packet_code(const unsigned char *packet);

unsigned char get_packet_seq(const unsigned char *packet);

unsigned short get_packet_data_size(const unsigned char *packet);

#endif // _BASE_PACKETS_H_
