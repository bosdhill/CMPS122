/**
 * Copyright (C) David C. Harrison - All Rights Reserved.
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
#include <time.h>

#define BYTES 2048
#define VALID 1
#define INVALID 0
#define SIZE 512
#define USERMAX 30
#define PASSMAX 30
#define MAX_USERS 2000
#define LINE_LEN USERMAX + PASSMAX + strlen(":") + strlen("\n") + 1

enum req_type{POST, GET, NONE};
char homedir[SIZE/2] = {0};
char SUCCESS[] = "HTTP/1.1 200 OK\r\n\r\n";
char NOTFOUND[] = "HTTP/1.1 404 Not Found\r\n\r\n";
char BADREQ[] = "HTTP/1.1 400 Bad Request\r\n\r\n";
char FORBIDDEN[] = "HTTP/1.1 401 Forbidden\r\n\r\n";
char COOKIE[] = "HTTP/1.1 200 OK\r\nSet-Cookie: ";
char CONTINUE[] = "HTTP/1.1 100-continue\r\n";
char keys[MAX_USERS][63];
int num_keys = 0;
unsigned char EXPECT = 0;
int content_length = 0;
mode_t umask_0 = 0777;
int SOCK;

// https://stackoverflow.com/questions/4553012/checking-if-a-file-is-a-directory-or-just-a-file
int is_regular_file(const char *path)
{
    struct stat path_stat;
    stat(path, &path_stat);
    return S_ISREG(path_stat.st_mode);
}

void send_http_response(int sock, char status[]) {
    if (send(sock, (void *)status, strlen(status) + 1,0) == -1) {
        perror("send");
    }
}

void send_http_cookie(int sock, char cookie[]) {
    strcat(cookie, "\r\n");
    send(sock, (void *)COOKIE, strlen(COOKIE) + 1,0);
    send(sock, (void *)cookie, strlen(cookie) + 1,0);
}

static void binary(int sock, char *fname) {
    int fd;
    int bytes;
    void *buffer[BYTES];
    if ((fd = open(fname, O_RDONLY)) != -1) {
        while ((bytes = read(fd, buffer, BYTES)) > 0) {
            write(sock, buffer, bytes);
        }
   }
}

int is_printable(int character) {
    if ((character >= 40 && character <= 126)
        || (character >= 129 && character <= 169))
        return VALID;
    return INVALID;
}

int create_file_named(const char *fname, char content[], int sock) {
    int fd;
    int flags = O_RDWR | O_CREAT | O_TRUNC;
    if (EXPECT) flags |= O_APPEND;
    if ((fd = open(fname, flags, umask_0)) != -1) {
        write(fd, content, BYTES);
        if (EXPECT == 1) {
            char response[content_length];
            memset(response, '\0', content_length);
            send_http_response(sock, CONTINUE);
            recv(sock, (void *)response, content_length, 0);
            write(fd, response, content_length);
        }
        EXPECT = 0;
        content_length = 0;
        close(fd);
        return 1;
    }
    return -1;
}

void get_path_to_file(const char path[], char file_path[]) {
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

void send_file_to(int sock, char path[]) {
    char absolute_file_path[SIZE] = {0};
    char absolute_path_discl_file[SIZE] = {0};
    char path_to_file[SIZE/2] = {0};
    get_path_to_file(path, path_to_file);
    strncat(absolute_file_path, homedir, strlen(homedir) + 1);
    strncat(absolute_file_path, path, strlen(path) + 1);
    strncat(absolute_path_discl_file, homedir, strlen(homedir) + 1);
    strncat(absolute_path_discl_file, path_to_file, strlen(path) + 1);
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
int create_directory_path_from(const char path[], char dir_path[]) {
    char orig_path[SIZE];
    strncpy(orig_path, path, SIZE);
    char *delim = "/";
    char *token = strtok(orig_path, delim);
    if (token == NULL) return INVALID;
    do {
        strncat(dir_path, token, SIZE);
        strncat(dir_path, delim, SIZE);
        mkdir(dir_path, umask_0);
    } while((token = strtok(NULL, delim)) != NULL);
    return VALID;
}

void get_path_to_file_and_filename(const char *path, char file[], char file_path[]) {
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
}

void write_file_to(int sock, const char path[], char content[]) {
    char absolute_file_path[SIZE] = {0};
    char path_to_file[SIZE] = {0};
    char fname[SIZE] = {0};
    char dir_path[SIZE] = {0};
    get_path_to_file_and_filename(path, fname, path_to_file);
    if (create_directory_path_from(path_to_file, dir_path) == INVALID){
        send_http_response(sock, BADREQ);
        exit(-1);
    }
    strncat(absolute_file_path, homedir, strlen(homedir) + 1);
    strncat(absolute_file_path, path, strlen(path) + 1);
    if(create_file_named(absolute_file_path, content, sock) == -1)
        send_http_response(sock, BADREQ);
    else
        send_http_response(sock, SUCCESS);
    chdir(homedir);
}

int get_user_pass_from_http(const char *content, char *user, char *pass) {
    char orig_content[SIZE];
    strncpy(orig_content, content, SIZE);
    char *token = strtok(orig_content, "&");
    if (token == NULL) return INVALID;
    strncpy(user, strstr(token, "login?username=") + strlen("login?username="), USERMAX);
    token = strtok(NULL, "&");
    strncpy(pass, strstr(token, "password=") + strlen("password="), PASSMAX);
    return VALID;
}

void get_user_from_path(const char *path, char user[]) {
    char orig_path[SIZE];
    strncpy(orig_path, path, SIZE);
    char *token = strtok(orig_path, "/");
    strncpy(user, token, USERMAX + 1);
}

int find_user_in_file(const char *user, const char *cookie) {
    FILE* file = fopen("users", "r");
    char line[LINE_LEN];
    char orig_line[LINE_LEN];
    char msg[LINE_LEN];
    char *next_user = NULL;
    char *next_pass = NULL;
    while (fgets(line, sizeof(line), file)) {
        strncpy(orig_line, line, LINE_LEN);
        next_user = strtok(line, ":");
        next_pass = strtok(NULL, ":");
        next_pass[strcspn(next_pass, "\n")] = 0;
        if (strcmp(next_user, user) == 0) {
            orig_line[strcspn(orig_line, "\n")] = 0;
            for (int i = 0; i < MAX_USERS; i++) {
                if (strlen(cookie) == strlen(keys[i])) {
                    for (int j = 0; j < strlen(cookie); j++) {
                        msg[j] = cookie[j] ^ keys[i][j];
                    }
                    printf("msg = %s, line = %s\n", msg, line);
                    if (strcmp(msg, orig_line) == 0) {
                        printf("User: %s is authenticated\n", user);
                        return VALID;
                    }
                }

            }
        }
    }
    fclose(file);
    return INVALID;
}

void encrypt_username(const char *line, char cookie[]) {
    char orig_line[LINE_LEN];
    strncpy(orig_line, line, LINE_LEN);
    int N = strlen(line);
    char cipher_text[N];
    char key[N];
    char msg[N];
    printf("\nmessage: ");
    srand(time(NULL));
    // initial key
    for (int i = 0; i < N; i++) {
        key[i] = (unsigned char)(1 + rand() % 100);
    }
    // adjust key and encrypt_username
    for (int i = 0; i < N; i++) {
        printf("%c", line[i]);
        while (!is_printable(line[i] ^ key[i])) {
            key[i] = (unsigned char)(1 + rand() % 100);
        }
        cipher_text[i] = line[i] ^ key[i];
    }
    // printf("\nkey: %s", key);
    memcpy(keys[num_keys++], key, N);

    printf("\ncipher text: %s", cipher_text);
    for (int i = 0; i < N; i++) {
        msg[i] = cipher_text[i] ^ key[i];
    }
    printf("\nmessage: %s\n", msg);
}

int verify_user(const char *user, const char *pass) {
    FILE* file = fopen("users", "r");
    char line[LINE_LEN];
    char orig_line[LINE_LEN];
    char *next_user = NULL;
    char *next_pass = NULL;
    while (fgets(line, sizeof(line), file)) {
        strncpy(orig_line, line, LINE_LEN);
        next_user = strtok(line, ":");
        next_pass = strtok(NULL, ":");
        next_pass[strcspn(next_pass, "\n")] = 0;
        if (strcmp(next_user, user) == 0 && strcmp(next_pass, pass) == 0) {
            orig_line[strcspn(orig_line, "\n")] = 0;
            char cookie[50];
            encrypt_username(orig_line, cookie);
            return VALID;
        }
    }
    fclose(file);
    return INVALID;
}

int verify_cookie(const char *path, const char *cookie) {
    char user[USERMAX + 1];
    get_user_from_path(path, user);
    return verify_user(user, cookie);
}

void get_path_from_http(const char *request, char path[]) {
    char orig_request[SIZE];
    strncpy(orig_request, request, SIZE);
    strtok(orig_request, " ");
    strcpy(path, strtok(NULL, " "));
}

int valid_path(const char *path) {
    char orig_path[SIZE];
    strncpy(orig_path, path, SIZE);
    return strlen(orig_path) > 1 && strstr(orig_path, "..") == NULL;
}

void get_content_from_http(const char *request, char content[]) {
    char* end = strstr(request, "\r\n\r\n");
    if (end == NULL) {
        return;
    }
    strncpy(content, end + strlen("\r\n\r\n"), BYTES);
}

int get_cookie_from_http(const char *request, char cookie[]) {
    char orig_request[SIZE];
    strncpy(orig_request, request, SIZE);
    char* begin = strstr(request, "Cookie: cookie=");
    if (begin == NULL) {
        return INVALID;
    }
    strncpy(cookie, begin + strlen("Cookie: cookie="), PASSMAX);
    cookie[strcspn(cookie, "\r\n")] = 0;
    return VALID;
}

int verify(const char *request, const char *path, char cookie[]) {
    return strlen(path) > 1 && get_cookie_from_http(request, cookie) == VALID
            && verify_cookie(path, cookie) == VALID;
}

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

void set_content_length(const char *request) {
    char *length = strstr(request, "Content-Length: ") + strlen("Content-Length: ");
    if (length != NULL) {
        char *end = length + strlen(length);
        content_length = strtol(length, &end, 10);
    }
}

void set_cookie(const char *user, const char *pass, char cookie[]) {
    strncpy(cookie, pass, PASSMAX);
}

void check_expect_100(char *request) {
    if (abs(content_length - (int)strlen(request)) > (BYTES/2)) EXPECT = 1;
}

void set_home_dir() {
    getcwd(homedir, SIZE);
}

void httpRequest(int sock, char *request) {
    SOCK = sock;
    set_home_dir();
    if (get_req_type(request) == GET) {
        char path[SIZE];
        char cookie[PASSMAX + strlen("\r\n") + 1];
        get_path_from_http(request, path);
        if (valid_path(path) && verify(request, path, cookie)) {
            send_file_to(sock, path);
        }
        else
            send_http_response(sock, FORBIDDEN);
    }
    else if (get_req_type(request) == POST) {
        char user[USERMAX + 1];
        char pass[PASSMAX + 1];
        char cookie[PASSMAX + strlen("\r\n") + 1];
        char path[SIZE];
        char content[BYTES];

        get_path_from_http(request, path);
        if (valid_path(path)) {
            if (get_cookie_from_http(request, cookie) == VALID
                && verify_cookie(path, cookie) == VALID) {
                    get_content_from_http(request, content);
                    set_content_length(request);
                    check_expect_100(request);
                    write_file_to(sock, path, content);
            }
            else if (get_user_pass_from_http(path, user, pass) == VALID
                    && verify_user(user, pass) == VALID) {
                    set_cookie(user, pass, cookie);
                    send_http_cookie(sock, cookie);
            }
            else
                send_http_response(sock, FORBIDDEN);
        }
        else
            send_http_response(sock, BADREQ);
    }
    else
        send_http_response(sock, BADREQ);
    close(sock);
}
