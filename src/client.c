#include "../header/socket_handler.h"
#include "../header/basic_for_packets.h"
#include "../header/common_packets.h"

void recv_file(int sockfd) {
    unsigned char buffer[PACKET_MAX_SIZE] = {0};
    FILE *file = fopen("received_file.mp4", "wb");
    if (file == NULL) {
        fprintf(stderr, "Erro ao abrir o arquivo\n");
        return;
    }

    short n = 0;
    unsigned char last_seq = 0, seq = 0;

    // Primeiro pacote
    while (1) {
        if ((n = recv(sockfd, buffer, PACKET_MAX_SIZE, 0)) == -1) {
            continue;
        }

        n = validate_packet(buffer, n);
        if (!n) {
            send_NACK(sockfd);
            continue;
        }

        seq = ((buffer[1] & 0x03) << 3) | (buffer[2] >> 5);

        fwrite(buffer + 3, 1, n - 4, file);

        send_ACK(sockfd);
        if ((buffer[2] & 0x1f) == END_DATA_COD) {
            fclose(file);
            return;
        }
        break;
    }
    last_seq = seq;

    // Restante
    while (1) {
        n = recv(sockfd, buffer, PACKET_MAX_SIZE, 0);
        if (n == -1) {
            continue;
        }

        n = validate_packet(buffer, n);
        if (!n) {
            send_NACK(sockfd);
            continue;
        }

        seq = ((buffer[1] & 0x03) << 3) | (buffer[2] >> 5);

        if (seq == last_seq) {
            send_ACK(sockfd);
            continue;
        }

        last_seq = seq;

        fwrite(buffer + 3, 1, n - 4, file);

        send_ACK(sockfd);
        if ( (buffer[2] & 0x1f) == END_DATA_COD) {
            break;
        }
    }

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
