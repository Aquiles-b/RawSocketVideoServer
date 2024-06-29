#include "../header/common_packets.h"

int send_ACK(int sockfd) {
    unsigned char ack[PACKET_MIN_SIZE] = {0};
    ack[0] = INIT_MARKER;
    ack[2] = ACK_COD;
    ack[PACKET_MIN_SIZE - 1] = calc_crc_8(ack + 1, PACKET_MIN_SIZE - 2);
    return send(sockfd, ack, PACKET_MIN_SIZE, 0);
}

int send_NACK(int sockfd) {
    unsigned char nack[PACKET_MIN_SIZE] = {0};
    nack[0] = INIT_MARKER;
    nack[2] = NACK_COD;
    nack[PACKET_MIN_SIZE - 1] = calc_crc_8(nack + 1, PACKET_MIN_SIZE - 2);
    return send(sockfd, nack, PACKET_MIN_SIZE, 0);
}
