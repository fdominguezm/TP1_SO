#define _BSD_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h>           /* For O_* constants */
#include <string.h>

#include "shm_definitions.h"

int main (int argc, char *argv[]) {
    // if (argc < 2) printf ("No files to be read\n");

    int fd = shm_open(SHM_NAME, O_CREAT|O_RDWR, S_IRUSR|S_IWUSR);
    if (fd == -1) perror ("shm_open");

    if (ftruncate(fd, SHM_SIZE) == -1) perror("ftruncate");

    void * address = mmap(NULL, SHM_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
    if (address == MAP_FAILED) perror ("mmap");

    //try communication beetween app and view
    char * message = "Hello it works";
    char * map = (char *) address;
    map = strcpy(map, message);

    if (munmap(address, SHM_SIZE) == -1) perror("view: munmap");
    if (shm_unlink(SHM_NAME) == -1) perror("view: shm_unlink");
    if (close(fd) == -1) perror("view: close");

    return 0;
}