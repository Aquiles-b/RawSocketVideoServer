#include "../header/server_tools.h"

unsigned int get_num_files_dir(DIR *dir) {
    struct dirent *entry;
    unsigned int num_files = 0;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG) {
            num_files++;
        }
    }
    rewinddir(dir);

    return num_files;
}

unsigned long int get_file_size(const char *file_path) {
    struct stat st;

    if (stat(file_path, &st) == 0) {
        return st.st_size;
    } 
    else {
        return 0;
    }
}

/* (Aloca memoria) Retorna um vetor de nomes de arquivos no diretorio.
 * O ultimo elemento da lista eh NULL. */
char **list_files_in_dir(char *dir_path) {
    DIR *dir;
    struct dirent *entry;

    if (!(dir = opendir(dir_path))) {
        return NULL;
    }

    unsigned int i = 0;
    unsigned int num_files = get_num_files_dir(dir);

    char **movies = malloc(sizeof(char *) * (num_files + 1));

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
    return 1;
}

/* Retorna 1 se todos os pacotes foram enviados com sucesso e 0 se nao foram. */
int send_seg_packets(unsigned char **packets, int sockfd) {
    unsigned char *buffer = malloc(sizeof(unsigned char) * PACKET_SIZE);
    unsigned char seq = 0, seq_ack = 0;
    unsigned long int i = 0;
    short n = 0;

    while (packets[i]) {
        if (send_packet(sockfd, packets[i]) == -1) {
            continue;
        }

        n = recv(sockfd, buffer, PACKET_SIZE, 0);

        if (n == -1) {
            continue;
        }

        n = validate_packet(buffer, n);

        seq = get_packet_seq(packets[i]);

        if (n && (get_packet_code(buffer) == ACK_COD)) {
            seq_ack = get_packet_seq(buffer);
            if (seq == seq_ack) {
                i++;
            }
        } 
    }
    return 1;
}

/* Retorna 1 se o arquivo foi enviado com sucesso e 0 se nao foi. */
int send_file(int sockfd, char *file_name) {
    FILE *file = fopen(file_name, "rb");
    if (file == NULL) {
        return 0;
    }

    unsigned char *buffer_data = malloc(sizeof(unsigned char) * DATA_SIZE);
    unsigned long int file_size = get_file_size(file_name);
    if (file_size == 0) {
        return 0;
    }
    unsigned long int num_bytes_read = 0;
    unsigned long int num_segs = file_size / DATA_SIZE;

    if (file_size % DATA_SIZE != 0) {
        num_segs++;
    }

    while(num_segs > 1) {
        num_bytes_read = fread(buffer_data, 1, DATA_SIZE, file);
        unsigned char **packets = segment_data_in_packets(buffer_data, num_bytes_read, DATA_COD);
        send_seg_packets(packets, sockfd);
        free_packets(&packets);
        num_segs--;
    }
    num_bytes_read = fread(buffer_data, 1, DATA_SIZE, file);
    unsigned char **packets = segment_data_in_packets(buffer_data, num_bytes_read, END_DATA_COD);
    send_seg_packets(packets, sockfd);
    free_packets(&packets);

    fclose(file);
    free(buffer_data);

    return 1;
}
