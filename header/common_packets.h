#ifndef _COMMON_PACKETS_H_
#define _COMMON_PACKETS_H_

#include <sys/socket.h>
#include "basic_for_packets.h"

int send_ACK(int sockfd, unsigned char seq);

int send_NACK(int sockfd, unsigned char seq);

short send_packet(int sockfd, unsigned char *packet);

#endif // _COMMON_PACKETS_H_
