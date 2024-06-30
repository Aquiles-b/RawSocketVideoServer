#include "../header/socket_handler.h"

void socket_error(const char *msg) {
    perror(msg);
    exit(1);
}

int open_raw_socket(const char *interface) {
    int sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (sockfd < 0)
        socket_error("Erro ao abrir socket");

    int ifindex = if_nametoindex(interface);
 
    struct sockaddr_ll endereco = {0};
    endereco.sll_family = AF_PACKET;
    endereco.sll_protocol = htons(ETH_P_ALL);
    endereco.sll_ifindex = ifindex;

    if (bind(sockfd, (struct sockaddr*) &endereco, sizeof(endereco)) == -1)
        socket_error("Erro ao fazer bind");
 
    struct packet_mreq mr = {0};
    mr.mr_ifindex = ifindex;
    mr.mr_type = PACKET_MR_PROMISC;

    struct timeval timeout = { .tv_sec = 0, .tv_usec = TIME_OUT_MSECONDS};

    // Não joga fora o que identifica como lixo: Modo promíscuo
    if (setsockopt(sockfd, SOL_PACKET, PACKET_ADD_MEMBERSHIP, &mr, sizeof(mr)) == -1)
        socket_error("Erro ao fazer setsockopt para modo promíscuo");

    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *) &timeout, sizeof(timeout)) == -1)
        socket_error("Erro ao fazer setsockopt para timeout");

    int socket_buffer_size = 67;
    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, &socket_buffer_size, sizeof(socket_buffer_size)) == -1)
        socket_error("Erro ao fazer setsockopt para buffer do socket");

    return sockfd;
}
