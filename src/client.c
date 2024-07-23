#include "../header/socket_handler.h"
#include "../header/client_tools.h"

int try_get_movie(int sockfd, unsigned char *packet_server, long int opt) {
    unsigned char data[PACKET_SIZE] = {0};
    memcpy(data, &opt, sizeof(long int));
    unsigned char *req_file_desc_packet = create_packet(data, sizeof(long int),
                                                        0, DOWNLOAD_FILE_COD);

    if (!send_packet_with_confirm(sockfd, req_file_desc_packet, packet_server)){
        printf("!! Sem resposta do server\n\n");
        return 0;
    }
    free(req_file_desc_packet);

    if (!handle_recv_file_desc_packet(sockfd, packet_server)) {
        return 0;
    }

    show_movie_date_size_packet(packet_server);

    opt = get_user_input("Prosseguir com o download? (1-Sim, 2-Nao): ");
    printf("\n");
    if (opt != 1) {
        send_error(sockfd, ERROR_DISK_FULL);
        return 0;
    }
    send_ACK(sockfd, 0);

    unsigned long int size = *((unsigned long int *) (packet_server + 3));
    if (!recv_file(sockfd, "stream_movie.mp4", size)) {
        printf("!! Erro ao baixar arquivo\n\n");
        return 0;
    }

    return 1;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Uso: %s <interface>\n", argv[0]);
        return 1;
    }

    unsigned char *packet_server = malloc(sizeof(unsigned char) * PACKET_SIZE);
    long int option = 0;

    int sockfd = open_raw_socket(argv[1]);
    while (1) {
        option = get_user_input("1- Mostrar filmes\n2- Baixar arquivo\n3- Sair\n: ");
        switch (option) {
            case 1:
                view_movies_list(sockfd, packet_server);
                break;
            case 2:
                option = get_user_input("Digite o ID do filme: ");
                printf("\n");
                if (try_get_movie(sockfd, packet_server, option)) {
                    printf("Concluido!\n\n");
                    system("celluloid stream_movie.mp4 > /etc/null 2>&1");
                }
                break;
        }
        clear_socket_buffer(sockfd);
    }

    free(packet_server);
    close(sockfd);

    return 0;
}
