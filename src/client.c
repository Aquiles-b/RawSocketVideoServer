#include "../header/socket_handler.h"
#include "../header/client_tools.h"

int main(int argc, char *argv[])
{
    if (argc != 2) {
        fprintf(stderr, "Uso: %s <interface>\n", argv[0]);
        return 1;
    }

    int sockfd = open_raw_socket(argv[1]);

    recv_file(sockfd, "received_file.mp4");

    close(sockfd);

    return 0;
}
