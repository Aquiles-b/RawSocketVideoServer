#include "../header/common_packets.h"

int send_ACK(int sockfd, unsigned char seq) {
    unsigned char ack[PACKET_SIZE] = {0};
    ack[0] = INIT_MARKER;
    ack[1] = seq >> 3;
    ack[2] = (seq << 5) | ACK_COD;
    ack[PACKET_SIZE - 1] = calc_crc_8(ack + 1, PACKET_SIZE - 2);
    return send(sockfd, ack, PACKET_SIZE, 0);
}

int send_NACK(int sockfd, unsigned char seq) {
    unsigned char nack[PACKET_SIZE] = {0};
    nack[0] = INIT_MARKER;
    nack[1] = seq >> 3;
    nack[2] = (seq << 5) | NACK_COD;
    nack[PACKET_SIZE - 1] = calc_crc_8(nack + 1, PACKET_SIZE - 2);
    return send(sockfd, nack, PACKET_SIZE, 0);
}

short send_packet(int sockfd, unsigned char *packet) {
    return send(sockfd, packet, PACKET_SIZE, 0);
}
