/**
 * Copyright (C) 2018 David C. Harrison - All Rights Reserved.
 * You may not use, distribute, or modify this code without
 * the written permission of the copyright holder.
 */
#define _GNU_SOURCE
#include <crypt.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#define KEYSPACESIZE 62
#define NUMTHREADS 24
#define NONE "0"
#define CPUAFFINITY 0
#define PWLEN 4
#define HASHLEN 13
#define MAXLEN 20
#define NUMUSERS 6

struct args_t {
    int lower;
    int upper;
    char salt[2];
    char *cryptPasswd;
    char *passwd;
};

char keySpace[KEYSPACESIZE] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
int cracked = 0;
char crackedPassword[5];
void crackStealthy(char *, char *, int, char *, int);

/*
 * Parse file in path FNAME into USERS and PASSWDS, returning the COUNT of both.
 */
void parseFile(char *fname, char users[][MAXLEN], char passwds[][HASHLEN + 1], int *count) {
    FILE *passwdStream = fopen(fname, "r");
    *count = 0;
    char *line = NULL;
    size_t length = MAXLEN;
    const char delim[2] = ":";
    char *token;
    if (passwdStream  == NULL) perror("fopen");
    while(getline(&line, &length, passwdStream) != -1) {
        token = strtok(line, delim);
        strcpy(users[*count], token);
        token = strtok(NULL, delim);
        strcpy(passwds[*count], token);
        *count += 1;
    }
    fclose(passwdStream);
}

/*
 * Find the plain-text password PASSWD of length PWLEN for the user USERNAME
 * given the encrypted password CRYPTPASSWD.
 */
void crackSingle(char *username, char *cryptPasswd, int pwlen, char *passwd) {
    crackStealthy(username, cryptPasswd, pwlen, passwd, 15);
}

/*
 * Find the plain-text passwords PASSWDS of length PWLEN for the users found
 * in the old-style /etc/passwd format file at path FNAME.
 */
void crackMultiple(char *fname, int pwlen, char **passwds) {
    int size;
    char users[NUMUSERS][MAXLEN];
    char usersCryptPasswds[NUMUSERS][HASHLEN + 1];
    parseFile(fname, users, usersCryptPasswds , &size);
    for (int i = 0; i < size; i++) {
        // printf("users[%d] = %s, usersCryptPasswds[%d] = %s\n", i, users[i], i, usersCryptPasswds[i]);
        crackSingle(users[i], usersCryptPasswds[i], pwlen, passwds[i]);
        cracked = 0;
    }
}

/*
 * Find the plain-text passwords PASSWDS of length PWLEN for the users found
 * in the old-style /etc/passwd format file at pathe FNAME.
 */
void crackSpeedy(char *fname, int pwlen, char **passwds) {
    crackMultiple(fname, pwlen, passwds);
}

void printArgs(pthread_t num, long unsigned int lower, long unsigned int upper, char salt[2],
    char *cryptPasswd, char *passwd) {
        printf("\t\tth: %lu\n\t\tlower=%lu\n\t\tupper=%lu\n\t\tsalt=%c%c\n\t\thash=%s\n\n",
            num, lower, upper, salt[0], salt[1], cryptPasswd);
}

void crackSingleReentrant(int lower, int upper, char salt[2],
    char *cryptPasswd, char *passwd) {
    char *cryptRandomPasswd;
    struct crypt_data data;
    data.initialized = 0;

    for (int i = lower; i < upper && !cracked; i++) {
        for (int j = 0; j < KEYSPACESIZE && !cracked; j++) {
            for (int k = 0; k < KEYSPACESIZE && !cracked; k++) {
                for (int w = 0; w < KEYSPACESIZE; w++) {
                    char randomPasswd[5] = {keySpace[i], keySpace[j],
                                            keySpace[k], keySpace[w], '\0'};

                    cryptRandomPasswd = crypt_r(randomPasswd, salt, &data);

                    if (strcmp(cryptRandomPasswd, cryptPasswd) == 0) {
                        strcpy(crackedPassword, randomPasswd);
                        printf("crypted! = %s\n", crackedPassword);
                        cracked = 1;
                        break;
                    }
                }
            }
        }
    }
}

void *crackMultiThreaded(void *vargp) {
    struct args_t *args = vargp;
    printArgs(pthread_self(), args->lower, args->upper, args->salt, args->cryptPasswd, args->passwd);
    crackSingleReentrant(args->lower, args->upper, args->salt, args->cryptPasswd, args->passwd);
    return NULL;
}

void createThread(pthread_t *thread, int lower, int upper, int cpu,
    char salt[2], char *cryptPasswd) {
    struct args_t *args = (struct args_t*)malloc(sizeof(struct args_t));
    args->lower = lower;
    args->upper = upper;
    memcpy(args->salt, salt, 2);
    args->cryptPasswd = cryptPasswd;

    int iret = pthread_create(thread, NULL, crackMultiThreaded, (void *)args);
    if (iret) perror("createThread: thread could not be created");

    if (CPUAFFINITY) {
        cpu_set_t cpuset;

        CPU_ZERO(&cpuset);
        CPU_SET(cpu, &cpuset);

        iret = pthread_setaffinity_np(*thread, sizeof(cpu_set_t), &cpuset);
        if (iret) perror("pthread_setaffinity_np");
    }
}

void createThreads(pthread_t threads[], char salt[2], char *cryptPasswd) {
    int offset = KEYSPACESIZE / NUMTHREADS;
    int from;
    int upto;
    printf("createThreads\n");
    for (int i = 0; i < NUMTHREADS - 1; i++) {
        from = offset*i;
        upto = offset*(i+1);
        printf("\tfrom=%d to=%d\n", from, upto);
        if (cracked) return;
        createThread(&threads[i], from, upto, i, salt, cryptPasswd);
    }
    from = offset * (NUMTHREADS - 1);
    upto = KEYSPACESIZE;
    printf("\tfrom=%d to=%d\n", from, upto);
    if (cracked) return;
    createThread(&threads[NUMTHREADS - 1], from, upto, NUMTHREADS - 1, salt,
        cryptPasswd);
}

void joinThreads(pthread_t threads[]) {
    int iret = 0;
    for (int i = 0; i < NUMTHREADS; i++) {
       iret = pthread_join(threads[i], NULL);
       if (iret) perror("joinThreads: thread could not join");
       if (cracked) break;
    }
}


/*
 * Find the plain-text password PASSWD of length PWLEN for the user USERNAME
 * given the encrypted password CRYPTPASSWD without using more than MAXCPU
 * percent of any processor.
 */
void crackStealthy(char *username, char *cryptPasswd, int pwlen, char *passwd,
    int maxCpu) {
    pthread_t threads[NUMTHREADS];
    char salt[2] = {username[0], username[1]};
    createThreads(threads, salt, cryptPasswd);
    joinThreads(threads);
    strcpy(passwd, crackedPassword);
}
