#include "../header/socket_handler.h"

void error(const char *msg) {
    perror(msg);
    exit(1);
}

int open_raw_socket() {
    int sockfd;
    sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (sockfd < 0)
        error("Erro ao abrir socket");

    return sockfd;
}

struct sockaddr_ll get_interface(const char *interface) {
    struct sockaddr_ll saddr;

    /* Preenchendo a estrutura sockaddr_ll para
     * enviar o pacote para a interface */
    memset(&saddr, 0, sizeof(saddr));
    saddr.sll_family = AF_PACKET;
    saddr.sll_protocol = htons(ETH_P_ALL);
    saddr.sll_ifindex = if_nametoindex(interface);

    return saddr;
}
