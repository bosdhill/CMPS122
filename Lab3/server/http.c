/**
 * Copyright (C) 2018 David C. Harrison - All Rights Reserved.
 * You may not use, distribute, or modify this code without
 * the express written permission of the copyright holder.
 */
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>

#define BYTES 2048
enum type{POST, GET};

// when sending files back
static void binary(int sock, char *fname) {
    int fd;
    int bytes;
    void *buffer[BYTES];
    if ((fd = open(fname, O_RDONLY)) != -1) {
        while ((bytes = read(fd, buffer, BYTES)) > 0)
            write(sock, buffer, bytes);
   }
}

char *handleGet(char *pathname) {
    return "SUCCESS\n";
}

// get type of request
enum type getType(char *request) {
    return strstr(request, "GET") != NULL ? GET: POST;
}

// \n\n == content
void httpRequest(int sock, char *request) {
	printf("request: \n%s\n", request);
    printf("sock: %d\n", sock);

    if (getType(request) == GET) {
        send(sock, (void *)handleGet("test"), sizeof("SUCCESS\n"), 0);
    }
}

