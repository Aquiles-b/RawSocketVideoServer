#include "../header/socket_handler.h"
#include "../header/server_tools.h"

int try_send_movie(int sockfd, movies_t *movies, unsigned char *packet_client) {
    long int file_index = get_movie_index(packet_client);

    if (file_index >= movies->num_movies || file_index < 0) {
        send_error(sockfd, ERROR_NOT_FOUND);
        return 0;
    }
    char file_name[128] = "movies/";
    strcat(file_name, movies->movies[file_index]);

    if (!send_file_desc(sockfd, file_name)) {
        return 0;
    }

    return send_file(sockfd, file_name);
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
        if (!recv_packet_in_timeout(sockfd, packet_client, 0)) {
            continue;
        }

        code = get_packet_code(packet_client);
        switch (code) {
            case LIST_FILES_COD:
                send_ACK(sockfd, 0);
                send_movies_list(sockfd, &movies_list);
                break; 
            case DOWNLOAD_FILE_COD:
                send_ACK(sockfd, 0);
                try_send_movie(sockfd, &movies_list, packet_client);
                break;
        }
        clear_socket_buffer(sockfd);
    }

    close(sockfd);
    free(packet_client);

    return 0;
}
