#include "../header/socket_handler.h"
#include "../header/server_tools.h"

int try_send_file(int sockfd, movies_t *movies, unsigned char *packet) {
    unsigned long int file_index = get_file_index(packet);

    if (file_index >= movies->num_movies || file_index < 0) {
        send_error(sockfd, ERROR_NOT_FOUND);
        return 0;
    }
    send_file_desc(sockfd, movies->movies[file_index]);

    send_file(sockfd, movies->movies[file_index]);

    return 1;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Uso: %s <interface> <file>\n", argv[0]);
        return 1;
    }
    unsigned char *buffer = malloc(sizeof(unsigned char) * PACKET_SIZE);
    short num_bytes_read = 0;
    unsigned char code = 0;
    unsigned long int num_movies = 0, file_index = 0;
    movies_t movies_list;
    list_files_in_dir("movies", &movies_list);

    int sockfd = open_raw_socket(argv[1]);
    while (1) {
        if ((num_bytes_read = recv(sockfd, buffer, PACKET_SIZE, 0)) == -1) {
            continue;
        }
        if (!validate_packet(buffer, num_bytes_read)) {
            send_NACK(sockfd, 0);
            continue;
        }
        send_ACK(sockfd, 0);

        code = get_packet_code(buffer);

        switch (code) {
            case LIST_FILES_COD:
                send_movies_list(sockfd, &movies_list);
                break; 
            case DOWNLOAD_FILE_COD:
                try_send_file(sockfd, &movies_list, buffer);
                break;
        }
    }

    close(sockfd);

    return 0;
}
