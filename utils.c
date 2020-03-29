#include "utils.h"
#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>
#include <dirent.h>
#include <unistd.h>
#include <time.h>

void strcat_c(char *str, char c)  {
    for (;*str;str++);
    *str++ = c; 
    *str++ = 0;
}

int isProcess(const char *s){
    while(*s){
        if (isdigit(*s++) == 0) return 0;
    }
    return 1;
}

unsigned long int cputime(){

    FILE *cpuTimeFile;
    char *procstat = "/proc/stat";

    char buffer[1024];
    cpuTimeFile = fopen(procstat, "r");
    fscanf(cpuTimeFile, "%[^\n]", buffer);

    char temp[1024] = "";
    unsigned long int sum = 0;
    int length = (int)strlen(buffer);
    int i = 0;

    for (i = 0; i < length; i++){

        if (isdigit(buffer[i])){
            strcat_c(temp, buffer[i]);
        }

        else{
            sum = sum + atoi(temp);
            temp[0] = '\0';
        }
    }

    return sum;
}

void makeTable(WINDOW *table, int maxx){

    char columns[7][24] = {"PID", "PR", "User", "State", "% CPU", "Time", "Command"};
    int spaceColumn = maxx/7;

    wmove(table, 2, 0);
    whline(table, ACS_HLINE, maxx);
    //wmove(table, 3, 0);

    wattrset(table, COLOR_PAIR(1));
    for(int i = 0; i < 7; i++){
        mvwprintw(table, 1, 1 + i*spaceColumn, "%-*s", spaceColumn - 1, columns + i); 
    }
    wattrset(table, COLOR_PAIR(2));
}

void fillTable(WINDOW *table, processLog procList[15], int maxx){

    char columns[7][24] = {"PID", "PR", "User", "State", "% CPU", "Time", "Command"};
    int spaceColumn = maxx/7;

    int maxProcess = 15;

    for(int i = 0; i < 15; i++){
        
        processLog aux = procList[i];

        int ofy = 3;


        mvwprintw(table, i + ofy, 1 + 0*spaceColumn, "%d", aux.pid);        // PID
        mvwprintw(table, i + ofy, 1 + 1*spaceColumn, "%ld",aux.priority);   // PR
        mvwprintw(table, i + ofy, 1 + 2*spaceColumn, "%d", aux.user);        // USER
        mvwprintw(table, i + ofy, 1 + 3*spaceColumn, "%c", aux.state);      // State
        mvwprintw(table, i + ofy, 1 + 4*spaceColumn, "%lf",aux.cpuusge);    // CPU Usage
        mvwprintw(table, i + ofy, 1 + 5*spaceColumn, "%llu",aux.starttime); // Startime
        mvwprintw(table, i + ofy, 1 + 6*spaceColumn, "%-*s", spaceColumn, aux.comm);       // Comm
        //mvwprintw(table, i, i, "%d", rand());
    }
}

void readData(processLog procList[1024], int fd[2]){
    
    DIR *d;

    struct dirent *dir;
    int k = 0;

    if ((d = opendir("/proc")) == NULL){
        exit(1);
    }

    while(1){
        while((dir = readdir(d)) !=NULL ){
            /* Bloco de cada processo */
            if (isProcess(dir->d_name)){
                
                FILE *statFile;
                processLog p;

                char out[1024] = "";
                strcat(out, "/proc/");
                strcat(out, dir->d_name);
                strcat(out, "/stat");
            
                statFile = fopen(out, "r");

                if(statFile == NULL){
                    continue;
                }

                char *args =    "%d %s %c " // pid, comm, state
                                    "%*d %*d %*d %*d %*d %*u %*lu %*lu %*lu %*lu " // ppid, pgrp, session, tty_nr, tpgid, flags, minflt, cminflt, majflt, majflt
                                    "%lu %lu " // utime, stime
                                    "%*ld %*ld " // cutime, cstime
                                    "%ld" // priority 
                                    "%*ld %*ld %*ld" // nice, num_threads, itrealvalue
                                    "%llu"; // starttime
                    
                fscanf(statFile, 
                args, 
                &(p.pid),
                p.comm, 
                &(p.state),
                &(p.utime),
                &(p.stime),
                &(p.priority),
                &(p.starttime)
                );

                unsigned long int HERTZ = sysconf(_SC_CLK_TCK);
                unsigned long int uptime = getUpTime();
                unsigned long int totalTime = p.stime + p.utime;
                float seconds = uptime - (p.starttime/HERTZ);
                p.cpuusge = 100 * ((totalTime/HERTZ) / seconds);
                p.user = rand() % 100;
                procList[k] = p;
                k++;

                fclose(statFile);
            }
            /* Bloco de cada processo */
        }

    qsort(procList, k, sizeof(processLog), compare);
    WritePipeProcess(procList, fd);
    k = 0;
    rewinddir(d);
    sleep(1.5);
    }


}

void WritePipeProcess(processLog procList[1024], int fd[2]){

    //close(fd[0]);
    write(fd[1], procList, 15 * sizeof(processLog));
    //close(fd[1]);
     
}

void ReadPipeProcess(processLog frontEndProcList[15], int fd[2]){
    
    //close(fd[1]);
    read(fd[0], frontEndProcList, 15 * sizeof(processLog));
    //fflush(stdout);
    //close(fd[0]);
}

void frontEnd(processLog frontEndProcList[15], int fd[2]){
     
    char ch;

    WINDOW *header,*table;
    
    initscr();
    noecho();
    curs_set(0);
    use_default_colors();
    start_color();
    init_pair(1, COLOR_CYAN, -1);
    init_pair(2, -1, -1);
    refresh();

    int maxx, maxy;
    getmaxyx(stdscr,maxy,maxx);
    int spaceColumn = (int)maxx/7;
    
    if ((header = newwin(8,maxx,0,0)) == NULL) exit(1);
    if ((table  = newwin(19,maxx,8,0)) == NULL) exit(1);

    wmove(header, 1, 0);   
    wprintw(header, "  __  __     _____              _       \n");
    wprintw(header, " |  \\/  |_  |_   _|__ _ __  ___| |_ ___ \n");
    wprintw(header, " | |\\/| | || || |/ _ \\ '_ \\/ -_)  _/ -_)\n");
    wprintw(header, " |_|  |_|\\_, ||_|\\___/ .__/\\___|\\__\\___|\n");
    wprintw(header, "         |__/        |_|                \n");
    //box(header, 0, 0);
    wrefresh(header);
    

    makeTable(table, maxx);
    
    for(;;){
        ReadPipeProcess(frontEndProcList, fd);
        fillTable(table, frontEndProcList, maxx);
        box(table, 0,0);
        //wclear(table);
        wrefresh(table);
        sleep(1.5);
    }

    while((ch = getch()) != 'q');
    endwin();
    
}

unsigned long int getUpTime(){
    
    unsigned long int aux;

    FILE *uptime;
    uptime = fopen("/proc/uptime", "r");
    fscanf(uptime, "%lu", &aux);
    fclose(uptime);

    return aux;
}

int compare(const void *a, const void *b) {
    float c = ((processLog *) a)->cpuusge;
    float d = ((processLog *) b)->cpuusge;

    if(c > d) return -1;
    if(c < d) return 1;
    return 0;
}