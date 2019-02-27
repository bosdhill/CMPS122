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
#define VALID 1
#define INVALID 0
enum type{POST, GET, NONE};
char *filepath;
char *content;

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

// extract file path from request body
void getFilePath(char *request, char *req_type) {
    filepath = strtok(request, req_type);
    filepath = strtok(filepath, "HTTP/1.1");
}

// extract content from request body
void getContent(char *content) {

}

char *handleGet(char *pathname) {
    return "BOOLIN\n";
}

// strtok by \n
// check first of line of each with get
/*
GET /hello/world HTTP/1.1
Host: localhost:4200
User-Agent: curl/7.47.0
Accept:
*/
// get type of request
enum type getReqType(char *request) {
    char *req_type;
    if ((req_type = strstr(request, "GET")) != NULL && req_type == request) {
        getFilePath(request, "GET");
        printf("filepath: %s\n", filepath);
        return GET;
    }
    if ((req_type = strstr(request, "POST")) != NULL && req_type == request) {
        getFilePath(request, "POST");
        getContent(request);
        return POST;
    }
    return NONE;
}

// check headers to see if valid http header
int checkHeader(char *request, char *header) {
    char *req_header;
    if ((req_header = strstr(request, header)) != NULL && req_header == request)
        return VALID;
    return INVALID;
}

// \n\n == content
void httpRequest(int sock, char *request) {
	printf("request: \n%s\n", request);
    printf("sock: %d\n", sock);

    if (getReqType(request) == GET) {
        send(sock, (void *)handleGet("test"), sizeof("SUCCESS\n"), 0);
    }
}

