#include "shm_definitions.h"

void clearBuffer(char * buffer) {
    int i = 0;
    while(buffer[i] != 0 && i<256){
        buffer[i] = 0;
        i++;
    }
}

//argv[1] = slave pid
//argv[2] = file_name
int main (int argc, char *argv[]) {
    if (argc < 2) perror("slave: not enough arguments");
    
    char buffer [256]= {0};
    char command [256] = {0};
    char * s = strcat(buffer, "md5sum ");
    s = strcat(buffer, argv[2]);
    FILE * md5 = popen(s, "r");
    if (md5 == NULL) perror("slave: md5");
    if((s = fgets(buffer, 256, md5)) == NULL) perror ("slave: read md5");
    printf("%s %s", argv[1], buffer);

    clearBuffer(buffer);
    int size;
    while ((size = read(STDIN_FILENO, buffer, 256)) > 1) {
        buffer[size-1] = 0; //remove '\n'
        s = strcat(command, "md5sum ");
        s = strcat(command, buffer);
        FILE * md5 = popen(s, "r");
        if (md5 == NULL) perror("slave: md5");
        if((s = fgets(buffer, 256, md5)) == NULL) perror ("slave: read md5");
        printf("%s %s", argv[1], buffer);
        clearBuffer(buffer);
        clearBuffer(command);
    }
    
    
    return 0;
}