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
#include <sys/types.h>
#include <sys/wait.h>
#define KEYSPACESIZE 62
#define NUMTHREADS 24
#define CPUAFFINITY 1
#define PWLEN 4
#define HASHLEN 13
#define MAXLEN 128
#define NUMUSERS 6
#define READ_END pipefd[0]
#define WRITE_END pipefd[1]

struct args_t {
    int lower;
    int upper;
    char *cryptPasswd;
};

char keySet[KEYSPACESIZE] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
int cracked = 0;
char crackedPassword[5];

/*
 * Compare all combinations of crypt hashs of ketSet from index LOWER to UPPER
 * and save result in crackedPassword.
 */
void crackSingleReentrant(int lower, int upper, char *cryptPasswd) {
    char *cryptRandomPasswd;
    struct crypt_data data;
    int i, j, k, w;
    char salt[2] = {cryptPasswd[0], cryptPasswd[1]};
    data.initialized = 0;

    for (i = lower; i < upper && !cracked; i++) {
        for (j = 0; j < KEYSPACESIZE && !cracked; j++) {
            for (k = 0; k < KEYSPACESIZE && !cracked; k++) {
                for (w = 0; w < KEYSPACESIZE; w++) {
                    char randomPasswd[5] = {keySet[i], keySet[j],
                                            keySet[k], keySet[w], '\0'};
                    cryptRandomPasswd = crypt_r(randomPasswd, salt, &data);
                    if (strcmp(cryptRandomPasswd, cryptPasswd) == 0) {
                        strcpy(crackedPassword, randomPasswd);
                        cracked = 1;
                        break;
                    }
                }
            }
        }
    }
}

/*
 * Wrapper function for crackSingleReentrant.
 */
void *crackMultiThreaded(void *vargp) {
    struct args_t *args = vargp;
    crackSingleReentrant(args->lower, args->upper, args->cryptPasswd);
    return NULL;
}

/*
 * Create a thread THREAD that searches all combinations from LOWER index to
 * UPPER index using the function crackMultiThreaded and pin thread to CPU.
 */
void createThread(pthread_t *thread, int lower, int upper, int cpu,
    char *cryptPasswd) {
    if (cracked) return;

    struct args_t *args = (struct args_t*)malloc(sizeof(struct args_t));
    args->lower = lower;
    args->upper = upper;
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

/*
 * Create NUMTHREADS threads in threads[] and partition KEYSPACESIZE search
 * space.
 */
void createThreads(pthread_t threads[], char *cryptPasswd) {
    int offset = KEYSPACESIZE / NUMTHREADS;
    int from, to;
    for (int i = 0; i < NUMTHREADS - 1; i++) {
        from = offset * i;
        to = offset * (i + 1);
        createThread(&threads[i], from, to, i, cryptPasswd);
    }
    from = offset * (NUMTHREADS - 1);
    to = KEYSPACESIZE;
    createThread(&threads[NUMTHREADS - 1], from, to, NUMTHREADS - 1,
        cryptPasswd);
}

/*
 * Join NUMTHREADS threads in threads[]
 */
void joinThreads(pthread_t threads[]) {
    int iret = 0;
    for (int i = 0; i < NUMTHREADS && !cracked; i++) {
       iret = pthread_join(threads[i], NULL);
       if (iret) perror("joinThreads: thread could not join");
    }
}

/*
 * Find the plain-text password PASSWD of length PWLEN for the user USERNAME
 * given the encrypted password CRYPTPASSWD by splitting KEYSPACESIZE search
 * space amongst NUMTHREADS cores.
 */
void crackMulti(char *username, char *cryptPasswd, int pwlen, char *passwd) {
    pthread_t threads[NUMTHREADS];
    createThreads(threads, cryptPasswd);
    joinThreads(threads);
    strcpy(passwd, crackedPassword);
}

/*
 * Parse file in path FNAME into USERS and PASSWDS, returning the COUNT of both.
 */
void parseFile(char *fname, char users[][MAXLEN], char passwds[][HASHLEN + 1],
    int *count) {
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
    crackMulti(username, cryptPasswd, pwlen, passwd);
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

/*
 * Find the plain-text password PASSWD of length PWLEN for the user USERNAME
 * given the encrypted password CRYPTPASSWD without using more than MAXCPU
 * percent of any processor. Uses IPC and a child process to stealthily crack
 * USERNAME's password.
 */
void crackStealthy(char *username, char *cryptPasswd, int pwlen, char *passwd,
    int maxCpu) {
    int pipefd[2];
    int ret;
    ret = pipe(pipefd);
    if (ret < 0) perror("pipe");
    pid_t child = fork();
    if (child < 0) perror("fork");
    if (child == 0) {
        close(READ_END);
        dup2(WRITE_END, STDOUT_FILENO);
        crackMulti(username, cryptPasswd, pwlen, passwd);
        ret = write(WRITE_END, (void *)passwd, PWLEN + 1);
        if (ret < 0) perror("write");
        close(WRITE_END);
    } else {
        close(WRITE_END);
        dup2(READ_END, STDIN_FILENO);
        wait(NULL);
        ret = read(READ_END, (void *)passwd, PWLEN + 1);
        if (ret < 0) perror("read");
        close(READ_END);
    }
}