#include "../header/socket_handler.h"
#include "../header/basic_for_packets.h"
#include "../header/common_packets.h"

void recv_file(int sockfd) {
    FILE *file = fopen("received_file.mp4", "wb");
    if (file == NULL) {
        fprintf(stderr, "Erro ao abrir o arquivo\n");
        return;
    }

    unsigned char buffer[PACKET_SIZE] = {0};
    short n = 0;
    unsigned char last_seq = -1, seq = 0, cod = 0;
    unsigned long int num_packets = 0;
    unsigned short tam_data = 0;

    send_ACK(sockfd, 0);
    clear_socket_buffer(sockfd);

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

        seq = ((buffer[1] & 0x03) << 3) | (buffer[2] >> 5);
        cod = buffer[2] & 0x1f;

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

        tam_data = (buffer[1] >> 2);

        fwrite(buffer + 3, 1, tam_data, file);
        num_packets++;

        if (cod == END_DATA_COD) {
            break;
        }
    }
    printf("Recebidos %ld pacotes\n", num_packets);

}

int main(int argc, char *argv[])
{
    if (argc != 2) {
        fprintf(stderr, "Uso: %s <interface>\n", argv[0]);
        return 1;
    }

    int sockfd = open_raw_socket(argv[1]);

    recv_file(sockfd);

    close(sockfd);

    return 0;
}
