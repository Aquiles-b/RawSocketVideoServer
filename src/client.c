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

    while (1) {
        n = recv(sockfd, buffer, PACKET_MAX_SIZE, 0);

        n = validate_packet(buffer, n);

        // Logica de comunicação aqui

        fwrite(buffer + 3, 1, n - 4, file);

        if ( (buffer[2] & 0x1f) == END_DATA_COD) {
            break;
        }
        send_ACK(sockfd);
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
