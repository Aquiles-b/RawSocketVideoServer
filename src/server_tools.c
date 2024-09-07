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
    return 0;
}

/* (Aloca memoria) Adiciona um vetor de nomes de arquivos em movies. */
void list_files_in_dir(char *dir_path, movies_t *movies) {
    DIR *dir;
    struct dirent *entry;

    if (!(dir = opendir(dir_path))) {
        movies->num_movies = 0;
        movies->movies = NULL;
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
    if (movies->num_movies == 0) {
        send_error(sockfd, ERROR_NOT_FOUND);
        return 0;
    }
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

void free_window_packet_list(window_packet_head_t *w_packet_h) {
    window_packet_t *w_packet = w_packet_h->head, *w_packet_aux = NULL;
    while (w_packet) {
        w_packet_aux = w_packet;
        w_packet = w_packet->next_packet;
        free(w_packet_aux->packet);
        free(w_packet_aux);
    }
    free(w_packet_h);
}

int send_window(int sockfd, window_packet_t *w_packet) {
    for (short i = 0; i < WINDOW_SIZE && w_packet; i++) {
        if (!send_packet_in_timeout(sockfd, w_packet->packet)) {
            return 0;
        }
        w_packet = w_packet->next_packet;
    }
    return 1;
}

void free_packets_list_until_node(window_packet_head_t *w_packet_h, 
                                        window_packet_t *w_packet_node) {
    window_packet_t *w_packet = w_packet_h->head, *w_packet_aux = NULL;
    while (w_packet) {
        w_packet_aux = w_packet;
        w_packet = w_packet->next_packet;
        free(w_packet_aux->packet);
        free(w_packet_aux);
        w_packet_h->size--;
        if (w_packet == w_packet_node) {
            break;
        }
    }
    w_packet_h->head = w_packet_node;
}

void change_last_node_packet_code(window_packet_head_t *w_packet_h, unsigned char code) {
    window_packet_t *w_packet = w_packet_h->tail;
    w_packet->packet[2] &= 0xe0;
    w_packet->packet[2] |= code;
    w_packet->packet[PACKET_SIZE - 1] = calc_crc_8(w_packet->packet + 1, PACKET_SIZE - 2);
}

window_packet_head_t *get_next_segment_file(FILE *file, unsigned char *sequence,
                                                        unsigned char *buffer_data) {
    window_packet_head_t *w_packet_head = NULL;
    unsigned long int num_bytes_read = fread(buffer_data, 1, DATA_SIZE, file);
    if (num_bytes_read == 0) {
        return NULL;
    }
    w_packet_head = segment_data_in_window_packets(buffer_data, num_bytes_read, 
                                                   sequence);
    if (num_bytes_read < DATA_SIZE) {
        change_last_node_packet_code(w_packet_head, END_DATA_COD);
        return w_packet_head;
    }

    num_bytes_read = fread(buffer_data, 1, DATA_SIZE, file);
    if (num_bytes_read == 0) {
        change_last_node_packet_code(w_packet_head, END_DATA_COD);
    }
    fseek(file, -num_bytes_read, SEEK_CUR);

    return w_packet_head;
}

/* Junta as duas listas de pacotes em w_packet_h1. A lista w_packet_h2 e liberada. */
void merge_window_packet_lists(window_packet_head_t *w_packet_h1, 
                                        window_packet_head_t *w_packet_h2) {
    w_packet_h1->tail->next_packet = w_packet_h2->head;
    w_packet_h1->tail = w_packet_h2->tail;
    w_packet_h1->size += w_packet_h2->size;
    free(w_packet_h2);
}

window_packet_t *move_window_until_last_sent_packet(window_packet_t *w_packet, 
                                                    unsigned char *client_packet, 
                                                    unsigned long int *send_packet_count) {
    window_packet_t *w_packet_aux = w_packet;
    unsigned char code = get_packet_code(client_packet);
    unsigned char seq = get_packet_seq(client_packet);
    short find = 0;
    if (code == NACK_COD) {
        for (short i = 0; i < WINDOW_SIZE; i++) {
            if (get_packet_seq(w_packet->packet) == seq) {
                find = 1;
                break;
            }
            w_packet = w_packet->next_packet;
            *send_packet_count += 1;
            if (!w_packet) {
                return NULL;
            }
        }
        if (!find) {
            return w_packet_aux;
        }
    }
    else {
        for (short i = 0; i < WINDOW_SIZE && w_packet; i++) {
            w_packet = w_packet->next_packet;
        }
        *send_packet_count += WINDOW_SIZE;
    }
    return w_packet;
}

/* Retorna 1 se todos os pacotes foram enviados com sucesso e 0 se nao foram. */
int send_packets_in_window(int sockfd, FILE *file_to_send) {
    unsigned char *buffer_data;
    unsigned char client_packet[PACKET_SIZE] = {0};
    window_packet_head_t *w_packet_head_aux = NULL, *w_packet_head = NULL;
    window_packet_t *w_packet = NULL;
    unsigned long int send_packet_count = 0, total_packets = 0;
    unsigned char last_packet_sequence = 0;
    int success = 1;

    if (!(buffer_data = malloc(sizeof(unsigned char) * DATA_SIZE))) {
        return 0;
    }
    w_packet_head = get_next_segment_file(file_to_send, &last_packet_sequence, buffer_data);
    w_packet = w_packet_head->head;

    while (w_packet) {
        if (!send_window(sockfd, w_packet)){
            success = 0;
            break;
        }
        if (!recv_ACK_or_NACK(sockfd, client_packet, 10)) {
            success = 0;
            break;
        }

        w_packet = move_window_until_last_sent_packet(w_packet, client_packet,
                                                            &send_packet_count);
        if (!w_packet) {
            break;
        }

        if ((w_packet_head->size - WINDOW_SIZE + 1 - send_packet_count) < WINDOW_SIZE) {
            free_packets_list_until_node(w_packet_head, w_packet);
            w_packet_head_aux = get_next_segment_file(file_to_send, 
                                            &last_packet_sequence, buffer_data);
            if (!w_packet_head_aux) {
                continue;
            }
            merge_window_packet_lists(w_packet_head, w_packet_head_aux);
            total_packets += send_packet_count;
            send_packet_count = 0;
        }
    }
    total_packets += send_packet_count;
    free(buffer_data);
    free_window_packet_list(w_packet_head);

    return success;
}

/* Retorna 1 se o arquivo foi enviado com sucesso e 0 se nao foi. */
int send_file(int sockfd, char *file_name) {
    FILE *file = fopen(file_name, "rb");
    if (file == NULL) {
        return 0;
    }
    int success = send_packets_in_window(sockfd, file);

    fclose(file);

    return success;
}

long int get_movie_index(const unsigned char *packet) {
    return *((long int *) (packet + 3)) - 1;
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
    clear_socket_buffer(sockfd);
    while (1) {
        if ((time(NULL) - start_time) >= 30) {
            break;
        }
        if (send_packet(sockfd, pck_file_desc) == -1) {
            continue;
        }
        if (!recv_packet_in_timeout(sockfd, buffer, 1)) {
            continue;
        }
        client_code = get_packet_code(buffer);
        if (client_code == ACK_COD) {
            free(pck_file_desc);
            return 1;
        }
        else if (client_code == ERROR_COD) {
            break;
        }
    }
    free(pck_file_desc);
    return 0;
}

/* Retorna 1 se for para por um escape simples.
 * 2 se for necessário o byte ir para o próximo pacote.
 * 0 se não for necessário o escape. */
unsigned short need_escape(unsigned char *data, unsigned current_size_packet) {
    unsigned char cbyte = *data;
    if ((cbyte == 0x81) || (cbyte == 0x88)) {
        if (current_size_packet == (PACKET_SIZE - 5)) {
            return 2;
        }
        return 1;
    }
    return 0;
}

int add_packet_on_list(window_packet_head_t *w_packet_h, unsigned char *packet) {
    window_packet_t *w_packet = malloc(sizeof(window_packet_t));
    if (w_packet == NULL) {
        return 0;
    }
    w_packet->packet = packet;
    w_packet->next_packet = NULL;
    if (w_packet_h->size == 0) {
        w_packet_h->head = w_packet;
        w_packet_h->tail = w_packet;
    }
    else {
        w_packet_h->tail->next_packet = w_packet;
        w_packet_h->tail = w_packet;
    }
    w_packet_h->size++;
    return 1;
}

int create_and_add_packet_on_list(unsigned char *data, unsigned short size, 
                                unsigned char *sequence, 
                                window_packet_head_t *w_packet_head) {
    unsigned char *packet = create_packet(data, size, *sequence, DATA_COD);
    *sequence += 1;
    *sequence &= 0x1f;
    if (packet == NULL) {
        free_window_packet_list(w_packet_head);
        return 0;
    }
    if (!add_packet_on_list(w_packet_head, packet)) {
        free_window_packet_list(w_packet_head);
        return 0;
    }
    return 1;
}

/* (Aloca memoria). Monta uma lista window_packet_t de pacotes a partir
 * de um vetor de dados. */
window_packet_head_t *segment_data_in_window_packets(unsigned char *data, 
                                        unsigned long int size, 
                                        unsigned char *sequence) {
    if (size == 0) {
        return NULL;
    }
    unsigned char aux_data_packet[PACKET_SIZE - 4] = {0};
    unsigned short current_size_packet = 0;
    unsigned short escape = 0, break_packet = 0;
    window_packet_head_t *w_packet_head = malloc(sizeof(window_packet_head_t));
    w_packet_head->size = 0;

    while (size != 0) {
        escape = need_escape(data, current_size_packet);
        if (escape == 1) {
            aux_data_packet[current_size_packet] = *data;
            aux_data_packet[current_size_packet+1] = 0xff;
            data++;
            size--;
            current_size_packet += 2;
        } 
        else if (escape == 2) {
            break_packet = 1;
        }
        else {
            aux_data_packet[current_size_packet] = *data;
            data++;
            size--;
            current_size_packet++;
        }

        if ((current_size_packet == (PACKET_SIZE - 4)) || break_packet) {
            if (!create_and_add_packet_on_list(aux_data_packet, current_size_packet, 
                                                sequence, w_packet_head)) {
                return NULL;
            }
            current_size_packet = 0;
            break_packet = 0;
        }
    }
    if (current_size_packet != 0) {
        if (!create_and_add_packet_on_list(aux_data_packet, current_size_packet, 
                                            sequence, w_packet_head)) {
            return NULL;
        }
    }
    return w_packet_head;
}
