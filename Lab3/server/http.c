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
#include <sys/stat.h>

#define BYTES 2048
#define VALID 1
#define INVALID 0
#define SIZE 512

enum req_type{POST, GET, NONE};
char homedir[SIZE/2] = {0};
char SUCCESS[] = "HTTP/1.1 200 OK\r\n\r\n";
char NOTFOUND[] = "HTTP/1.1 404 Not Found\r\n\r\n";
char BADREQ[] = "HTTP/1.1 400 Bad Request\r\n\r\n";
char CONTINUE[] = "HTTP/1.1 100-continue\r\n";
unsigned char EXPECT = 0;
int content_length = 0;
mode_t umask_0 = 0777;

// https://stackoverflow.com/questions/4553012/checking-if-a-file-is-a-directory-or-just-a-file
int is_regular_file(const char *path)
{
    struct stat path_stat;
    stat(path, &path_stat);
    return S_ISREG(path_stat.st_mode);
}

void send_http_response(int sock, char status[]) {
    send(sock, (void *)status, strlen(status) + 1,0);
}

// writes out file to sock
// if zero bytes, then send NOTFOUND
static int binary(int sock, char *fname) {
    printf("binary\n");
    int fd;
    int bytes;
    void *buffer[BYTES];
    // char *end = "\r\n";
    if ((fd = open(fname, O_RDONLY)) != -1) {
        printf("\tfd = %d\n", fd);
        while ((bytes = read(fd, buffer, BYTES)) > 0) {
            write(sock, buffer, bytes);
        }
   }
   printf("buffer = %s\n", (char *)buffer);
//    write(sock, (void *)end, strlen(end) + 1);
   return strlen((char *)buffer) == 0? -1: 1;
}

int create_file_named(char *fname, char content[], int sock) {
    printf("create_file_named\n");
    int fd;
    int flags = (EXPECT == 1? O_RDWR | O_CREAT : O_RDWR | O_CREAT | O_APPEND);
    if ((fd = open(fname, flags, umask_0)) != -1) {
        write(fd, content, BYTES);
        if (EXPECT) {
            EXPECT = 0;
            int recv_bytes;
            do {
                char response[BYTES] = {0};
                printf("\tHTTP/1.1 100-continue\r\n");
                send_http_response(sock, CONTINUE);
                recv_bytes = recv(sock, (void *)response, BYTES, 0);
                write(fd, response, BYTES);
                printf("received: \n%s\n", response);
                content_length -= recv_bytes;
                printf("bytes left: %d\n", content_length);
            } while (content_length > 0);
        }
        return 1;
    }
    return -1;
}

void get_file_name_from(char path[], char file_name[]) {
    printf("get_file_name_from\n");
    char orig_path[SIZE/2];
    strncpy(orig_path, path, SIZE/2);
    char *delim = "/";
    char *token = strtok(path, delim);
    char *prev = NULL;
    do {
        prev = token;
        token = strtok(NULL, delim);
    } while (token != NULL);
    strncpy(file_name, prev, SIZE/2);
    strncpy(path, orig_path, SIZE/2);
}

void get_path_to_file(char path[], char file_path[]) {
    printf("get_path_to_file\n");
    char orig_path[SIZE/2];
    strncpy(orig_path, path, SIZE/2);
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
    strncpy(path, orig_path, SIZE/2);
}

// need file name
// need absolute path to file
void send_file_to(int sock, char path[]) {
    printf("send_file_to\n");
    char absolute_file_path[SIZE] = {0};
    char absolute_path_discl_file[SIZE] = {0};
    char path_to_file[SIZE/2] = {0};
    get_path_to_file(path, path_to_file);
    strncat(absolute_file_path, homedir, strlen(homedir) + 1);
    strncat(absolute_file_path, path, strlen(path) + 1);
    printf("\tabsolute_path = %s\n", absolute_file_path);
    strncat(absolute_path_discl_file, homedir, strlen(homedir) + 1);
    strncat(absolute_path_discl_file, path_to_file, strlen(path) + 1);
    printf("\tabsolute_path_discl_file = %s\n", absolute_path_discl_file);
    if (chdir(absolute_path_discl_file) == -1) {
        send_http_response(sock, NOTFOUND);
    }
    else {
        if (is_regular_file(absolute_file_path)) {
            send_http_response(sock, SUCCESS);
            binary(sock, absolute_file_path);
        }
        else
            send_http_response(sock, NOTFOUND);
    }
    chdir(homedir);
}

// pre-cond: at least one directory since root exists
void create_directory_path_from(char path[]) {
    printf("create_directory_path_from\n");
    char orig_path[SIZE/2];
    char dir_path[SIZE] = {0};
    strncpy(orig_path, path, SIZE/2);
    char *delim = "/";
    char *token = strtok(path, delim);
    strncat(dir_path, token, SIZE);
    strncat(dir_path, delim, SIZE);
    chdir(homedir);
    do {
        mkdir(dir_path, umask_0);
        printf("\tpwd: %s\n", dir_path);
        token = strtok(NULL, delim);
        strncat(dir_path, token, SIZE);
        strncat(dir_path, delim, SIZE);
    } while(token != NULL);
    strncpy(path, dir_path, SIZE);
    printf("\tpath is now %s\n", path);
    exit(1);
}

void write_file_to(int sock, char path[], char content[]) {
    printf("write_file_to\n");
    printf("path = %s\n", path);
    char absolute_file_path[SIZE] = {0};
    char path_to_file[SIZE/2] = {0};
    char fname[SIZE/2] = {0};
    get_file_name_from(path, fname);
    printf("\tfile = %s\n", fname);
    printf("\tpath is now %s\n", path);
    get_path_to_file(path, path_to_file);
    printf("\tfile path = %s\n", path_to_file);
    printf("\tpath is now %s\n", path);
    strncat(absolute_file_path, homedir, strlen(homedir) + 1);
    strncat(absolute_file_path, path_to_file, strlen(homedir) + 1);
    printf("\tabsolute_path = %s\n", absolute_file_path);
    if (chdir(absolute_file_path) == -1)
        create_directory_path_from(path_to_file);
    if(create_file_named(fname, content, sock) == -1)
        send_http_response(sock, BADREQ);
    else
        send_http_response(sock, SUCCESS);
    chdir(homedir);
}

// extract file path from request body
void get_path_from_http(char *request, char path[]) {
    printf("get_path_from_http\n");
    char orig_request[BYTES];
    strncpy(orig_request, request, BYTES);
    strtok(request, " ");
    strcpy(path, strtok(NULL, " "));
    strncpy(request, orig_request, BYTES);
}

void get_content_from_http(char *request, char content[]) {
    printf("get_content_from_http\n");
    char orig_request[BYTES];
    strncpy(orig_request, request, BYTES);
    char* end = strstr(request, "\r\n\r\n");
    if (end == NULL) {
        printf("cant find carriage newline\n");
    }
    strncpy(content, end + strlen("\r\n\r\n"), BYTES);
    strncpy(request, orig_request, BYTES);
}

// get type of request
enum req_type get_req_type(char *request) {
    char *req_type;
    if ((req_type = strstr(request, "GET")) != NULL && req_type == request) {
        return GET;
    }
    if ((req_type = strstr(request, "POST")) != NULL && req_type == request) {
        return POST;
    }
    return NONE;
}

void set_content_length(char *request) {
    printf("set_content_length\n");
    char orig_request[BYTES];
    strncpy(orig_request, request, BYTES);
    char *length = strstr(request, "Content-Length: ") + strlen("Content-Length: ");
    if (length != NULL) {
        char *end = length + strlen(length);
        content_length = strtol(length, &end, 10);
        printf("\tcontent-length: %d\n", content_length);
    }
    strncpy(request, orig_request, BYTES);
}

void check_expect_100(char *request) {
    printf("check_expect_100\n");
    if (content_length - strlen(request) > BYTES/2) EXPECT = 1;
}

void set_home_dir() {
    getcwd(homedir, SIZE);
}

void httpRequest(int sock, char *request) {
    set_home_dir();
    if (get_req_type(request) == GET) {
        char path[SIZE/2] = {0};
        get_path_from_http(request, path);
        check_expect_100(request);
        printf("\tpath = %s\n", path);
        send_file_to(sock, path);
    }
    else if (get_req_type(request) == POST) {
        char path[SIZE/2] = {0};
        char content[BYTES] = {0};
        get_content_from_http(request, content);
        printf("\tcontent = %s\n", content);
        get_path_from_http(request, path);
        set_content_length(request);
        check_expect_100(request);
        printf("\tpath = %s\n", path);
        write_file_to(sock, path, content);
    }
    else
        send_http_response(sock, BADREQ);
}