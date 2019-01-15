/**
 * Copyright (C) 2018 David C. Harrison - All Rights Reserved.
 * You may not use, distribute, or modify this code without
 * the written permission of the copyright holder.
 */
#include <crypt.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define KEYSPACELEN 62
#define MAXLEN 1024


void parseFile(char *fname, char users[][MAXLEN], char passwds[][MAXLEN], int *size) {
    FILE *passwdStream = fopen(fname, "r");
    int count = 0;
    char *line = NULL;
    size_t length = MAXLEN;
    const char delim[2] = ":";
    char *token;
    if (passwdStream  == NULL) perror("fopen");
    while(getline(&line, &length, passwdStream) != -1) {
        token = strtok(line, delim);
        strcpy(users[count], token);
        token = strtok(NULL, delim);
        strcpy(passwds[count], token);
        count++;
    }
    *size = count;
    fclose(passwdStream);
}

/*
 * Find the plain-text password PASSWD of length PWLEN for the user USERNAME
 * given the encrypted password CRYPTPASSWD.
 */
void crackSingle(char *username, char *cryptPasswd, int pwlen, char *passwd) {
    char keySpace[KEYSPACELEN] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    char salt[] = {username[0], username[1]};
    char *cryptRandomPasswd;

    for (int i = 0; i < KEYSPACELEN; i++) {
        for (int j = 0; j < KEYSPACELEN; j++) {
            for (int k = 0; k < KEYSPACELEN; k++) {
                if (pwlen == 4) {
                    for (int w = 0; w < KEYSPACELEN; w++) {
                        char randomPasswd[5] = {keySpace[i], keySpace[j],
                                                keySpace[k], keySpace[w], '\0'};
                        cryptRandomPasswd = crypt(randomPasswd, salt);
                        if (strcmp(cryptRandomPasswd, cryptPasswd) == 0) {
                            strcpy(passwd, randomPasswd);
                            break;
                        }
                    }
                }
                else {
                    char randomPasswd[4] = {keySpace[i], keySpace[j],
                                            keySpace[k], '\0'};
                    cryptRandomPasswd = crypt(randomPasswd, salt);
                    if (strcmp(cryptRandomPasswd, cryptPasswd) == 0) {
                        strcpy(passwd, randomPasswd);
                        break;
                    }
                }
            }
        }
    }
}

/*
 * Find the plain-text passwords PASSWDS of length PWLEN for the users found
 * in the old-style /etc/passwd format file at path FNAME.
 */
void crackMultiple(char *fname, int pwlen, char **passwds) {
    int size;
    char users[MAXLEN][MAXLEN];
    char usersCryptPasswds[MAXLEN][MAXLEN];
    parseFile(fname, users, usersCryptPasswds , &size);
    for (int i = 0; i < size; i++)
        crackSingle(users[i], usersCryptPasswds[i], pwlen, passwds[i]);
}

/*
 * Find the plain-text passwords PASSWDS of length PWLEN for the users found
 * in the old-style /etc/passwd format file at pathe FNAME.
 */
void crackSpeedy(char *fname, int pwlen, char **passwds) {
    crackMultiple(fname, pwlen, passwds);
}

// A normal C function that is executed as a thread
// when its name is specified in pthread_create()
void *myThreadFun(void *vargp)
{
    sleep(1);
    printf("Printing GeeksQuiz from Thread \n");
    return NULL;
}

/*
 * Find the plain-text password PASSWD of length PWLEN for the user USERNAME
 * given the encrypted password CRYPTPASSWD without using more than MAXCPU
 * percent of any processor.
 */
void crackStealthy(char *username, char *cryptPasswd, int pwlen, char *passwd, int maxCpu) {
    printf("maxCpu = %d\n", maxCpu);
    pthread_t thread_id;
    pthread_create(&thread_id, NULL, myThreadFun, NULL);
    pthread_join(thread_id, NULL);
    printf("After Thread\n");

    // long double a[4], b[4], loadavg;
    // FILE *fp;
    // char dump[50];

    // for(;;)
    // {
    //     fp = fopen("/proc/stat","r");
    //     fscanf(fp,"%*s %Lf %Lf %Lf %Lf",&a[0],&a[1],&a[2],&a[3]);
    //     fclose(fp);
    //     sleep(1);

    //     fp = fopen("/proc/stat","r");
    //     fscanf(fp,"%*s %Lf %Lf %Lf %Lf",&b[0],&b[1],&b[2],&b[3]);
    //     fclose(fp);

    //     loadavg = ((b[0]+b[1]+b[2]) - (a[0]+a[1]+a[2])) / ((b[0]+b[1]+b[2]+b[3]) - (a[0]+a[1]+a[2]+a[3]));
    //     printf("The current CPU utilization is : %Lf\n",loadavg);
    // }
}
