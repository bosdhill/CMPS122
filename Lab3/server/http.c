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
char SUCCESS[] = "HTTP/1.1 200 OK\r\n";
char NOTFOUND[] = "HTTP/1.1 404 Not Found\r\n";
char BADREQ[] = "HTTP/1.1 400 Bad Request\r\n";


void http_response(int sock, char status[]) {
    send(sock, (void *)status, strlen(status) + 1,0);
}

// writes out file to sock
// if zero bytes, then send NOTFOUND
static void binary(int sock, char *fname) {
    printf("binary\n");
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

int create_file_named(char *fname, char content[]) {
    int fd;
    if ((fd = open(fname, O_RDWR | O_CREAT)) != -1) {
        write(fd, content, BYTES);
        return 1;
    }
    return -1;
}

void get_file_name_from(char *path, char file_name[]) {
    printf("get_file_name_from\n");
    char *delim = "/";
    char *token = strtok(path, delim);
    char *prev = NULL;
    do {
        prev = token;
        token = strtok(NULL, delim);
    } while (token != NULL);
    strncpy(file_name, prev, SIZE/2);
}

void get_path_to_file(char *path, char file_path[]) {
    printf("get_path_to_file\n");
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
    printf("send_file_to\n");
    char absolute_file_path[SIZE] = {0};
    strncat(absolute_file_path, homedir, strlen(homedir) + 1);
    strncat(absolute_file_path, path, strlen(path) + 1);
    printf("\tabsolute_path = %s\n", absolute_file_path);
    http_response(sock, SUCCESS);
    binary(sock, absolute_file_path);
}

void write_file_to(int sock, char *path, char content[]) {
    printf("write_file_to\n");
    char absolute_file_path[SIZE] = {0};
    char path_to_file[SIZE/2] = {0};
    char fname[SIZE/2];
    get_file_name_from(path, fname);
    printf("\tfile = %s\n", fname);
    printf("\tpath is now %s\n", path);
    get_path_to_file(path, path_to_file);
    printf("\tfile path = %s\n", path_to_file);
    printf("\tpath is now %s\n", path);
    strncat(absolute_file_path, homedir, strlen(homedir) + 1);
    strncat(absolute_file_path, path_to_file, strlen(homedir) + 1);
    printf("\tabsolute_path = %s\n", absolute_file_path);
    chdir(absolute_file_path);
    if (create_file_named(fname, content) == -1)
        http_response(sock, BADREQ);
    else
        http_response(sock, SUCCESS);
    chdir(homedir);
}

// extract file path from request body
void get_path_from_http(char *request, char path[]) {
    printf("get_path_from_http\n");
    strtok(request, " ");
    strcpy(path, strtok(NULL, " "));
}

void get_after(char *request, char *delim, char after[]){
    printf("get_after\n");
    strtok(request, delim);
    strcpy(after, strtok(NULL, delim));
}

void get_content_from_http(char *request, char content[]) {
    printf("get_content_from_http\n");
    char* end = strstr(request, "\r\n\r\n");
    if (end == NULL) {
        printf("cant find carriage newline\n");
    }
    strncpy(content, end + strlen("\r\n\r\n"), BYTES);
}

// get type of request
enum req_type getReqType(char *request) {
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
	// printf("request:\n%s\n", request);
    setHomeDir();
    if (getReqType(request) == GET) {
        char path[SIZE/2] = {0};
        get_path_from_http(request, path);
        printf("\tpath = %s\n", path);
        send_file_to(sock, path);
    }
    else if (getReqType(request) == POST) {
        char path[SIZE/2] = {0};
        char content[BYTES] = {0};
        get_content_from_http(request, content);
        printf("\tcontent = %s\n", content);
        get_path_from_http(request, path);
        printf("\tpath = %s\n", path);
        write_file_to(sock, path, content);
    }
    else
        http_response(sock, BADREQ);
}