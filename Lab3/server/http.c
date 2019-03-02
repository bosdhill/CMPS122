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
#include <errno.h>

#define BYTES 2048
#define VALID 1
#define INVALID 0
#define SIZE 512
#define USERMAX 30
#define PASSMAX 30

enum req_type{POST, GET, NONE};
char homedir[SIZE/2] = {0};
char SUCCESS[] = "HTTP/1.1 200 OK\r\n\r\n";
char NOTFOUND[] = "HTTP/1.1 404 Not Found\r\n\r\n";
char BADREQ[] = "HTTP/1.1 400 Bad Request\r\n\r\n";
char FORBIDDEN[] = "HTTP/1.1 401 Forbidden\r\n\r\n";
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
    printf("send_http_response\n");
    printf("\tsending code: %s\n", status);
    if (send(sock, (void *)status, strlen(status) + 1,0) == -1) {
        perror("send");
    }
}

// writes out file to sock
// if zero bytes, then send NOTFOUND
static void binary(int sock, char *fname) {
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
}

int create_file_named(const char *fname, char content[], int sock) {
    printf("create_file_named\n");
    int fd;
    int flags = O_RDWR | O_CREAT | O_TRUNC;
    if (EXPECT) flags |= O_APPEND;
    printf("\tfname = %s\n", fname);
    if ((fd = open(fname, flags, umask_0)) != -1) {
        write(fd, content, BYTES);
        if (EXPECT == 1) {
            printf("\tHTTP/1.1 100-continue\r\n");
            int recv_bytes;
            char response[content_length];
            memset(response, '\0', content_length);
            send_http_response(sock, CONTINUE);
            recv_bytes = recv(sock, (void *)response, content_length, 0);
            write(fd, response, content_length);
            printf("received: \n%s\n", (char *)response);
            content_length -= recv_bytes;
            printf("bytes left: %d\n", content_length);
        }
        EXPECT = 0;
        content_length = 0;
        close(fd);
        // printf("gets here?\n");
        return 1;
    }
    return -1;
}

void get_file_name_from(const char path[], char file_name[]) {
    printf("get_file_name_from\n");
    char orig_path[SIZE];
    strncpy(orig_path, path, SIZE);
    char *delim = "/";
    char *token = strtok(orig_path, delim);
    char *prev = NULL;
    do {
        prev = token;
        token = strtok(NULL, delim);
    } while (token != NULL);
    strncpy(file_name, prev, SIZE);
}

void get_path_to_file(const char path[], char file_path[]) {
    printf("get_path_to_file\n");
    char orig_path[SIZE];
    strncpy(orig_path, path, SIZE);
    char *delim = "/";
    char *token = strtok(orig_path, delim);
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
void create_directory_path_from(const char path[], char dir_path[]) {
    printf("create_directory_path_from\n");
    char orig_path[SIZE];
    strncpy(orig_path, path, SIZE);
    char *delim = "/";
    char *token = strtok(orig_path, delim);
    do {
        strncat(dir_path, token, SIZE);
        strncat(dir_path, delim, SIZE);
        printf("\tpwd: %s\n", dir_path);
        mkdir(dir_path, umask_0);
    } while((token = strtok(NULL, delim)) != NULL);
    printf("\tdir_path is now %s\n", dir_path);
}

void get_path_to_file_and_filename(const char *path, char file[], char file_path[]) {
    printf("get_path_to_file_and_file\n");
    char orig_path[SIZE] = {0};
    strncpy(orig_path, path, SIZE);
    char *delim = "/";
    char *token = strtok(orig_path, delim);
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
    strncpy(file, prev, strlen(prev) + 1);
    printf("file = %s\n", file);
    printf("path to it is = %s\n", file_path);
}

void write_file_to(int sock, const char path[], char content[]) {
    printf("write_file_to\n");
    printf("path = %s\n", path);
    char absolute_file_path[SIZE] = {0};
    char path_to_file[SIZE] = {0};
    char fname[SIZE] = {0};
    char dir_path[SIZE] = {0};
    // char orig_path[SIZE];
    // strncpy(orig_path, path, SIZE);
    printf("\tpath is now %s\n", path);
    get_path_to_file_and_filename(path, fname, path_to_file);
    printf("\tfile name = %s\n", fname);
    printf("\tpath to file = %s\n", path_to_file);
    printf("\tpath = %s\n", path);
    create_directory_path_from(path_to_file, dir_path);
    strncat(absolute_file_path, homedir, strlen(homedir) + 1);
    strncat(absolute_file_path, path, strlen(path) + 1);
    printf("absolute = %s\n", absolute_file_path);
    // chdir(absolute_file_path);
    if(create_file_named(absolute_file_path, content, sock) == -1)
        send_http_response(sock, BADREQ);
    else
        send_http_response(sock, SUCCESS);
    // printf("huh??");
    printf("pwd: %s\n",getcwd(NULL,SIZE));
    chdir(homedir);
}

void user_pass_from_http(const char *content, char *user, char *pass) {
    printf("user_pass_from_http\n");
    char orig_content[SIZE];
    strncpy(orig_content, content, SIZE);
    char *token = strtok(orig_content, "&");
    // printf("token %s\n", token);
    strncpy(user, strstr(token, "login?username=") + strlen("login?username="), USERMAX);
    token = strtok(NULL, "&");
    strncpy(pass, strstr(token, "password=") + strlen("password="), PASSMAX);
    printf("\tuser = %s\n", user);
    printf("pass = %s\n", pass);
}

void verify_user(char *user, char *pass) {
    char const* const fileName = "users"; /* should check that argc > 1 */
    FILE* file = fopen(fileName, "r"); /* should check the result */
    char line[USERMAX + PASSMAX + strlen(":") + strlen("\n") + 1];

    while (fgets(line, sizeof(line), file)) {
        /* note that fgets don't strip the terminating \n, checking its
           presence would allow to handle lines longer that sizeof(line) */
        printf("%s", line);
    }
    /* may check feof here to make a difference between eof and io failure -- network
       timeout for instance */
    fclose(file);
    exit(1);
}

// extract file path from request body
void get_path_from_http(const char *request, char path[]) {
    printf("get_path_from_http\n");
    char orig_request[SIZE];
    strncpy(orig_request, request, SIZE);
    strtok(orig_request, " ");
    strcpy(path, strtok(NULL, " "));
}

void get_content_from_http(const char *request, char content[]) {
    printf("get_content_from_http\n");
    char* end = strstr(request, "\r\n\r\n");
    if (end == NULL) {
        printf("cant find carriage newline\n");
    }
    strncpy(content, end + strlen("\r\n\r\n"), BYTES);
}

// get type of request
enum req_type get_req_type(const char *request) {
    char *req_type;
    if ((req_type = strstr(request, "GET")) != NULL && req_type == request) {
        return GET;
    }
    if ((req_type = strstr(request, "POST")) != NULL && req_type == request) {
        return POST;
    }
    return NONE;
}

// http://<host>:<port>/login?username=<user>&password=<pass>
// login?username=Bobby&password=Dhillon

void set_content_length(const char *request) {
    printf("set_content_length\n");
    // char orig_request[BYTES];
    // strncpy(orig_request, request, BYTES);
    char *length = strstr(request, "Content-Length: ") + strlen("Content-Length: ");
    if (length != NULL) {
        char *end = length + strlen(length);
        content_length = strtol(length, &end, 10);
        printf("\tcontent-length: %d\n", content_length);
    }
    // strncpy(request, orig_request, BYTES);
}

void check_expect_100(char *request) {
    printf("check_expect_100\n");
    if (abs(content_length - (int)strlen(request)) > (BYTES/2)) EXPECT = 1;
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
        char user[USERMAX + 1];
        char pass[PASSMAX + 1];
        char path[SIZE/2] = {0};
        char content[BYTES] = {0};
        get_content_from_http(request, content);
        user_pass_from_http(content, user, pass);
        verify_user(user, pass);
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