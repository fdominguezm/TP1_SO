#define _BSD_SOURCE

#ifndef DEFINITIONS_H
#define DEFINITIONS_H
#include <stdio.h>
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
#define SEM_waitView "/sem2"
#define SLAVE_NUM 5
#define BUFFER_SIZE 256

typedef struct slave
{
    pid_t pid;
    int fd_in;
    int fd_out;
}slave;


#endif