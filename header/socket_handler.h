#ifndef _SOCKET_HANDLER_H_
#define _SOCKET_HANDLER_H_

#include <arpa/inet.h>
#include <net/ethernet.h>
#include <linux/if_packet.h>
#include <string.h>
#include <net/if.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#define TIME_OUT_SECONDS 10

void socket_error(const char *msg);

int open_raw_socket(const char *interface);

#endif // _SOCKET_HANDLER_H_
