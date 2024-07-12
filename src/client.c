#include "../header/socket_handler.h"
#include "../header/client_tools.h"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Uso: %s <interface>\n", argv[0]);
        return 1;
    }

    unsigned char *packet_server = malloc(sizeof(unsigned char) * PACKET_SIZE);
    unsigned int option = 0;

    int sockfd = open_raw_socket(argv[1]);
    while (1) {
        option = get_user_input("1- Mostrar filmes\n2- Baixar arquivo\n3- Sair\n:");
        switch (option) {
            case 1:
                view_movies_list(sockfd, packet_server);
                break;
            case 2:
                get_user_input("Digite o ID do filme\n: ");
                /* get_movie(sockfd, packet_server); */
                break;
        }
    }

    free(packet_server);
    close(sockfd);

    return 0;
}
