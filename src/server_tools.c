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

/* (Aloca memoria) Adiciona um vetor de nomes de arquivos em movies. */
void list_files_in_dir(char *dir_path, movies_t *movies) {
    DIR *dir;
    struct dirent *entry;

    if (!(dir = opendir(dir_path))) {
        return;
    }

    unsigned int i = 0;
    unsigned long int num_files = get_num_files_dir(dir);
    movies->num_movies = num_files;

    movies->movies = malloc(sizeof(char *) * num_files);

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG) {
            movies->movies[i] = malloc(sizeof(char) * (strlen(entry->d_name) + 1));
            strcpy(movies->movies[i], entry->d_name);
            i++;
        }
    }
    closedir(dir);
}

int send_movies_list(int sockfd, movies_t *movies) {
    unsigned char *buffer = malloc(sizeof(unsigned char) * PACKET_SIZE);
    unsigned char **packets_movies = malloc(sizeof(unsigned char *) * movies->num_movies);
    unsigned char seq = 0;
    unsigned int i = 0, success = 1;

    for (; i < movies->num_movies - 1; i++) {
       packets_movies[i] = create_packet((unsigned char *) movies->movies[i], 
                                              strlen(movies->movies[i]) + 1,
                                              seq , SHOW_IN_SCREEN_COD);
        seq++;
    }
    packets_movies[i] = create_packet((unsigned char *) movies->movies[i], 
                                          strlen(movies->movies[i]) + 1,
                                          seq , END_DATA_COD);

    for (i = 0; i < movies->num_movies; i++) {
        if (!send_packet_with_confirm(sockfd, packets_movies[i], buffer)) {
            success = 0;
            break;
        }
    }

    for (i = 0; i < movies->num_movies; i++) {
        free(packets_movies[i]);
    }

    free(packets_movies);
    free(buffer);

    return success;
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

        if (!recv_packet_in_timeout(sockfd, buffer)) {
            continue;
        }

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

long int get_movie_index(const unsigned char *packet) {
    long int idx = 0;
    memcpy(&idx, packet + 3, sizeof(long int));
    return idx;
}

/* (Aloca memoria) Retorna NULL em caso de falha. */
unsigned char *create_packet_file_desc(int sockfd, char *file_name){
    unsigned long int file_size = get_file_size(file_name);
    struct stat attr;
    if (stat(file_name, &attr) < 0) {
        return NULL;
    }
    time_t mod_time = attr.st_mtime;
    struct tm *timeinfo = localtime(&mod_time);
    unsigned short day, month, year;
    day = timeinfo->tm_mday;
    month = timeinfo->tm_mon + 1;
    year = timeinfo->tm_year + 1900;

    unsigned char data[12] = {0};
    memcpy(data, &file_size, sizeof(unsigned long int));
    data[8] = day;
    data[9] = month;
    data[10] = year >> 8;
    data[11] = year;

    return create_packet(data, 12, 0, FILE_DESC_COD);
}

/* Retorna 1 se o descritor do arquivo foi enviado com sucesso e 0 se nao foi. */
int send_file_desc(int sockfd, char *file_name) {
    unsigned char *pck_file_desc = create_packet_file_desc(sockfd, file_name);
    if (!pck_file_desc) {
        send_error(sockfd, ERROR_ACCESS_DENIED);
        return 0;
    }
    time_t start_time = time(NULL);
    unsigned char buffer[PACKET_SIZE] = {0};
    unsigned char client_code = 0;
    while (1) {
        if ((time(NULL) - start_time) >= TIMEOUT*30) {
            break;
        }
        if (send_packet(sockfd, pck_file_desc) == -1) {
            continue;
        }
        if (!recv_packet_in_timeout(sockfd, buffer)) {
            continue;
        }
        client_code = get_packet_code(buffer);
        if (client_code == ACK_COD) {
            free(pck_file_desc);
            return 1;
        }
        else if (client_code == ERROR_DISK_FULL) {
            break;
        }
    }
    free(pck_file_desc);
    return 0;
}

