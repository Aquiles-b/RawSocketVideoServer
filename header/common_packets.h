#ifndef _COMMON_PACKETS_H_
#define _COMMON_PACKETS_H_

#include "basic_for_packets.h"
#include <time.h>

#define ERROR_ACCESS_DENIED 1
#define ERROR_NOT_FOUND 2
#define ERROR_DISK_FULL 3

#define TIMEOUT 1

int send_ACK(int sockfd, unsigned char seq);

int send_NACK(int sockfd, unsigned char seq);

int send_packet(int sockfd, unsigned char *packet);

/* Retorna 1 em caso de sucesso e 0 em falha. */
int recv_packet_in_timeout(int sockfd, unsigned char *buffer);

/* Retorna 1 em caso de sucesso e 0 em falha. */
int send_packet_with_confirm(int sockfd, unsigned char *packet, unsigned char *buffer);

int send_error(int sockfd, unsigned char error_code);

int send_packet_in_timeout(int sockfd, unsigned char *packet);

unsigned char get_error_type(const unsigned char *packet);

#endif // _COMMON_PACKETS_H_
