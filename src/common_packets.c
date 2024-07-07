#include "../header/common_packets.h"
#include <time.h>

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

int send_packet(int sockfd, unsigned char *packet) {
    return send(sockfd, packet, PACKET_SIZE, 0);
}

/* Retorna 1 em caso de sucesso e 0 em falha. */
int recv_packet_in_timeout(int sockfd, unsigned char *buffer) {
    short num_bytes_read = 0;
    time_t start_time = time(NULL);
    while (1) {
        if ((time(NULL) - start_time) >= TIMEOUT) {
            return 0;
        }
        if ((num_bytes_read = recv(sockfd, buffer, PACKET_SIZE, 0)) <= 0) {
            continue;
        }
        if (!validate_packet(buffer, num_bytes_read)) {
            send_NACK(sockfd, 0);
            continue;
        }
        return 1;
    }
}

/* Retorna 1 em caso de sucesso e 0 em falha. */
int send_packet_in_timeout(int sockfd, unsigned char *packet, unsigned char *buffer) {
    unsigned char seq = get_packet_seq(packet);
    time_t start_time = time(NULL);
    while (1) {
        if ((time(NULL) - start_time) >= TIMEOUT) {
            return 0;
        }
        if (send_packet(sockfd, packet) == -1) {
            continue;
        }
        if (!recv_packet_in_timeout(sockfd, buffer)) {
            continue;
        }
        if (get_packet_code(buffer) == ACK_COD) {
            if (seq == get_packet_seq(buffer)) {
                return 1;
            }
        } 
    }
}

int send_error(int sockfd, unsigned char error_code) {
    unsigned char error_packet[PACKET_SIZE] = {0};
    error_packet[0] = INIT_MARKER;
    error_packet[1] = 0;
    error_packet[2] = ERROR_COD;
    error_packet[3] = error_code;
    error_packet[PACKET_SIZE - 1] = calc_crc_8(error_packet + 1, PACKET_SIZE - 2);
    return send(sockfd, error_packet, PACKET_SIZE, 0);
}
