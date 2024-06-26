#include "../header/socket_handler.h"
#include "../header/basic_for_packets.h"

int main(int argc, char *argv[])
{
    if (argc != 2) {
        fprintf(stderr, "Uso: %s <interface>\n", argv[0]);
        return 1;
    }

    int sockfd = open_raw_socket(argv[1]);

    unsigned char buf[PACKET_MAX_SIZE] = {0};

    int received = 0;

    while (!received) {
        u_short num_bytes = recv(sockfd, buf, PACKET_MAX_SIZE, 0);
        if (num_bytes < 0) {
            socket_error("recvfrom");
        }
        if (validate_packet(buf, num_bytes)) {
            printf("Pacote válido\n");
            received = 1;
        } 
        else {
            printf("Pacote inválido\n");
        }
    }

    printf("ACK recebido");

    close(sockfd);

    return 0;
}
