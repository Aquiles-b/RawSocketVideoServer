#ifndef _COMMON_PACKETS_H_
#define _COMMON_PACKETS_H_

#include <sys/socket.h>
#include "basic_for_packets.h"

int send_ACK(int sockfd);

int send_NACK(int sockfd);

#endif // _COMMON_PACKETS_H_
