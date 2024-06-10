#include "../header/socket_handler.h"

int main(int argc, char *argv[])
{
    if (argc != 2) {
        fprintf(stderr, "Uso: %s <interface>\n", argv[0]);
        return 1;
    }

    int sockfd = open_raw_socket(argv[1]);

    char data[BUF_SIZE];

    if (recv(sockfd, data, BUF_SIZE, 0) < 0)
        socket_error("Erro ao receber dados");

    printf("%s\n", data);

    close(sockfd);

    return 0;
}
