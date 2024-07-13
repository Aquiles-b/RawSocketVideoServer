#include "../header/basic_for_packets.h"

unsigned char calc_crc_8(const unsigned char *data, const short size) {
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
int validate_crc_8(const unsigned char *data, const short size) {
    unsigned char crc = calc_crc_8(data + 1, size-2);
    return crc == data[size-1];
}

/* Retorna 1 se for valido e 0 se nao for. */
int validate_packet(const unsigned char *data, const short size) {
    if ((size < PACKET_SIZE) || (data[0] != INIT_MARKER)) {
        return 0;
    }
    if (validate_crc_8(data, size)) {
        return 1;
    }
    return 0;
}

/* (Aloca memoria). Retorna NULL em caso de falha */
unsigned char *create_packet(const unsigned char *data, unsigned short size_data,
                        unsigned char seq, unsigned char code) {
    unsigned char *packet = calloc(PACKET_SIZE, sizeof(unsigned char));
    if (packet == NULL) {
        return NULL;
    }
    packet[0] = INIT_MARKER;
    packet[1] = (size_data << 2) | (seq >> 3);
    packet[2] = (seq << 5) | code;
    if (data != NULL) {
        memcpy(packet + 3, data, size_data);
    }
    packet[PACKET_SIZE - 1] = calc_crc_8(packet + 1, PACKET_SIZE - 2);

    return packet;
}

/* (Aloca memoria). A ultima posicao do vetor eh NULL. */
unsigned char **segment_data_in_packets(unsigned char *data, 
                                        const unsigned long int size, 
                                        unsigned char last_packet_code,
                                        unsigned char *sequence) {
    if (size == 0) {
        return NULL;
    }

    unsigned short max_size_data = PACKET_SIZE - 4;
    unsigned long int num_packets = size / max_size_data;
    unsigned short last_packet_size = size % max_size_data;

    if (last_packet_size != 0) {
        ++num_packets;
    } 
    else {
        last_packet_size = max_size_data;
    }

    unsigned char **packets = malloc(sizeof(unsigned char *) * (num_packets + 1));
    unsigned long int i = 0;

    for (; i < num_packets - 1; i++) {
        packets[i] = create_packet(data, max_size_data, *sequence, DATA_COD);
        data += max_size_data;
        *sequence += 1;
        *sequence &= 0x1f;
    }
    packets[i] = create_packet(data, last_packet_size, *sequence, last_packet_code);
    *sequence += 1;
    *sequence &= 0x1f;

    packets[num_packets] = NULL;

    return packets;
}

void free_packets(unsigned char ***packets) {
    for (unsigned long int i = 0; (*packets)[i] != NULL; i++) {
        free((*packets)[i]);
    }
    free(*packets);
    *packets = NULL;
}

void clear_socket_buffer(int sockfd) {
    unsigned char buffer[PACKET_SIZE] = {0};
    while (recv(sockfd, buffer, PACKET_SIZE, MSG_DONTWAIT) != -1);
}

unsigned char get_packet_code(const unsigned char *packet) {
    return packet[2] & 0x1f;
}

unsigned char get_packet_seq(const unsigned char *packet) {
    return ((packet[1] & 0x03) << 3) | (packet[2] >> 5);
}

unsigned short get_packet_data_size(const unsigned char *packet) {
    return packet[1] >> 2;
}
