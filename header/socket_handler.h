#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>
#include <netinet/in.h>
#include <unistd.h>
#include <net/if.h>

#define BUF_SIZE 1024

void error(const char *msg);

int open_raw_socket();

struct sockaddr_ll get_interface(const char *interface);
