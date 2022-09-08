#define _BSD_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h>           /* For O_* constants */
#include <string.h>
#include <semaphore.h>

#define SHM_NAME "/myshm"
#define SHM_SIZE 1024
#define VALUE 10
#define SEM_CreateShm "/sem1"
#define SEM_waitView "/sem2"