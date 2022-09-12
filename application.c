#include "definitions.h"

void read_and_send_files(slave slaves[], int num_task,int num_slaves, char *shm_p, FILE * result_file, char *const argv[], sem_t * sem_newFile);
void create_slaves(slave slaves[], int num_slave,char *const argv[]);

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

    int shm_size = num_task * BUFFER_SIZE; //Size de la Shared Memory

    sem_t * sem_createShm = sem_open(SEM_CreateShm, O_CREAT|O_RDWR, S_IRUSR|S_IWUSR, 0); //Semaforo para que el proceso vista espere a que se cree la shared memory
    if (sem_createShm == SEM_FAILED) perror ("create shm sem_open");
    sem_t * sem_waitViewToStart = sem_open(SEM_waitViewToStart, O_CREAT|O_RDWR, S_IRUSR|S_IWUSR, 0); //Semaforo para esperar a que comience el proceso vista
    if (sem_waitViewToStart == SEM_FAILED) perror ("wait view to finish sem_open");
    sem_t * sem_waitViewToFinish = sem_open(SEM_waitViewToFinish, O_CREAT|O_RDWR, S_IRUSR|S_IWUSR, 0); //Semaforo para esperar a que finalice el proceso vista
    if (sem_waitViewToFinish == SEM_FAILED) perror ("wait view to finish sem_open");
    sem_t * sem_newFile = sem_open(SEM_newFile, O_CREAT|O_RDWR, S_IRUSR|S_IWUSR, 0); //Semaforo para comunicarle al proceso viste que se escribio un nuevo archivo en la SHM
    if (sem_newFile == SEM_FAILED) perror ("newFile sem_open");

    //Abro y configuro la SHM
    int fd = shm_open(SHM_NAME, O_CREAT|O_RDWR, S_IRUSR|S_IWUSR);
    if (fd == -1) perror ("shm_open");

    if (ftruncate(fd, (off_t)shm_size) == -1) perror("ftruncate");

    void * address = mmap(NULL, shm_size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
    if (address == MAP_FAILED) perror ("mmap");
    char * shm_p = (char *) address;

    // Le aviso al Vista que ya se creo la SHM
    if (sem_post(sem_createShm) == -1) perror("View to start sem_post");    
    
    //Envio datos relevantes al proceso vista
    char buff[8] = {0};
    sprintf(buff,"%d\n",num_task); 
    write(STDOUT_FILENO, buff, 8);

    sleep(2); //Espero a que comience el proceso vista

    create_slaves(slaves, num_slave, argv); //Creo los esclavos
    read_and_send_files(slaves, num_task, num_slave, shm_p, result_file, argv, sem_newFile); // Proceso todos los archivos y se los envio a los esclavos

    // Me fijo si empezo el vista para ver si lo tengo que esperar a que termine
    int value;
    sem_getvalue(sem_waitViewToStart, &value);
    if (value) {
        if (sem_wait(sem_waitViewToFinish) == -1) perror("sem_wait");
    } else {
        printf("View did not start\n");
    }


    // //Unlink a todos los semaforos
    if (sem_unlink(SEM_CreateShm) == -1) perror("sem_unlink");
    if (sem_unlink(SEM_waitViewToStart) == -1) perror("sem_unlink");
    if (sem_unlink(SEM_waitViewToFinish) == -1) perror("sem_unlink");
    if (sem_unlink(SEM_newFile) == -1) perror("sem_unlink");

    //Cierro los semaforos
    if (sem_close(sem_createShm) == -1) perror("sem_unlink");
    if (sem_close(sem_waitViewToFinish) == -1) perror("sem_unlink");
    if (sem_close(sem_waitViewToStart) == -1) perror("sem_unlink");
    if (sem_close(sem_newFile) == -1) perror("sem_unlink");

    //Termino de trabajar con la SHM y cierro el result_file
    if (munmap(address, shm_size) == -1) perror("munmap");
    if (shm_unlink(SHM_NAME) == -1) perror("shm_unlink");
    if (close(fd) == -1) perror("close fd");
    if (fclose(result_file) == -1) perror("close result_file");

    return 0;
}

void create_slaves(slave slaves[], int num_slave,char *const argv[]){
    int fd_send_files[2],fd_receive_buffer[2];
    pid_t pid;

    for (int i = 0; i < num_slave; i++) {
        
        if(pipe(fd_send_files) == -1 || pipe(fd_receive_buffer) == -1) perror("create_slaves() pipe");

        if((pid = fork()) == -1) perror("create_slaves() fork");

        if (pid == 0){ // Estoy en el hijo
            close(fd_send_files[STDOUT_FILENO]); // Cierro la entrada del pipe por el que envio los archivos
            close(fd_receive_buffer[STDIN_FILENO]); // Cierro la salida del pipe por el que se reciben los datos del hijo

            if(dup2(fd_send_files[STDIN_FILENO],STDIN_FILENO) == -1 || dup2(fd_receive_buffer[STDOUT_FILENO],STDOUT_FILENO) == -1 ) perror("create_slaves() dup2");

            char str[8];
            sprintf(str, "%d", getpid());
            char * args[] = {str, argv[i+1],NULL};
            if(execv("./slave",args) == -1) perror("create_slave() execv");

            return;

        } else {// Estoy en el padre

            //Copio los datos de cada slave en la estructura
            slaves[i].fd_in = fd_send_files[1];
            slaves[i].fd_out = fd_receive_buffer[0];
            slaves[i].pid = pid;
            slaves[i].active = 1;
        }
        
    }
}

void read_and_send_files(slave slaves[], int num_task,int num_slaves, char *shm_p, FILE * result_file, char *const argv[], sem_t *sem_newFile){
    int finished_tasks = 0, tasks_to_send = num_slaves + 1;
    char buffer[BUFFER_SIZE];

    //Itero por las tareas
    while (finished_tasks < num_task) {
        fd_set read_fdSet;
        FD_ZERO(&read_fdSet);
        int max = 0;

        //Cargo los sets para el select
        for(int i = 0; i < num_slaves; i++){
            if (slaves[i].active) {
                FD_SET(slaves[i].fd_out,&read_fdSet);
                if (slaves[i].fd_out > max) {
                    max = slaves[i].fd_out;
                }
            }
        }

        if( select(max + 1, &read_fdSet, NULL, NULL,NULL) == -1) perror("read_and_send_files() select");

        //Me fijo que fd esta para leer, en tal caso lo leo y le envio otro archivo al esclavo
        for(int i = 0; i < num_slaves;i++ ) {
            if(slaves[i].active && FD_ISSET(slaves[i].fd_out,&read_fdSet)){
                read(slaves[i].fd_out, buffer, BUFFER_SIZE); 
                fprintf(result_file,"%d %s",slaves[i].pid, buffer);
                int position = (finished_tasks)*BUFFER_SIZE;
                char str[8];
                sprintf(str, "%d", slaves[i].pid);
                strcat((shm_p+position), str);
                strcat((shm_p+position), " ");
                strcat((shm_p+position), buffer);
                sem_post(sem_newFile);     

                if (tasks_to_send <= num_task) {
                    write(slaves[i].fd_in, argv[tasks_to_send], strlen(argv[tasks_to_send]));
                } else {
                    char c = EOF;
                    write(slaves[i].fd_in, &c,1);
                    slaves[i].active = 0;
                }
                finished_tasks++;
                tasks_to_send++;
            }
        }
    }
    
}