/* README (plis)

 Hi There!!!
 Here is a simple http server written in C.
 As you may notice, there is a lot of vulnerabilities for you to enjoy and exploit! Have fun and exploit them all, but please, read first this:

 After many hours of research and work I have managed to build this simple server, (yes, probably you are thinking that its written by a 5 year Old kid) but it's written by a nice international student with is struggling with this course. So please be considered, I'm not an expert at all on this, so it took to much work for me to learn http, sockets, C, bugs, errors more errors and lot of stack overflow stuff.

 And also... WHY C?!!!! WHYYYYYYYYYYY????????

  Having saying this, here are some hints of how I have been using my code, and If you don't follow the steps it is very likely that it crash (bonus point for you).
  Since for the Part 1 we are expected that the users are non-malicious, I expect you to consider this. For Part 2 please have fun and break as much as you want.


 BASIC REQUIREMENTS (PART I)

 1. SET boolean BASIC = true;

 2. GET request
 For a get request, do something like:
 wget localhost:8080/path/file.txt

 This will recover the file from the server (if exist), and send it through socket to your current folder.

 3. POST
 For a normal post, do something like:
 curl -d "data to post" -X POST localhost:8080/path/file.txt

 This will move (OR CREATE) the path and the file, and write request & "data to file" in path/file.txt
 Consider that ir overrides the founded file
 (so YES, I just realized you can override the users.txt with one that you want [and don't have time to fix it], SO EASY SECURITY BREAK)


 ADVANCED REQUIREMENT

 0. SET boolean BASIC = false; // Now you have to login

 1. LOGIN,
 !!IMPORTANT NOTE: DON'T FORGET TO PLACE THE QUOTES after localhost:port like the example
 curl -d post.txt -X POST localhost:8080"/login?username=user1&password=pass&pasword=pwd"


 2. ONCE YOU HAVE LOGGED with a valid user: POST data like:
 curl -d "data to post" -X POST --header "Set-Cookie: YummyCookie" localhost:8080/path/file.txt

 - Resources and references at the end -

 LAST: there is a lot of warnings when doing make, coudn't fixe them, but the code compile hopefully! :)
 Thanks for readme, have fun!
*/

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
#include <stdbool.h>
#include <errno.h>


#define BYTES 2048


void server_POST_response(int sock, char *request);
void server_normal_post(int sock, char *request);
bool autenticate_user(char *user, char* password);
void serve_get_response(int sock,char *request);
char * path_to_file(char *request);
char * path_to_file_POST(char *request);
char * get_data_POST(request);
bool login(char *request);
char * get_the_user(char *request);
char * get_the_password(char *request);
bool check_cookie(char * request);

//CHANGE THIS VARIABLE FOR ADVANCE REQUIREMENTS TO FALSE
bool BASIC = false;

bool user_autenticated = false;
//SET loged to false, (turned true only for testing purposes)
bool loged = true;
//Default cookie, change for hash or something later
char * cookie = "YummyCookie";

// Binary method, [given]
void binary(int sock, char *fname) {
    int fd;
    int bytes;
    void *buffer[BYTES];
    if ((fd = open(fname, O_RDONLY)) != -1) {
        while ((bytes = read(fd, buffer, BYTES)) > 0)
            write(sock, buffer, bytes);
    }
}


void httpRequest(int sock, char *request) {

    //if request type POST LOGIN & user is NOT loged yet
    if(strstr(request, "login?username") && (!loged)){
        //request type login
        printf("REQUEST TYPE LOGIN");
        loged = login(request);
        if (loged){
            // Send response socket && Send cookie
            write(sock, "HTTP/1.1 200 OK\nContent-Type: text/html\nSet-Cookie: ", strlen("HTTP/1.1 200 OK\nContent-Type: text/html\nSet-Cookie: "));
            write(sock, cookie, strlen(cookie));
            write(sock, "\nContent-Length: 200\n\n", strlen("\nContent-Length: 200\n\n"));
            write(sock, "<!DOCTYPE html><html><body><h1>PAGE</h1><p>Welcome to my home</p></body></html>", strlen("<!DOCTYPE html><html><body><h1>PAGE</h1><p>Welcome to my home</p></body></html>"));
        }else{
            // ERROR ON LOGIN
            printf("\nLOGIN FAILED, sending negative response\n");
            write(sock, "HTTP/1.1 401 \nContent-Type: text/plain\n\nFAILED LOGIN", strlen("HTTP/1.1 401 \nContent-Type: text/plain\n\nFAILED LOGIN"));

        }

    }else{
        //Check if request has cookie && user loged (no apply if BASIC=true, for basic part)
        if(check_cookie(request) || BASIC){
            //If the request has the cookie:

            //BRAVO! User loged and autentificated, attend request:
            printf("\n\nREQUEST RECEIVED:\n %s\n\n", request);

            // If is request type GET:
            if (memcmp(request, "GET", strlen("GET")) == 0){
                printf(" type: GET\n");

                //CALL method in charge of serving GET request
                serve_get_response(sock, request);

            // If is request type POST:
            }else if (memcmp(request, "POST", strlen("POST")) == 0){
                printf(" type POST");

                //CALL method that sirves POST responses
                server_POST_response(sock, request);
                write(sock, "HTTP/1.1 200 OK, posted correctly", strlen("HTTP/1.1 200 OK, posted correctly"));
            }
            else{
                // I dont server other than GET/POST [YET]
                printf("NOT EITHER GET OR POST REQUEST");
                write(sock, "HTTP/1.1 401 \nContent-Type: text/plain\nNOT EITHER GET OR POST REQUEST", strlen("HTTP/1.1 401 \nContent-Type: text/plain\nNOT EITHER GET OR POST REQUEST"));
            }

        }else{
            // Request DOESN'T HAVE COOKIE, kick user off!
            char * response = "HTTP/1.1 401 Unauthorized\r\nDate: Wed, 21 Oct 2015 07:28:00 GMT\r\nWWW-Authenticate: Basic realm=\"Access to staging site";
            printf("\nAUTENTIFICATION ERROR, cookie token is missing\n");
            write(sock,response, strlen(response));
            //write(sock, "HTTP/1.1 401 AUTENTIFICATION ERROR, cookie token is missing", strlen("HTTP/1.1 401 AUTENTIFICATION ERROR, cookie token is missing"));
        }

        //End of Communication
        printf("\nEnd of Communication, Proceed to close socket\n\n");
        write(sock, "\n\nEND of service, Thank you!\n", strlen("\n\nEND of service, Thank you!\n"));
        shutdown(sock, SHUT_RDWR);
        close(sock);
    }

}


// Method that check the LOGIN
/*
 1. extract the user of the request
 2. extract the pasword of the request
 3. Open the file users.txt
 4. Check if user exist in users.txt
 5. Check if password match
 6. Return result of login(true/false)
 */
bool login(char *request){

    char * user = get_the_user(request);
    printf("\nUser:-%s-", user);

    char * password = get_the_password(request);
    printf("\nPassword:-%s-", password);

    char *path = "users.txt";
    //*Error, not working properly! */
    FILE *fp = fopen(path, "rb");
    //printf("url:%s---", path);

    // If error, give advise of how to do it (correct url) &404 page
    if (!fp){
        perror("\n\nERROR\nusers.txt Not opened. please CREATE users.txt in the server's directory, fill with user:password (one per line)");
        exit(1);
    }else{
        bool founded = false;

        // Read line by line
        char *line = malloc(30); //Max size users length
        while(!founded){
            //char a = fgets();
            free(line);
            line = malloc(30);
            if(fgets(line, 30, fp) == NULL){
                break;
            }
            if(memcmp(user, line, strcspn(line, ":")) == 0)
                founded = true;


        }

        //printf("end of lecture");
        if(founded){
            line += strcspn(line, ":") + 1;
            if(memcmp(password, line, strcspn(line, " ") -1)== 0){
                printf("\nUser & Password match: -%s-%s", user, password);
                printf("\nLOGIN SUCCEED\n");
                fclose(fp);
                return true;
            }else{
                printf("\nPassword doesn't match (devolver error trhough socket");
            }
            //if(strncmp(password, (line + strcspn(line, ":")), strcspn((line + strcspn(line, ":"), "\n")))){
            //     printf("\nPassword match: -%s-", password);
            // }
            //  printf("password:-%s-", (line + strcspn(line, ":")));
        }else{
            printf("\nUser not founded");
        }
    }


    printf("\n\nLOGIN FAILED\n\n");
    fclose(fp);
    return false;
}




// Method that serve the GET requests
/* Steps:
 - get te path of the file
 - open the file
 - retrieve data
 - send back (socket)
 */
void serve_get_response(int sock,char *request){

    char *path = path_to_file(request);
    printf("GET path:-%s-", path);
    //printf("\n\nRequest received: %s\n\n", request);

    /* DESACTIVATED, fix
    if(!BASIC){
        //USER RESTRICTIONS, user can only operates in their own folder

        if(!strstr(path, general_user){
            //kick out user
            char * response = "HTTP/1.1 401 Unauthorized\r\nDate: Wed, 21 Oct 2015 07:28:00 GMT\r\nWWW-Authenticate: Basic realm=\"Access to staging site";
            printf("\nAUTENTIFICATION ERROR, cookie token is missing\n");
            write(sock,response, strlen(response));
            //write(sock, "HTTP/1.1 401 AUTENTIFICATION ERROR, cookie token is missing", strlen("HTTP/1.1 401 AUTENTIFICATION ERROR, cookie token is missing"));
        }

    }*/

    // Opening the file and sending
    // Resources [1]. Sendinf a file trough socket:
    long fsize = 0;
    //FILE *fp = fopen("src/index.html", "rb");

    //*Error, not working properly! */
    FILE *fp = fopen(path, "rb");
    //printf("url:%s---", path);

    // If error, give advise of how to do it (correct url) &404 page
    if (!fp){
        perror("The file was not opened, check the url is valid");
        printf("url:---%s---", path);
        //bad_request(sock);
        //exit(1);
    }else{
        printf("The file was opened correctly\n");
    }


    //To Learn
    fseek(fp, 0, SEEK_END);

    fsize = ftell(fp);
    if (fsize == -1) {
        perror("The file size was not retrieved");
        //exit(1);
        return;
    }

    printf("The file size is %ld\n", fsize);

    rewind(fp);

    char *msg = (char*) malloc(fsize);

    if (fread(msg, fsize, 1, fp) != 1){
        perror("The file was not read\n");
        //exit(1);
        return;
    }
    close(fp);

    //Problem with content length!! see how to do variable [later]
    // Sending response:
    char *rp = "HTTP/1.1 200 OK\nContent-Type: text/html\nContent-Length: 800\n\n";
    printf("Sending back: \n%s\n%s", rp, msg);
    write(sock, rp, strlen(rp));
    // Sending body (requested file):
    write(sock, msg, strlen(msg));
    printf("file:%s sent correctly", path);

    return;
}


// Method that handdles the POST resoponse
/* See the steps in the comments
 - create OR open PATH according to the request
 - Create or open file and write the POST request data
 */
void server_POST_response(int sock, char * request){

    // 1. get the path
    char *path = path_to_file_POST(request);
    printf("\n\nPATH:-%s-\n\n", path);

    // 2. Create the structure of folders needed
    if(fopen_mkdir(path)){
        printf("folder structure created");
    }else{
        printf("Structure of folders created");
    }

    // 3. Open the file (or create if not before)
    int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0)
    {
        perror("coudn't open the file");
        exit(1);
    }

    //4. Write data (POST)
    // For the moment it writes all the request, parse data later
    write(fd, request, strlen(request));
    close(fd);

    printf("END POST");

}



// Check if detects the cookie on the request
bool check_cookie(char * request){
    // to do later: Security break, look for it only in header
    if (strstr(request, cookie)){
        return true;
    }
    return false;
}


    // AUXILIAR METHODS (serve previous ones)


// Method that retrieves the path of the file
char * path_to_file_POST(char *request){
    //move pointer of path just at the begining of the path
    //"POST localhost:/path/file.txt";
    // Pointer         ^

    char *path = request;
    path += strcspn(path, "/") + 1;

    int len = strcspn(path, " ");
    char *url = malloc(len + 1);
    strncpy(url, path, len);
    printf("\nthe path is: -%s-", url);
    printf("\nlength:%d\n", strlen(url));
    //printf("\nlength:%d\n", len);
    return url;
}

// Method that retrieves the data of the request
char * get_data_POST(request){
    char *path = request;
    path +=50;
    printf("length body: %d", strcspn(path, "\n"));
    //path += strcspn(path, "\n");
   // int len = 500;
    //char *content = malloc(len + 1);
    //printf("\nRequest Content:%s\n", content);
    return path;
}

// Method that rerieves the path of the file
char * path_to_file(char *request){
    //first, get the path to file
    int length;
    const char *pointer_to_request = request;

    //Move pointer at the beginning of the path
    pointer_to_request +=5;
    length = strcspn(pointer_to_request, " ");

    char *path = malloc(length + 1);
    strncpy(path, pointer_to_request, length);

    //printf("GET path:-%s-", path);
    return path;
}


// Method that open path
// NOTE, recursive, if doens't exist path then CREATES one
void rek_mkdir(char *path)
{
    // REFERENCES
    // [3] create a path of files
    // https://stackoverflow.com/questions/7430248/creating-a-new-directory-in-c
    char *sep = strrchr(path, '/' );
    if(sep != NULL) {
        *sep = 0;
        rek_mkdir(path);
        *sep = '/';
        printf("mkdireeeee");
    }
    if(mkdir(path,0777) && errno != EEXIST )
        printf("error while trying to create '%s'\n%m\n",path );

}

// Method works with the previous one
void fopen_mkdir( char *path, char *mode )
{
    char *sep = strrchr(path, '/' );
    if(sep ) {
        char *path0 = strdup(path);
        path0[ sep - path ] = 0;
        rek_mkdir(path0);
        free(path0);
    }
    return;
}



// Method that parse and extract the password of a request
char * get_the_password(char *request){

    //char *request = "POST localhost:8080"/login?username=user1&password=pass"
    char *pointer_to_request = request;
    //get the USER (delimited by the characters '=' and '&'
    int length = strcspn(request, "\&");

    pointer_to_request += length + 10;
    length = strcspn(pointer_to_request, "\n");

    char * pass = malloc(5);
    strncpy(pass, pointer_to_request, length);

    return pass;
}

// Method that parse and extract the user of a request
char * get_the_user(char *request){
    //char *request = "POST localhost:8080"/login?username=user1&password=pass&pasword=pwd"
    char *pointer_to_request = request;
    //get the USER (delimited by the characters '=' and '&'
    int length = strcspn(request, "=");
    pointer_to_request += length + 1;
    length = strcspn(pointer_to_request, "\&");

    char * user = malloc(5);
    strncpy(user, pointer_to_request, length);

    return user;
}



/* RESOURCES:
 [1] Sending a file trhough a socket: https://stackoverflow.com/questions/32749925/sending-a-file-over-a-tcp-ip-socket-web-server

 [2] Parsing request to get url: : https://codereview.stackexchange.com/questions/188384/http-request-parser-in-c

 [3] create a path of files // https://stackoverflow.com/questions/7430248/creating-a-new-directory-in-c
 */


