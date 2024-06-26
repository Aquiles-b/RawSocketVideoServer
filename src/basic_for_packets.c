#include "../header/basic_for_packets.h"

unsigned char calc_crc_8(unsigned char *data, const short size) {
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
short validate_crc_8(unsigned char *data, const short size) {
    unsigned char crc = calc_crc_8(data + 1, size-2);
    return crc == data[size-1];
}

/* Retorna o tamanho real do pacote se for valido e 0 se nao for. */
short validate_packet(unsigned char *data, const short size) {
    if ((size < PACKET_MIN_SIZE) || (data[0] != 0x7e)) {
        return 0;
    }
    short tam_packet = 0;
    tam_packet = data[1] >> 2;
    tam_packet += 4;

    if (validate_crc_8(data, tam_packet)) {
        return tam_packet;
    }
    return 0;
}
