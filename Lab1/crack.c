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
#define MAXLEN 1024
#define NUMTHREADS 20
#define NONE "0"
#define CPUAFFINITY 0

struct args_t {
    int lower;
    int upper;
    char username[MAXLEN];
    char cryptPasswd[MAXLEN];
    char passwd[MAXLEN];
};

char keySpace[KEYSPACESIZE] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
int cracked = 0;

void crackStealthy(char *, char *, int, char *, int);
/*
 * Parse file in path FNAME into USERS and PASSWDS, returning the COUNT of both.
 */
void parseFile(char *fname, char users[][MAXLEN], char passwds[][MAXLEN], int *count) {
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
    char users[MAXLEN][MAXLEN];
    char usersCryptPasswds[MAXLEN][MAXLEN];
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

void printArgs(int num, int lower, int upper, char *username,
    char *cryptPasswd, char *passwd) {
        printf("round %d args: %d %d %s %s %s\n", num, lower, upper, username,
            cryptPasswd, passwd);
}

void crackSingleReentrant(int lower, int upper, char username[],
    char cryptPasswd[], char *passwd) {
    char salt[] = {username[0], username[1]};
    char *cryptRandomPasswd;
    struct crypt_data data;
    data.initialized = 0;

    // if (lower == 54 || upper == 57)
    //     printArgs(2, lower, upper, username, cryptPasswd, passwd);

    for (int i = lower; i < upper && !cracked; i++) {
        for (int j = 0; j < KEYSPACESIZE && !cracked; j++) {
            for (int k = 0; k < KEYSPACESIZE && !cracked; k++) {
                for (int w = 0; w < KEYSPACESIZE; w++) {
                    char randomPasswd[5] = {keySpace[i], keySpace[j],
                                            keySpace[k], keySpace[w], '\0'};

                    cryptRandomPasswd = crypt_r(randomPasswd, salt, &data);
                    if (strcmp(cryptRandomPasswd, cryptPasswd) == 0) {
                        strcpy(passwd, randomPasswd);
                        // printf("found with id = %lu\n", pthread_self());
                        // printArgs(3, lower, upper, username, cryptPasswd, passwd);
                        cracked = 1;
                        break;
                    }
                }
            }
        }
    }
}

void *crackMultiThreaded(void *vargp) {
    struct args_t *args = (struct args_t *)vargp;
    // if (args->lower == 54)
    //     printArgs(1, args->lower, args->upper, args->username,
    //         args->cryptPasswd, args->passwd);
    // if (args->lower == 57)
    //     printArgs(11, args->lower, args->upper, args->username,
    //         args->cryptPasswd, args->passwd);

    crackSingleReentrant(args->lower, args->upper, args->username,
        args->cryptPasswd, args->passwd);
    if (strcmp(args->passwd, NONE) != 0)
        return (void *)args->passwd;
    return NULL;
}

void createThread(pthread_t *thread, int lower, int upper, int cpu,
    char username[], char cryptPasswd[]) {
    struct args_t args;
    args.lower = lower;
    args.upper = upper;
    strcpy(args.username, username);
    strcpy(args.cryptPasswd, cryptPasswd);
    strcpy(args.passwd, NONE);

    // if (lower == 54)
    //     printArgs(0, lower, upper, args.username, args.cryptPasswd, args.passwd);
    // if (lower == 57)
    //     printArgs(10, lower, upper, args.username, args.cryptPasswd, args.passwd);

    int iret = pthread_create(thread, NULL, crackMultiThreaded, &args);
    if (iret) perror("createThread: thread could not be created");

    if (CPUAFFINITY) {
        cpu_set_t cpuset;

        CPU_ZERO(&cpuset);
        CPU_SET(cpu, &cpuset);

        iret = pthread_setaffinity_np(*thread, sizeof(cpu_set_t), &cpuset);
        if (iret) perror("pthread_setaffinity_np");
    }
}

void createThreads(pthread_t threads[], char *username, char *cryptPasswd) {
    int offset = KEYSPACESIZE / NUMTHREADS;
    int from;
    int upto;
    char args_username[MAXLEN];
    char args_cryptPasswd[MAXLEN];

    for (int i = 0; i < NUMTHREADS - 1; i++) {
        from = offset*i;
        upto = offset*(i+1);
        // printf("from %d to %d\n", from, upto);
        strcpy(args_username, username);
        strcpy(args_cryptPasswd, cryptPasswd);
        createThread(&threads[i], from, upto, i, args_username,
            args_cryptPasswd);
        memset(args_username, '\0', sizeof(args_username));
        memset(args_cryptPasswd, '\0', sizeof(args_username));
    }
    from = offset * (NUMTHREADS - 1);
    upto = KEYSPACESIZE;
    // printf("from %d to %d\n", from, upto);
    strcpy(args_username, username);
    strcpy(args_cryptPasswd, cryptPasswd);
    createThread(&threads[NUMTHREADS - 1], from, upto, NUMTHREADS - 1, args_username,
        args_cryptPasswd);
}

void joinThreads(pthread_t threads[], char *passwd) {
    int iret = 0;
    void *ret = NULL;
    for (int i = 0; i < NUMTHREADS; i++) {
    //    printf("joining thread id = %lu\n", threads[i]);
       iret = pthread_join(threads[i], &ret);
       if (iret) perror("joinThreads: thread could not join");
       if (ret != NULL) {
           strcpy(passwd, (char *)ret);
           break;
       }
    }
    printf("passwd = %s\n", passwd);
}


/*
 * Find the plain-text password PASSWD of length PWLEN for the user USERNAME
 * given the encrypted password CRYPTPASSWD without using more than MAXCPU
 * percent of any processor.
 */
void crackStealthy(char *username, char *cryptPasswd, int pwlen,
    char *passwd, int maxCpu) {
    pthread_t threads[NUMTHREADS];
    createThreads(threads, username, cryptPasswd);
    joinThreads(threads, passwd);
}
