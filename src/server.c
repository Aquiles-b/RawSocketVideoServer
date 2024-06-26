#include "../header/socket_handler.h"
#include "../header/common_packets.h"

#include <stdio.h>
#include <dirent.h>
#include <string.h>

char **list_files_in_dir(char *dir_path) {
    DIR *dir;
    struct dirent *entry;
    char **movies = malloc(sizeof(char *) * 1024);

    dir = opendir(dir_path);

    if (dir == NULL) {
        return NULL;
    }

    unsigned int i = 0;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG) {
            movies[i] = entry->d_name;
            i++;
        }
    }
    movies[i] = NULL;

    closedir(dir);

    return movies;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Uso: %s <interface>\n", argv[0]);
        return 1;
    }

    int sockfd = open_raw_socket(argv[1]);

    char **movies = list_files_in_dir("movies");

    if (send_NACK(sockfd) == -1) {
        fprintf(stderr, "Erro ao enviar ACK.\n");
        return 1;
    }

    free(movies);
    close(sockfd);

    return 0;
}
