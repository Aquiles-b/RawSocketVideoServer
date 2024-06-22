#include "../header/common_packets.h"

int send_ACK(int sockfd) {
    char ack[PACKET_MIN_SIZE] = {0};
    ack[0] = 0x7e;
    return send(sockfd, ack, PACKET_MIN_SIZE, 0);
}

int send_NACK(int sockfd) {
    char nack[PACKET_MIN_SIZE] = {0};
    nack[0] = 0x7e;
    nack[3] = 0x0b;
    return send(sockfd, nack, PACKET_MIN_SIZE, 0);
}

char calc_crc_8(const char *data, const short size) {
    unsigned char crc = 0;
    for (int i = 0; i < size; i++) {
        crc ^= data[i];
        for (int j = 0; j < 8; j++) {
            if (crc & 0x80) {
                crc = (crc << 1) ^ CRC_8_POLY;
            } else {
                crc <<= 1;
            }
        }
    }
    return crc;
}

/* Retorna 1 se for valido e 0 se nao for. */
short validate_crc_8(const char *data, const short size) {
    data = data + 1;
    return calc_crc_8(data, size-1) == data[size - 1];
}

/* Retorna 1 se for valido e 0 se nao for. */
short validate_packet(const char *data, const short size) {
    return size >= PACKET_MIN_SIZE && data[0] == 0x7e && validate_crc_8(data, size);
}
