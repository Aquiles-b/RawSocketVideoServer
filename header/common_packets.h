#include <sys/socket.h>

#define CRC_8_POLY 0xb
#define PACKET_MAX_SIZE 67
#define PACKET_MIN_SIZE 14

int send_ACK(int sockfd);

int send_NACK(int sockfd);

char calc_crc_8(const char *data, const short size);

short validate_packet(const char *data, const short size);
