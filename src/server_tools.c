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

void free_window_packet_list(window_packet_t *w_packet_init) {
    window_packet_t *w_packet_current = w_packet_init;
    window_packet_t *w_packet_next = NULL;
    while (w_packet_current != NULL) {
        w_packet_next = w_packet_current->next_packet;
        free(w_packet_current->packet);
        free(w_packet_current);
        w_packet_current = w_packet_next;
    }
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
}

window_packet_head_t *get_next_segment_file(FILE *file, unsigned char *sequence,
                                                        unsigned char *buffer_data) {
    window_packet_head_t *w_packet_head = NULL;
    unsigned long int num_bytes_read = fread(buffer_data, 1, DATA_SIZE, file);
    if (num_bytes_read < DATA_SIZE) {
        return segment_data_in_window_packets(buffer_data, num_bytes_read, 
                                                END_DATA_COD, sequence);
    }
    w_packet_head = segment_data_in_window_packets(buffer_data, num_bytes_read, 
                                                DATA_COD, sequence);

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
<<<<<<< HEAD
<<<<<<< HEAD
int send_seg_packets(unsigned char **packets, int sockfd) {
    unsigned char *buffer = malloc(sizeof(unsigned char) * PACKET_SIZE);
    unsigned char seq = 0, seq_ack = 0;
    unsigned long int i = 0;

    while (packets[i]) {
        if (send_packet(sockfd, packets[i]) == -1) {
            continue;
        }
        if (!recv_packet_in_timeout(sockfd, buffer)) {
            continue;
=======
int send_packets_in_window(window_packet_t *w_packet_head, int sockfd) {
=======
int send_packets_in_window(int sockfd, FILE *file_to_send) {
<<<<<<< HEAD:src/server_tools.c
>>>>>>> d42b23a ([FIX] janela deslizante (falta algumas coisas))
    unsigned char *buffer;
=======
    unsigned char *buffer_data;
    unsigned char client_packet[PACKET_SIZE] = {0};
>>>>>>> e39fa6e ([FIX] forma que o cliente manipula os pacotes recebidos):trabalho1/src/server_tools.c
    window_packet_head_t *w_packet_head_aux = NULL, *w_packet_head = NULL;
    window_packet_t *w_packet = NULL;
    unsigned long int send_packet_count = 0;
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
<<<<<<< HEAD:src/server_tools.c
<<<<<<< HEAD
        if (!recv_packet_in_timeout(sockfd, buffer)) {
            success = 0;
            break;
        }

        code = get_packet_code(buffer);
        if (code != ACK_COD && code != NACK_COD) {
            success = 0;
            break;
>>>>>>> 66e7900 ([FIX] janela deslizante)
        }
        seq = get_packet_seq(packets[i]);

<<<<<<< HEAD
        if (get_packet_code(buffer) == ACK_COD) {
            seq_ack = get_packet_seq(buffer);
            if (seq == seq_ack) {
                i++;
            }
        } 
    }
    return 1;
=======
        w_packet_aux = w_packet;
        while (w_packet_aux) {
            if (get_packet_seq(w_packet_aux->packet) == packet_seq) {
=======
        while (1) {
            clear_socket_buffer(sockfd);
            if (!recv_packet_in_timeout(sockfd, buffer)) {
                success = 0;
>>>>>>> d42b23a ([FIX] janela deslizante (falta algumas coisas))
                break;
            }
            code = get_packet_code(buffer);
            if (code != ACK_COD && code != NACK_COD) {
=======
        clear_socket_buffer(sockfd);
<<<<<<< HEAD:src/server_tools.c
        while (1) {
            if (!recv_packet_in_timeout(sockfd, client_packet, 0)) {
>>>>>>> e39fa6e ([FIX] forma que o cliente manipula os pacotes recebidos):trabalho1/src/server_tools.c
                continue;
            }
            code = get_packet_code(client_packet);
            if ((code == ACK_COD) || (code == NACK_COD)) {
                break;
            }
=======
        if (!recv_ACK_or_NACK(sockfd, client_packet)) {
            continue;
>>>>>>> 52495da ([FIX] Ultimo pacote a ser enviado com tamanho maior):trabalho1/src/server_tools.c
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
            send_packet_count = 0;
        }
    }
    free(buffer_data);

    return success;
>>>>>>> 66e7900 ([FIX] janela deslizante)
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

/* (Aloca memoria). Monta uma lista window_packet_t de pacotes a partir
 * de um vetor de dados. */
window_packet_head_t *segment_data_in_window_packets(unsigned char *data, 
                                        const unsigned long int size, 
                                        unsigned char last_packet_code,
                                        unsigned char *sequence) {
    if (size == 0) {
        return NULL;
    }
    unsigned short max_size_data = PACKET_SIZE - 4;
    unsigned long int num_packets = size / max_size_data;
    unsigned short last_packet_size = size % max_size_data;

    if (last_packet_size != 0) {
        ++num_packets;
    } 
    else {
        last_packet_size = max_size_data;
    }

    window_packet_head_t *w_packet_head = malloc(sizeof(window_packet_head_t));
    window_packet_t *w_packet_init = malloc(sizeof(window_packet_t));
    window_packet_t *w_packet_current = NULL, *w_packet_last = NULL;
    unsigned long int i = 0;

    w_packet_current = w_packet_init;
    w_packet_last = w_packet_init;

    for (; i < num_packets - 1; i++) {
        w_packet_last->next_packet = w_packet_current;
        w_packet_current->packet = create_packet(data, max_size_data, *sequence,
                                                                    DATA_COD);
        w_packet_last = w_packet_current;
        w_packet_current = malloc(sizeof(window_packet_t));

        data += max_size_data;
        *sequence += 1;
        *sequence &= 0x1f;
    }
    w_packet_last->next_packet = w_packet_current;
    w_packet_current->packet = create_packet(data, last_packet_size, *sequence,
                                                            last_packet_code);
    w_packet_current->next_packet = NULL;

    w_packet_head->head = w_packet_init;
    w_packet_head->tail = w_packet_current;
    w_packet_head->size = num_packets;

    *sequence += 1;
    *sequence &= 0x1f;

    return w_packet_head;
}
