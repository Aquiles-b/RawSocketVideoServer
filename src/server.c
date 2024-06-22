#include "../header/socket_handler.h"
#include "../header/common_packets.h"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Uso: %s <interface>\n", argv[0]);
        return 1;
    }

    int sockfd = open_raw_socket(argv[1]);

    if (send_ACK(sockfd) < 0)
        socket_error("Erro ao enviar ACK");

    close(sockfd);

    return 0;
}
