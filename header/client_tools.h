#ifndef _CLIENT_TOOLS_H
#define _CLIENT_TOOLS_H

#include <stdlib.h>
#include <stdio.h>
#include "../header/basic_for_packets.h"
#include "../header/common_packets.h"

/* Retorna 1 caso sucesso, 0 caso falha */
int recv_file(int sockfd, char *filename);

#endif // _CLIENT_TOOLS_H
