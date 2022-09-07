#define _BSD_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h>           /* For O_* constants */

#include "shm_definitions.h"

int main (int argc, char *argv[]) {
    if (argc < 2) printf ("No files to be read\n");

    int fd = shm_open(SHM_NAME, O_RDWR, 0);
    if (fd == -1) perror ("view: shm_open");

    if (ftruncate(fd, SHM_SIZE) == -1) perror("ftruncate");

    void * address = mmap(NULL, SHM_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
    if (address == MAP_FAILED) perror ("view: mmap");

}