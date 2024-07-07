#include "../header/socket_handler.h"
#include "../header/server_tools.h"

int try_send_file(int sockfd, movies_t *movies, unsigned char *packet_client) {
    unsigned long int file_index = get_file_index(packet_client);

    if (file_index >= movies->num_movies || file_index < 0) {
        send_error(sockfd, ERROR_NOT_FOUND);
        return 0;
    }
    if(!send_file_desc(sockfd, movies->movies[file_index])) {
        return 0;
    }

    return send_file(sockfd, movies->movies[file_index]);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Uso: %s <interface>\n", argv[0]);
        return 1;
    }
    unsigned char *packet_client = malloc(sizeof(unsigned char) * PACKET_SIZE);
    unsigned char code = 0;
    movies_t movies_list;
    list_files_in_dir("movies", &movies_list);

    int sockfd = open_raw_socket(argv[1]);
    while (1) {
        if (!recv_packet_in_timeout(sockfd, packet_client)) {
            continue;
        }
        send_ACK(sockfd, 0);

        code = get_packet_code(packet_client);

        switch (code) {
            case LIST_FILES_COD:
                send_movies_list(sockfd, &movies_list);
                break; 
            case DOWNLOAD_FILE_COD:
                try_send_file(sockfd, &movies_list, packet_client);
                break;
        }
    }

    close(sockfd);

    return 0;
}
