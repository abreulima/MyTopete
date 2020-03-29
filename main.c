#include <stdio.h>
#include <ncurses.h>
#include <dirent.h>
#include <string.h>
#include <sys/types.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <time.h>
#include "utils.h"

int main(){

    srand(time(NULL));

    /* Abre Pipes */
    int pipeStructProcessList[2];
    pipe(pipeStructProcessList);
    /* Fecha Pipe */

    pid_t p;

    processLog procList[1024];
    processLog frontEndProcList[15];

    p = fork();

    if(p == 0){
        readData(procList, pipeStructProcessList);
    }   
    else{
        frontEnd(frontEndProcList, pipeStructProcessList);
    }    

	return 0;

};