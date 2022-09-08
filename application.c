#include "shm_definitions.h"

int main (int argc, char *argv[]) {
    // if (argc < 2) printf ("No files to be read\n");

    sem_t * sem_createShm = sem_open(SEM_CreateShm, O_CREAT|O_RDWR, S_IRUSR|S_IWUSR, 0);
    if (sem_createShm == SEM_FAILED) perror ("sem_open");

    sem_t * sem_waitView = sem_open(SEM_waitView, O_CREAT|O_RDWR, S_IRUSR|S_IWUSR, 0);
    if (sem_waitView == SEM_FAILED) perror ("sem_open");

    int fd = shm_open(SHM_NAME, O_CREAT|O_RDWR, S_IRUSR|S_IWUSR);
    if (fd == -1) perror ("shm_open");

    if (ftruncate(fd, SHM_SIZE) == -1) perror("ftruncate");

    void * address = mmap(NULL, SHM_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
    if (address == MAP_FAILED) perror ("mmap");

    if (sem_post(sem_createShm) == -1) perror("sem_post");

    sleep(4);
    
    //try communication beetween app and view
    char * message = "Hello it works";
    char * map = (char *) address;
    map = strcpy(map, message);

    if (sem_wait(sem_waitView) == -1) perror("sem_wait");

    if (sem_unlink(SEM_CreateShm) == -1) perror("sem_unlink");
    if (sem_unlink(SEM_waitView) == -1) perror("sem_unlink");

    if (munmap(address, SHM_SIZE) == -1) perror("munmap");
    if (shm_unlink(SHM_NAME) == -1) perror("shm_unlink");
    if (close(fd) == -1) perror("close");

    return 0;
}