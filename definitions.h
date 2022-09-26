#ifndef DEFINITIONS_H
#define DEFINITIONS_H

#define _BSD_SOURCE
#define _POSIX_C_SOURCE 200112L
#define _XOPEN_SOURCE 600

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h>           /* For O_* constants */
#include <string.h>
#include <semaphore.h>
#include <sys/select.h>

#define SHM_NAME "/myshm"
#define SHM_SIZE 1024
#define VALUE 10
#define SEM_CreateShm "/sem1"
#define SEM_newFile "/sem2"
#define SEM_waitViewToFinish "/sem3"
#define SEM_waitViewToStart "/sem4"
#define SLAVE_NUM 5
#define BUFFER_SIZE 256
#define N_PIPE "/mypipe"

typedef struct slave
{
    pid_t pid;
    int fd_in;
    int fd_out;
    int active;
}slave;


#endif