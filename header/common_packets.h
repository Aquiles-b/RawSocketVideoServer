#include <sys/types.h>

#include <arpa/inet.h>
#include <net/ethernet.h>
#include <linux/if_packet.h>
#include <string.h>
#include <net/if.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>


#define CRC_8_POLY 0xb
#define PACKET_MAX_SIZE 67
#define PACKET_MIN_SIZE 4

int send_ACK(int sockfd);

int send_NACK(int sockfd);

unsigned char calc_crc_8(const u_char *data, const u_short size);

short validate_packet(const u_char *data, const u_short size);
