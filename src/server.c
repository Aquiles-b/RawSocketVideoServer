#include "../header/socket_handler.h"

int main(int argc, char *argv[]) {
    int sockfd = open_raw_socket();
    struct sockaddr_ll saddr = get_interface("lo");
    unsigned int saddr_len = sizeof(saddr);

    char *data = "Hello, this is a message from server";

    if (sendto(sockfd, data, strlen(data)+1, 0, (struct sockaddr *)&saddr, saddr_len) < 0)
        error("Erro ao enviar dados");

    close(sockfd);

    return 0;
}
