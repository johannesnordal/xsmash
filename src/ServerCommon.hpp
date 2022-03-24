#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <netdb.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <arpa/inet.h>
#define PORT 3000
#define BACKLOG 5
#define RES_SIZE 5
#define exit_err(msg) do { perror(msg); exit(EXIT_FAILURE); } while (0)

#endif
