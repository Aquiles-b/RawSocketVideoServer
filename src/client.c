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
    int (*send)(int) = send_ACK;
    int i = 0;

    send_ACK(sockfd);

    while (1) {
        if ((n = recv(sockfd, buffer, PACKET_MAX_SIZE, 0) == -1)) {
            printf("Timeout em %d\n", i);
            send(sockfd);
            continue;
        }

        n = validate_packet(buffer, n);

        if (!n) {
            printf("Pacote %d invalido. Mandando NACK\n", i);
            send_NACK(sockfd);
            send = send_NACK;
            continue;
        }

        fwrite(buffer + 3, 1, n - 4, file);
        printf("Recebido\n");

        if ( (buffer[2] & 0x1f) == END_DATA_COD) {
            break;
        }
        i++;
        send_ACK(sockfd);
        send = send_ACK;
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
