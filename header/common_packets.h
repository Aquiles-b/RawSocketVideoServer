#ifndef _COMMON_PACKETS_H_
#define _COMMON_PACKETS_H_

#include <sys/socket.h>
#include "basic_for_packets.h"

#define ERROR_ACCESS_DENIED 1
#define ERROR_NOT_FOUND 2
#define ERROR_DISK_FULL 3

int send_ACK(int sockfd, unsigned char seq);

int send_NACK(int sockfd, unsigned char seq);

int send_packet(int sockfd, unsigned char *packet);

int send_error(int sockfd, unsigned char error_code);

#endif // _COMMON_PACKETS_H_
