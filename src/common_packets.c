#include "../header/common_packets.h"

int send_ACK(int sockfd) {
    u_int ack = 0x7e000000;
    return send(sockfd, &ack, 4, 0);
}

int send_NACK(int sockfd) {
    u_int nack = 0x7e00000b;
    return send(sockfd, &nack, 4, 0);
}

unsigned char calc_crc_8(const u_char *data, const u_short size) {
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
short validate_crc_8(const u_char *data, const u_short size) {
    data = data + 1;
    return calc_crc_8(data, size-1) == data[size - 1];
}

/* Retorna 1 se for valido e 0 se nao for. */
short validate_packet(const u_char *data, const u_short size) {
    return data[0] == 0x7e && validate_crc_8(data, size);
}
