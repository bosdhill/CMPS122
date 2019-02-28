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
#define SIZE 256
enum type{POST, GET, NONE};
char homedir[SIZE];

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

char *findFile(char *path) {
    char *token = strtok(path, "/");
    char *prev;
    do {
        prev = token;
        token = strtok(NULL, "/");
    } while (token != NULL);
    printf("prev = %s\n", prev);
    return prev;
}

char *concat(char dest[], char src[]) {
    char *ret_str = calloc(1, strlen(dest) + strlen(src) + 1);
    strcat(ret_str, dest);
    strcat(ret_str, src);
    return ret_str;
}

char *getPathToFile(char *path) {
    char *delim = "/";
    char *pathToFile = calloc(1, strlen(path));
    char *token = strtok(path, delim);
    strcat(pathToFile, token);
    strcat(pathToFile, delim);
    do {
        token = strtok(NULL, delim);
        strcat(pathToFile, token);
        strcat(pathToFile, delim);
        printf("%s\n", pathToFile);
    } while (token != NULL);
    printf("pathToFile: %s\n", pathToFile);
    printf("path= %s\n", concat(homedir, pathToFile));
    return concat(homedir, pathToFile);
}

void sendFile(int sock, char *path) {
    printf("sendFile\n");
    chdir(getPathToFile(path));
    binary(sock, findFile(path));
    chdir(homedir);
}

// extract file path from request body
char *getPathFromHttp(char *request) {
    strtok(request, " ");
    return strtok(NULL, " ");
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
        return GET;
    }
    if ((req_type = strstr(request, "POST")) != NULL && req_type == request) {
        return POST;
    }
    return NONE;
}

void setHomeDir() {
    getcwd(homedir, SIZE);
}

// \r\n is a newline in curl
void httpRequest(int sock, char *request) {
	printf("request: \n%s\n", request);
    printf("sock: %d\n", sock);

    setHomeDir();
    if (getReqType(request) == GET) {
        char *path = getPathFromHttp(request);
        printf("path = %s\n", path);
        sendFile(sock, path);
    }
}

