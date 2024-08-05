#include "../header/client_tools.h"

long int get_user_input(char *text) {
    char option[64];
    unsigned int option_int = 0;

    while (1) {
        printf("%s", text);
        fgets(option, sizeof(option), stdin);

        option[strcspn(option, "\n")] = '\0';
        option_int = atoi(option);

        if (option_int) {
            return option_int;
        }
        printf("Opcao invalida\n");
    }
}

void print_movie(unsigned char *packet, unsigned int id_movie) {
    unsigned char *data = packet + 3;
    short tam = get_packet_data_size(packet);

    if (id_movie % 2 == 0 || tam > 30) {
        printf("%d- %-30s\n", id_movie, data);
    }
    else {
        printf("%d- %-30s ", id_movie, data);
    }
}

int recv_and_print_movie_names_packets(int sockfd, unsigned char *packet_server) {
    unsigned char code = 0;
    unsigned char packet_seq = 0, current_seq = 0;
    unsigned int success = 1, id_movie = 1;

    printf("-Filmes-------------------------------------\n");
    while (1) {
        if (!recv_packet_in_timeout(sockfd, packet_server, 1)) {
            success = 0;
            break;
        }
        code = get_packet_code(packet_server);
        if (code == ERROR_COD) {
            if (get_error_type(packet_server) == ERROR_NOT_FOUND) {
                printf("Nenhum filme encontrado\n");
            }
            success = 0;
            break;
        }
        if ((code != SHOW_IN_SCREEN_COD) && (code != END_DATA_COD)) {
            send_NACK(sockfd, 0);
            continue;
        }
        packet_seq = get_packet_seq(packet_server);
        if (packet_seq == current_seq) {
            current_seq++;
            current_seq &= 0x1f;
            print_movie(packet_server, id_movie);
            id_movie++;
        }
        send_ACK(sockfd, packet_seq);
        if (code == END_DATA_COD) {
            break;
        }
    }
    printf("\n--------------------------------------------\n");

    return success;
}

int view_movies_list(int sockfd, unsigned char *packet_server) {
    unsigned char *solicit_movies_pckt = create_packet(NULL, 0, 0, LIST_FILES_COD);
    if (!solicit_movies_pckt) {
        printf("\n!! Erro ao fazer pedido\n\n");
        return 0;
    }

    if (!send_packet_with_confirm(sockfd, solicit_movies_pckt, packet_server)) {
        printf("\n!! Sem resposta do server.\n\n");
        free(solicit_movies_pckt);
        return 0;
    }
    free(solicit_movies_pckt);

    return recv_and_print_movie_names_packets(sockfd, packet_server);
}

/* Retorna 1 se o pacote for de dados e 0 caso contrario. */
int is_data_packet(unsigned char *packet) {
    unsigned char code = get_packet_code(packet);
    return (code == DATA_COD) || (code == END_DATA_COD);
}

/* Retorna a quantidade de buffers validos em sequencia. */
int sort_server_packets(unsigned char **server_packets, unsigned char current_seq,
                                                            short num_buffers) {
    unsigned char *aux;
    unsigned char next_packet_seq = current_seq;
    short j = 0, i = 0;
    for (; i < num_buffers; i++) {
        for (j = i; j < num_buffers; j++) {
            if (!is_data_packet(server_packets[j])) {
                continue;
            }
            if (get_packet_seq(server_packets[j]) == next_packet_seq) {
                if (i != j) {
                    aux = server_packets[i];
                    server_packets[i] = server_packets[j];
                    server_packets[j] = aux;
                }
                next_packet_seq++;
                next_packet_seq &= 0x1f;
                break;
            }
        }
        if (j == num_buffers) {
            return i;
        }
    }
    return i;
}

/* Retorna o numero de pacotes validos recebidos. */
short recv_window_packets(int sockfd, unsigned char **server_packets) {
    short idx_buffer = 0, num_bytes_recv = 0;
    while (idx_buffer < WINDOW_SIZE) {
        if ((num_bytes_recv = recv(sockfd, server_packets[idx_buffer], PACKET_SIZE, 0)) <= 0) {
            return idx_buffer;
        }
        if (!validate_packet(server_packets[idx_buffer], num_bytes_recv)) {
            continue;
        }
        idx_buffer++;
    }
    return idx_buffer;
}

void write_data_without_escapes(unsigned char *packet, FILE *file) {
    unsigned short tam_data = get_packet_data_size(packet);
    unsigned char *data_ptr = packet + 3;
    unsigned short i = 0;
    while(i < tam_data) {
        if ((data_ptr[i] == 0x88) || (data_ptr[i] == 0x81)) {
            fwrite(data_ptr + i, 1, 1, file);
            i += 2;
        }
        else {
            fwrite(data_ptr + i, 1, 1, file);
            i++;
        }
    }
}

int recv_file(int sockfd, char *filename, unsigned long int file_size) {
    FILE *file = fopen(filename, "wb");
    if (file == NULL) {
        fprintf(stderr, "Erro ao criar o arquivo\n");
        return 0;
    }
    unsigned char *server_packets[WINDOW_SIZE];
    for (short i = 0; i < WINDOW_SIZE; i++) {
        server_packets[i] = malloc(sizeof(unsigned char) * PACKET_SIZE);
    }
    
    unsigned char current_seq = 0, cod = 0;
    unsigned long int num_packets = 0;
    short idx_buffer = 0, idx_sorted_data = 0, num_tries = 0, success = 1;

    float percent = (float) (PACKET_SIZE - 4) / file_size * 100.0f;

    printf("Baixando... 0%%");
    clear_socket_buffer(sockfd);
    while (1) {
        if (num_tries == 200) {
            success = 0;
            break;
        }

        idx_buffer = recv_window_packets(sockfd, server_packets);
        idx_sorted_data = sort_server_packets(server_packets, current_seq, idx_buffer);

        if (!idx_sorted_data) {
            num_tries++;
            send_NACK(sockfd, current_seq);
            continue;
        }
        num_tries = 0;

        for (short i = 0; i < idx_sorted_data; i++) {
            printf("\r");
            printf("Baixando... %d%%", (int)(num_packets*percent));
            fflush(stdout);
            current_seq++;
            current_seq &= 0x1f;
            num_packets++;
            write_data_without_escapes(server_packets[i], file);
        }
        if (idx_sorted_data == WINDOW_SIZE) {
            send_ACK(sockfd, (current_seq - 1) & 0x1f);
        }
        else {
            send_NACK(sockfd, current_seq);
        }

        cod = get_packet_code(server_packets[idx_sorted_data-1]);
        if (cod == END_DATA_COD) {
            break;
        }
    }
    fclose(file);
    if (success) {
        printf("\r");
        printf("Baixando... 100%%\n");
    }

    for (short i = 0; i < WINDOW_SIZE; i++) {
        free(server_packets[i]);
    }

    return success;
}

void show_movie_date_size_packet(unsigned char *packet_server) {
    unsigned char *data = packet_server + 3;
    unsigned long int size = *((unsigned long int *) (data));
    unsigned short day, month, year;
    day = data[8];
    month = data[9];
    year = (data[10] << 8) | data[11];

    printf("Tamanho: %.2fMb (%lu)\n", size / 1048576.0f, size);
    printf("Data: %02d/%02d/%d\n", day, month, year);
}

int client_space_enough(unsigned char *packet_server) {
    unsigned long int size;
    memcpy(&size, packet_server + 3, sizeof(size));

    struct statvfs stat;
    if (statvfs("/", &stat) != 0) {
        printf("!! Erro ao obter informacoes do disco\n\n");
        return 0;
    }
    unsigned long long free_space = (unsigned long int)stat.f_bsize * stat.f_bavail;
    float free_space_gb = (float)free_space / 1e9;
    printf("Espaço livre: %.2f Gb\n", free_space_gb);

    return free_space >= size;
}

int handle_recv_file_desc_packet(int sockfd, unsigned char *packet_server) {
    while (1) {
        if (!recv_packet_in_timeout(sockfd, packet_server, 1)) {
            printf("!! Sem resposta do server\n\n");
            return 0;
        }
        if (get_packet_code(packet_server) == FILE_DESC_COD) {
            break;
        }
        if (get_packet_code(packet_server) == ERROR_COD) {
            if (get_error_type(packet_server) == ERROR_ACCESS_DENIED) {
                printf("!! Arquivo com acesso nao permitido\n\n");
            }
            else {
                printf("!! Arquivo nao encontrado\n\n");
            }
            return 0;
        }
    }
    return 1;
}
