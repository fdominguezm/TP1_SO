#include "definitions.h"

int main (int argc, char *argv[]) {
    if (argc < 2){ 
        printf ("No files to be read\n");
        return 1;
    }

    int num_task = argc - 1; //Cantidad de archivos
    int num_slave =  (SLAVE_NUM < num_task)?SLAVE_NUM : num_task; // Cantidad de procesos esclavos

    //Creo los vectores para guardar los datos de los slaves
    slave slaves[SLAVE_NUM];

    FILE *result_file = fopen("fresult.txt", "w"); //Creo el archivo result para enviarselo a los slaves
    if (result_file == NULL) perror("Result File Failed");

    off_t shm_size = num_task * BUFFER_SIZE; //Size de la Shared Memory

    setvbuf(stdout, NULL, _IONBF, BUFFER_SIZE); //Seteo el buffer de la Shared Memory

    sem_t * sem_createShm = sem_open(SEM_CreateShm, O_CREAT|O_RDWR, S_IRUSR|S_IWUSR, 0); //Semaforo para que el proceso vista espere a que se cree la shared memory
    if (sem_createShm == SEM_FAILED) perror ("create shm sem_open");
    sem_t * sem_waitViewToStart = sem_open(SEM_waitView, O_CREAT|O_RDWR, S_IRUSR|S_IWUSR, 0); //Semaforo para esperar a que comience el proceso vista
    if (sem_waitView == SEM_FAILED) perror ("wait view to start sem_open");
    sem_t * sem_waitViewToFinish = sem_open(SEM_waitView, O_CREAT|O_RDWR, S_IRUSR|S_IWUSR, 0); //Semaforo para esperar a que finalice el proceso vista
    if (sem_waitView == SEM_FAILED) perror ("wait view to finish sem_open");


    int fd = shm_open(SHM_NAME, O_CREAT|O_RDWR, S_IRUSR|S_IWUSR);
    if (fd == -1) perror ("shm_open");

    if (ftruncate(fd, shm_size) == -1) perror("ftruncate");

    void * address = mmap(NULL, shm_size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
    if (address == MAP_FAILED) perror ("mmap");

    if (sem_post(sem_createShm) == -1) perror("Create shm sem_post");

    struct timespec * abs_timeout; //Creo la estructura para que espere 2 segundos al proceso vista
    abs_timeout->tv_nsec = 2;
    if (sem_timedwait(sem_waitViewToStart, abs_timeout) == -1) perror("sem_timedwait");
    
    printf("%s %d %d",SHM_NAME, shm_size,num_task); //Envio datos relevantes al proceso vista
    
    create_slaves(slaves, num_slave, argv[1]);//??

    if (sem_wait(sem_waitView) == -1) perror("sem_wait");

    //Unlink a todos los semaforos
    if (sem_unlink(sem_CreateShm) == -1) perror("sem_unlink");
    if (sem_unlink(sem_waitViewToStart) == -1) perror("sem_unlink");
    if (sem_unlink(sem_waitViewToFinish) == -1) perror("sem_unlink");


    if (munmap(address, shm_size) == -1) perror("munmap");
    if (shm_unlink(SHM_NAME) == -1) perror("shm_unlink");
    if (close(fd) == -1) perror("close fd");
    if (fclose(result_file) == -1) perror("close result_file");

    return 0;
}

void create_slaves(slave slaves[], int num_slave,char *const argv[]){
    int fd_send_files[2],fd_receive_buffer[2];
    pid_t pid;

    for (size_t i = 0; i < num_slave; i++)
    {
        if(pipe(fd_send_files) == -1 || pipe(fd_receive_buffer) == -1) perror("create_slaves() pipe");

        if((pid = fork()) == -1) perror("create_slaves() fork");

        if (pid == 0){ // Estoy en el hijo
            close(fd_send_files[1]); // Cierro la entrada del pipe por el que envio los archivos
            close(fd_receive_buffer[0]); // Cierro la salida del pipe por el que se reciben los datos del hijo

            if(dup2(fd_send_files[0],STDIN_FILENO) == -1 || dup2(fd_receive_buffer[1],STDOUT_FILENO) == -1 ) perror("create_slaves() dup2");

            char str[8];
            sprintf(str, "%d", getpid());
            const char *args[] = {str, argv[i],NULL};
            if(execv("./slave",args) == -1) perror("create_slave() execv");

        } else {// Estoy en el padre

            //Copio los datos de cada slave en la estructura
            slaves[i].fd_in = fd_send_files[1];
            slaves[i].fd_out = fd_receive_buffer[0];
            slaves[i].pid = pid;
        }
        
    }
    


}

void read_and_send_files(slave slaves[], int num_task,int num_slaves, char *const argv[]){
    int finished_tasks = 0;
    char buffer[BUFFER_SIZE];

    while (finished_tasks < num_task)
    {
        fd_set read_fdSet;

        FD_ZERO(&read_fdSet);

        int max = 0;

        for(int i = 0; i < num_slaves;i++ ){

            FD_SET(slaves[i].fd_out,&read_fdSet);
            if (slaves[i].fd_out > max)
            {
                max = slaves[i].fd_out;
            }
        }

        if( select(max + 1, &read_fdSet, NULL, NULL,NULL) == -1) perror("read_and_send_files() select");


        for(int i = 0; i < num_slaves;i++ ){

            if(FD_ISSET(slaves[i].fd_out,&read_fdSet)){
                read(slaves[i].fd_out, buffer,BUFFER_SIZE); 
                /*seguir con:
                        -pasar buffer a shared memory
                        -pasar buffer a archivo
                        -enviarle nuevo file al slave
                 */
                
            }
        }
    }
    
}