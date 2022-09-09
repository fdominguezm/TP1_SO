#include "shm_definitions.h"

int main (int argc, char *argv[]) {

    sem_t * sem_createShm = sem_open(SEM_CreateShm, O_CREAT|O_RDWR, S_IRUSR|S_IWUSR, 0); //Semaforo para que el proceso vista espere a que se cree la shared memory
    if (sem_createShm == SEM_FAILED) perror ("view: sem_open");
    sem_t * sem_waitViewToStart = sem_open(SEM_waitView, O_CREAT|O_RDWR, S_IRUSR|S_IWUSR, 0); //Semaforo para esperar a que comience el proceso vista
    if (sem_waitView == SEM_FAILED) perror ("sem_open");
    sem_t * sem_waitView = sem_open(SEM_waitView, O_CREAT|O_RDWR, S_IRUSR|S_IWUSR, 0); //Semaforo para esperar a que finalice el proceso vista
    if (sem_waitView == SEM_FAILED) perror ("view: sem_open");

    if (sem_post(sem_waitViewToStart) == -1) perror("View to start sem_post");
    

    if (sem_wait(sem_createShm) == -1) perror ("view: sem_wait");

    int fd = shm_open(SHM_NAME, O_RDWR, 0);
    if (fd == -1) perror ("view: shm_open");

    if (ftruncate(fd, SHM_SIZE) == -1) perror("view: ftruncate");

    void * address = mmap(NULL, SHM_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
    if (address == MAP_FAILED) perror ("view: mmap");

    //recieve message deployed by app
    char * map = (char *) address;
    while (*map == 0) {
        printf(".\n");
        sleep(1);
    }
    printf("Message recieved: %s\n", map);

    if (sem_post(sem_waitView) == -1) perror("View: sem_post");

    // if (munmap(address, SHM_SIZE) == -1) perror("view: munmap");
    // if (shm_unlink(SHM_NAME) == -1) perror("view: shm_unlink");
    // if (close(fd) == -1) perror("view: close");

    return 0;
}