#include "../header/socket_handler.h"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Uso: %s <interface>\n", argv[0]);
        return 1;
    }

    int sockfd = open_raw_socket(argv[1]);

    char *data = "Hello, this is a message from server";

    if (send(sockfd, data, strlen(data)+1, 0) < 0)
        socket_error("Erro ao enviar dados");

    close(sockfd);

    return 0;
}
