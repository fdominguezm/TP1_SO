#include "definitions.h"

void clearBuffer(char * buffer) {
    int i = 0;
    while(buffer[i] != 0 && i<256){
        buffer[i] = 0;
        i++;
    }
}

//argv[0] = slave pid
//argv[1] = file_name
int main (int argc, char *argv[]) {
    if (argc < 1) perror("slave: not enough arguments");
    
    char buffer [256]= {0};
    char command [256] = {0};
    char * s;
    int size;
    FILE * md5;

    s = strcat(command, "md5sum ");
    s = strcat(command, argv[1]);
    md5 = popen(s, "r");
    if (md5 == NULL) perror("slave: md5");
    if((s = fgets(buffer, 256, md5)) == NULL) perror ("slave: read md5");
    write(STDOUT_FILENO, buffer, BUFFER_SIZE);
    clearBuffer(buffer);
    clearBuffer(command);
    pclose(md5);

    while ((size = read(STDIN_FILENO, buffer, 256)) > 1) {
        // buffer[size-1] = 0; //remove '\n'
        s = strcat(command, "md5sum ");
        s = strcat(command, buffer);
        FILE * md5 = popen(s, "r");
        if (md5 == NULL) perror("slave: md5");
        if((s = fgets(buffer, 256, md5)) == NULL) perror ("slave: read md5");
        write(STDOUT_FILENO, buffer, BUFFER_SIZE);
        clearBuffer(buffer);
        clearBuffer(command);
    }
    
    
    return 0;
}