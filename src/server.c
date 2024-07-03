#include "../header/socket_handler.h"
#include "../header/server_tools.h"

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Uso: %s <interface> <file>\n", argv[0]);
        return 1;
    }

    int sockfd = open_raw_socket(argv[1]);

    send_file(sockfd, argv[2]);

    close(sockfd);

    return 0;
}
