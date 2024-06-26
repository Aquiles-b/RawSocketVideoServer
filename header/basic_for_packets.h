#ifndef _BASE_PACKETS_H_
#define _BASE_PACKETS_H_

#define CRC_8_POLY 0xb

#define PACKET_MAX_SIZE 67
#define PACKET_MIN_SIZE 14

#define INIT_MARKER 0x7e

/* Codigo de cada tipo */
#define ACK_COD 0x00
#define NACK_COD 0x01

/* CRC-8 pre calculados */
#define ACK_CRC 0x00
#define NACK_CRC 0x10

/* Retorna 1 se for valido e 0 se nao for. */
unsigned char calc_crc_8(unsigned char *data, const short size);

/* Retorna 1 se for valido e 0 se nao for. */
short validate_packet(unsigned char *data, const short size);

unsigned char calc_crc_8(unsigned char *data, const short size);

#endif // _BASE_PACKETS_H_
