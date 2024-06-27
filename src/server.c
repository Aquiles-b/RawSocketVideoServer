#include "../header/socket_handler.h"
#include "../header/common_packets.h"
#include "../header/basic_for_packets.h"

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

int send_movies_list(int sockfd, char **movies) {
    return 0;
}

void send_file(int sockfd, char *file_name) {
    FILE *file = fopen(file_name, "rb");
    if (file == NULL) {
        fprintf(stderr, "Erro ao abrir o arquivo %s\n", file_name);
        return;
    }

    fseek(file, 0, SEEK_END);
    unsigned long int file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    unsigned char *data = malloc(file_size);
    fread(data, 1, file_size, file);

    unsigned char **packets = segment_data_in_packets(data, file_size);

    unsigned char *buffer = malloc(sizeof(unsigned char) * 67);

    unsigned long int i = 0;
    while (1) {
        if (send_packet(sockfd, packets[i]) == -1) {
            fprintf(stderr, "Erro ao enviar pacote %lu\n", i);
            exit(1);
        }
        int n = 0;
        if (packets[i+1] == NULL) {
            break;
        }

        if ((n = recv(sockfd, buffer, 67, 0)) == -1) {
            fprintf(stderr, "Erro ao receber pacote de confirmação\n");
            exit(1);
        }

        // Logica de comunicação aqui

        n = validate_packet(buffer, n);
        i++;
    }

    free(data);
    free_packets(&packets);
    fclose(file);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Uso: %s <interface>\n", argv[0]);
        return 1;
    }

    int sockfd = open_raw_socket(argv[1]);

    send_file(sockfd, "./movies/Untitled.mp4");

    close(sockfd);

    return 0;
}
