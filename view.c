#include "definitions.h"

int main (int argc, char *argv[]) {
    char *shm_name = {0};
    int shm_size;
    int num_task;
    char buffer[BUFFER_SIZE];

    if (!setvbuf(STDIN_FILENO, NULL, _IONBF, BUFFER_SIZE)) perror("SetvBuf");

    if(argc < 2){
        scanf("%s %d %d\n", shm_name,&shm_size, &num_task);
        
    }else {
        if(argc == 4){
            shm_name = argv[1];
            shm_size = atoi(argv[2]);
            num_task = atoi(argv[3]);
        }else{
            printf("view.c: Error in arguments");
            return 1;
        }
    }
    // if (argc == 1) {
    //     char buff[MEM_LEN] = {0};
    //     if(read(STDIN, buff, MEM_LEN) == ERROR_CODE) {
    //         errorHandler("Error reading data");
    //     }
    //     smSize = atoi(buff);
    // } else if (argc == 2) {
    //     smSize = atoi(argv[1]);
    // } else {
    //     errorHandler("Error: invalid amount of arguments");
    // }

    // if(smSize <= 0) {
    //     errorHandler("Error defining size of shared memory");
    // }
    printf("%s %d %d\n", shm_name, shm_size, num_task);
    
    

    sem_t * sem_newFile = sem_open(SEM_newFile, O_RDWR, S_IRUSR|S_IWUSR, 0); //Semaforo para saber que se escribio en la shared memory
    if (sem_newFile == SEM_FAILED) perror ("newFile sem_open");

    //Abro la shared memory
    int fd = shm_open(shm_name, O_RDWR, S_IRUSR|S_IWUSR);
    if (fd == -1) perror ("shm_open");

    if (ftruncate(fd, (off_t)shm_size) == -1) perror("ftruncate");

    void * address = mmap(NULL, shm_size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
    if (address == MAP_FAILED) perror ("mmap");
    char * shm_p = (char *) address;


    for (size_t i = 0; i < num_task; i++)
    {
        sem_wait(sem_newFile);
        int position = (i)*BUFFER_SIZE;
        strcpy(buffer, (shm_p+position));
        printf("%s\n",buffer);
    }
    
    
    // sem_t * sem_createShm = sem_open(SEM_CreateShm, O_CREAT|O_RDWR, S_IRUSR|S_IWUSR, 0); //Semaforo para que el proceso vista espere a que se cree la shared memory
    // if (sem_createShm == SEM_FAILED) perror ("view: sem_open");
    // sem_t * sem_waitViewToStart = sem_open(SEM_waitViewToStart, O_CREAT|O_RDWR, S_IRUSR|S_IWUSR, 0); //Semaforo para esperar a que comience el proceso vista
    // if (sem_waitViewToStart == SEM_FAILED) perror ("sem_open");
    // sem_t * sem_waitViewToFinish = sem_open(SEM_waitViewToFinish, O_CREAT|O_RDWR, S_IRUSR|S_IWUSR, 0); //Semaforo para esperar a que finalice el proceso vista
    // if (sem_waitViewToFinish == SEM_FAILED) perror ("view: sem_open");

    // if (sem_post(sem_waitViewToStart) == -1) perror("View to start sem_post");
    

    // if (sem_wait(sem_createShm) == -1) perror ("view: sem_wait");

    // int fd = shm_open(SHM_NAME, O_RDWR, 0);
    // if (fd == -1) perror ("view: shm_open");

    // if (ftruncate(fd, SHM_SIZE) == -1) perror("view: ftruncate");

    // void * address = mmap(NULL, SHM_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
    // if (address == MAP_FAILED) perror ("view: mmap");

    // //recieve message deployed by app
    // char * map = (char *) address;
    // while (*map == 0) {
    //     printf(".\n");
    //     sleep(1);
    // }
    // printf("Message recieved: %s\n", map);

    // if (sem_post(sem_waitViewToFinish) == -1) perror("View: sem_post");

    // // if (munmap(address, SHM_SIZE) == -1) perror("view: munmap");
    // // if (shm_unlink(SHM_NAME) == -1) perror("view: shm_unlink");
    // // if (close(fd) == -1) perror("view: close");

    return 0;
}