#include "../header/socket_handler.h"

int main(int argc, char *argv[])
{
    int sockfd = open_raw_socket();
    struct sockaddr_ll saddr = get_interface("lo");
    unsigned int saddr_len = sizeof(saddr);

    char data[BUF_SIZE];

    if (recvfrom(sockfd, data, BUF_SIZE, 0, (struct sockaddr *)&saddr, &saddr_len) < 0)
        error("Erro ao receber dados");

    printf("%s\n", data);

    close(sockfd);

    return 0;
}
