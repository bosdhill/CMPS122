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

struct args_t {
    int lower;
    int upper;
    char username[MAXLEN];
    char cryptPasswd[MAXLEN];
    char passwd[MAXLEN];
};

char keySpace[KEYSPACESIZE] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
pthread_mutex_t lock;
int cracked = 0;

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
    // exit(1);
    // char salt[] = {username[0], username[1]};
    // char *cryptRandomPasswd;
    // int cracked = 0;
    // int count = 0;

    // for (int i = 0; i < KEYSPACESIZE && !cracked; i++) {
    //     for (int j = 0; j < KEYSPACESIZE && !cracked; j++) {
    //         for (int k = 0; k < KEYSPACESIZE && !cracked; k++) {
    //             for (int w = 0; w < KEYSPACESIZE; w++) {
    //                 char randomPasswd[5] = {keySpace[i], keySpace[j],
    //                                         keySpace[k], keySpace[w], '\0'};
    //                 count++;
    //                 if (count == 100000000)
    //                     exit(1);

    //                 cryptRandomPasswd = crypt(randomPasswd, salt);
    //                 if (strcmp(cryptRandomPasswd, cryptPasswd) == 0) {
    //                     strcpy(passwd, randomPasswd);
    //                     cracked = 1;
    //                     break;
    //                 }
    //             }
    //         }
    //     }
    // }
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

void crackSingleReentrant(int lower, int upper, char *username,
    char *cryptPasswd, char *passwd) {
    pthread_t me = pthread_self();
    char salt[] = {username[0], username[1]};
    char *cryptRandomPasswd;
    struct crypt_data data;
    data.initialized = 0;
    int count = 0;
    printf("thread %lu with %d %d %s %s %s\n", me, lower, upper,
        username, cryptPasswd, passwd);
    // printf("in thread %lu\n", me);
    for (int i = lower; i < upper && !cracked; i++) {
        for (int j = 0; j < KEYSPACESIZE && !cracked; j++) {
            for (int k = 0; k < KEYSPACESIZE && !cracked; k++) {
                for (int w = 0; w < KEYSPACESIZE; w++) {
                    char randomPasswd[5] = {keySpace[i], keySpace[j],
                                            keySpace[k], keySpace[w], '\0'};

                    cryptRandomPasswd = crypt_r(randomPasswd, salt, &data);
                    // if (strcmp(randomPasswd, "mA5s") == 0) {
                    //     printf("password = %s\n", randomPasswd);
                    //     printf("got it!\n");
                    //     printf("cryptRand = %s, cryptPass = %s\n", cryptRandomPasswd, cryptPasswd);
                    // }
                    if (strcmp(cryptRandomPasswd, cryptPasswd) == 0) {
                        // pthread_mutex_lock(&lock);
                        strcpy(passwd, randomPasswd);
                        printf("cracked in %lu!", me);
                        // acquire lock
                        cracked = 1;
                        // pthread_mutex_unlock(&lock);
                        break;
                    }
                    // usleep(4500);
                }
            }
        }
    }
}

void *crackMultiThreaded(void *vargp) {
    struct args_t *args = (struct args_t *)vargp;
    pthread_t me = pthread_self();
    printf("thread %lu with %d %d %s %s %s\n", me, args->lower, args->upper,
        args->username, args->cryptPasswd, args->passwd);
    crackSingleReentrant(args->lower, args->upper, args->username,
        args->cryptPasswd, args->passwd);

    if (strcmp(args->passwd, NONE) != 0) {
        printf("found %s\n", args->passwd);
        return (void *)args->passwd;
    }
    return NULL;
}

void createThread(pthread_t *thread, int lower, int upper, int cpu,
    char *username, char *cryptPasswd) {
    struct args_t args;
    args.lower = lower;
    args.upper = upper;
    strcpy(args.username, username);
    strcpy(args.cryptPasswd, cryptPasswd);
    strcpy(args.passwd, NONE);

    printf("    args = %d %d %s %s %s\n", args.lower, args.upper, args.username, args.cryptPasswd, args.passwd);

    cpu_set_t cpuset;

    CPU_ZERO(&cpuset);
    CPU_SET(cpu, &cpuset);

    int iret = pthread_create(thread, NULL, crackMultiThreaded, &args);
    // printf("thread %lu from %d to %d\n", thread, lower, upper);
    if (iret) perror("createThread: thread could not be created");
    sleep(0.1);

    iret = pthread_setaffinity_np(*thread, sizeof(cpu_set_t), &cpuset);
    if (iret) perror("pthread_setaffinity_np");

    iret = pthread_getaffinity_np(*thread, sizeof(cpu_set_t), &cpuset);
    if (iret) perror("pthread_getaffinity_np");
}

void createThreads(pthread_t threads[], char *username, char *cryptPasswd) {
    int offset = KEYSPACESIZE / NUMTHREADS;
    int from;
    int upto;

    for (int i = 0; i < NUMTHREADS - 1; i++) {
        from = offset*i;
        upto = offset*(i+1);
        char args_username[MAXLEN];
        char args_cryptPasswd[MAXLEN];
        strcpy(args_username, username);
        strcpy(args_cryptPasswd, cryptPasswd);
        createThread(&threads[i], from, upto, i, args_username,
            args_cryptPasswd);
    }
    from = offset * (NUMTHREADS - 1);
    upto = KEYSPACESIZE;
    char args_username[MAXLEN];
    char args_cryptPasswd[MAXLEN];
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
           printf("passwd = %s\n", passwd);
           break;
       }
    }
    printf("cracked = %d\n", cracked);
    printf("password = %s\n", passwd);
}


/*
 * Find the plain-text password PASSWD of length PWLEN for the user USERNAME
 * given the encrypted password CRYPTPASSWD without using more than MAXCPU
 * percent of any processor.
 */
void crackStealthy(char *username, char *cryptPasswd, int pwlen,
    char *passwd, int maxCpu) {
    pthread_t threads[NUMTHREADS];
    // pthread_mutex_init(&lock, NULL);
    createThreads(threads, username, cryptPasswd);
    joinThreads(threads, passwd);
    // pthread_mutex_destroy(&lock);
    printf("password = %s\n", passwd);
}
