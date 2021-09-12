#ifndef MAIN_H
#define MAIN_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>	
#include <arpa/inet.h>	
#include <unistd.h>
#include <fcntl.h>

#define CHUNK_SIZE 4000

struct settings {
    char name[60];
    char IP[20];
    char PORT[10];
    char cred_file[50]; //!< credentials file
};

struct settings server_settings;

int send_all(char *buf, int *len);
char *recv_all(int mode);
int is_socket_alive(void);

#endif