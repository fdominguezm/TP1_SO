#include "definitions.h"

int main (int argc, char *argv[]) {
    
    int shm_size;
    int num_task;

    if (argc == 1) {
        char buff[BUFFER_SIZE] = {0};
        if(read(STDIN_FILENO, buff, BUFFER_SIZE) == -1) {
            perror("Error reading data");
        }
        num_task = atoi(buff);
    } else if (argc == 2) {
        num_task = atoi(argv[1]);
    } else {
        perror("Error: invalid amount of arguments");
    }

    shm_size = num_task*BUFFER_SIZE;

    if(shm_size <= 0) {
        perror("Error defining size of shared memory");
    }    
    

    //Creo los semaforos necesarios
    sem_t * sem_createShm = sem_open(SEM_CreateShm, O_CREAT|O_RDWR, S_IRUSR|S_IWUSR, 0); //Semaforo para que el proceso vista espere a que se cree la shared memory
    if (sem_createShm == SEM_FAILED) perror ("view: sem_open");
    sem_t * sem_waitViewToStart = sem_open(SEM_waitViewToStart, O_CREAT|O_RDWR, S_IRUSR|S_IWUSR, 0); //Semaforo para esperar a que comience el proceso vista
    if (sem_waitViewToStart == SEM_FAILED) perror ("sem_open");
    sem_t * sem_waitViewToFinish = sem_open(SEM_waitViewToFinish, O_CREAT|O_RDWR, S_IRUSR|S_IWUSR, 0); //Semaforo para esperar a que finalice el proceso vista
    if (sem_waitViewToFinish == SEM_FAILED) perror ("view: sem_open");
    sem_t * sem_newFile = sem_open(SEM_newFile, O_CREAT|O_RDWR, S_IRUSR|S_IWUSR, 0); //Semaforo para saber que se escribio en la shared memory
    if (sem_newFile == SEM_FAILED) perror ("VIEW.C: newFile sem_open");

    //Aviso que empezo el proceso vista 
    if (sem_post(sem_waitViewToStart) == -1) perror("View to start sem_post");

    //Espero a la creacion de la SHM
    if (sem_wait(sem_createShm) == -1) perror("VIEW.C: sem_createSHM wait");
    
    //Abro la shared memory y la configuro
    // int fd = shm_open(SHM_NAME, O_RDWR, 0);
    // if (fd == -1) perror ("VIEW.C: shm_open");

    // if (ftruncate(fd, (off_t)shm_size) == -1) perror("VIEW.C: ftruncate");

    // void * address = mmap(NULL, shm_size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
    // if (address == MAP_FAILED) perror ("VIEW.C: mmap");
    // char * shm_p = (char *) address;

    // if (mkfifo(N_PIPE, S_IRUSR) == -1) perror ("view: n_pipe create");
    
    int n_pipe = open(N_PIPE,  O_RDONLY); //Creo el archivo result para enviarselo a los slaves
    if (n_pipe == -1) perror("Result File Failed");
    
    int n = 0;
    char s [BUFFER_SIZE] = {0};
    //Leo las entradas de la SHM
    for (size_t i = 0; i < num_task; i++){
        // sem_wait(sem_newFile);
        // int position = (i)*BUFFER_SIZE;
        // printf("%s\n",(shm_p+position));
        // n = read(n_pipe, s, BUFFER_SIZE);
        // perror("justo antes del read");
        n = read (n_pipe, s ,BUFFER_SIZE);
        printf("%s", s);
        for (int j = 0; j<BUFFER_SIZE; j++) {
            s[j]=0;
        }
    }

    if (close(n_pipe) == -1) perror("view: close n pipe");


    //Aviso que termino el proceso vista
    if (sem_post(sem_waitViewToFinish) == -1) perror("View: sem_post");

    // if (munmap(address, shm_size) == -1) perror("view: munmap");

    return 0;
}