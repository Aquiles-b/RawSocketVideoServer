#include "../header/client_tools.h"

int recv_file(int sockfd, char *filename) {
    FILE *file = fopen(filename, "wb");
    if (file == NULL) {
        fprintf(stderr, "Erro ao abrir o arquivo\n");
        return 0;
    }

    unsigned char buffer[PACKET_SIZE] = {0};
    short n = 0;
    unsigned char last_seq = -1, seq = 0, cod = 0;
    unsigned long int num_packets = 0;
    unsigned short tam_data = 0;

    while (1) {
        n = recv(sockfd, buffer, PACKET_SIZE, 0);

        if (n == -1) {
            continue;
        }

        n = validate_packet(buffer, n);
        if (!n) {
            send_NACK(sockfd, seq);
            continue;
        }

        seq = get_packet_seq(buffer);
        cod = get_packet_code(buffer);

        if ((cod != DATA_COD) && (cod != END_DATA_COD)) {
            send_NACK(sockfd, seq);
            continue;
        }

        if (seq == last_seq) {
            send_ACK(sockfd, seq);
            continue;
        }
        last_seq = seq;

        send_ACK(sockfd, seq);

        tam_data = get_packet_data_size(buffer);

        fwrite(buffer + 3, 1, tam_data, file);
        num_packets++;

        if (cod == END_DATA_COD) {
            break;
        }
    }
    printf("Recebidos %ld pacotes\n", num_packets);

    return 1;
}
