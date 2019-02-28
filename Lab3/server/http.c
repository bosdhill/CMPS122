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
#define SIZE 512

enum req_type{POST, GET, NONE};
char homedir[SIZE/2] = {0};
char SUCCESS[] = "\nHTTP/1.1 200 OK\n";
char NOTFOUND[] = "\nHTTP/1.1 404 Not Found\n";
char BADREQ[] = "\nHTTP/1.1 400 Bad Request\n"; 


void http_response(int sock) {
    send(sock, (void *)SUCCESS, strlen(SUCCESS) + 1,0);
}

// writes out file to sock
// if zero bytes, then send NOTFOUND
static void binary(int sock, char *fname) {
    printf("binary(%d, %s)\n", sock, fname);
    int fd;
    int bytes;
    void *buffer[BYTES];
    if ((fd = open(fname, O_RDONLY)) != -1) {
        printf("\tfd = %d\n", fd);
        while ((bytes = read(fd, buffer, BYTES)) > 0) {
            write(sock, buffer, bytes);
        }
   }
}

void get_file_name_from(char *path, char file_name[]) {
    printf("get_file_name_from(%s, %s)\n", path, file_name);
    char *delim = "/";
    char *token = strtok(path, delim);
    char *prev = NULL;
    do {
        prev = token;
        token = strtok(NULL, delim);
    } while (token != NULL);
    strncpy(file_name, prev, SIZE/4);
}

void get_path_to_file(char *path, char file_path[]) {
    printf("get_path_to_file(%s, %s)\n", path, file_path);
    char *delim = "/";
    char *token = strtok(path, delim);
    char *prev = NULL;
    strcat(file_path, delim);
    if (token != NULL) {
        prev = token;
        while ((token = strtok(NULL, delim)) != NULL) {
            strcat(file_path, prev);
            strcat(file_path, delim);
            prev = token;
        }
    }
}

// need file name
// need absolute path to file
void send_file_to(int sock, char path[]) {
    printf("send_file_to(%d, %s)\n", sock, path);
    char absolute_file_path[SIZE] = {0};
    strncat(absolute_file_path, homedir, strlen(homedir) + 1);
    strncat(absolute_file_path, path, strlen(path) + 1);
    printf("\tabsolute_path = %s\n", absolute_file_path);
    http_response(sock);
    binary(sock, absolute_file_path);
}

void write_file_to(int sock, char path[]) {
    printf("write_file_to(%d, %s)\n", sock, path);
    char absolute_file_path[SIZE] = {0};
    strncat(absolute_file_path, homedir, strlen(homedir) + 1);

}

// extract file path from request body
void getPathFromHttp(char *request, char path[]) {
    printf("getPathFromHttp(%s, %s)\n", request, path);
    strtok(request, " ");
    strcpy(path, strtok(NULL, " "));
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
enum req_type getReqType(char *request) {
    printf("getReqType(\n%s\n)\n", request);
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
    setHomeDir();
    if (getReqType(request) == GET) {
        char path[SIZE/2] = {0};
        getPathFromHttp(request, path);
        printf("\tpath = %s\n", path);
        send_file_to(sock, path);
    }
    if (getReqType(request) == POST) {
        char path[SIZE/2] = {0};
        getPathFromHttp(request, path);
        printf("\tpath = %s\n", path);
        write_file_to(sock, path);
    }
}