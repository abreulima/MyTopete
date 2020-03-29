#ifndef UTILS_H
#define UTILS_H

#include <ctype.h>
#include <ncurses.h>

typedef struct{
    int pid;                            // %d pid
    char comm[512];                     // %s comm
    char state;                         // %c state
    int user;
    unsigned long int utime;            // %lu ctime
    unsigned long int stime;            // %lu utime
    long int priority;                  // %ld priority
    unsigned long long ttime;           // %lu total_time
    float cpuusge;                      // %lu
    unsigned long long int starttime;   // %llu starttime
} processLog;


int isProcess(const char *s);
unsigned long int cputime();
void makeTable(WINDOW *table, int maxx);
void fillTable(WINDOW *table, processLog procList[1024], int maxx);
void readData(processLog procList[1024], int fd[2]);
void frontEnd(processLog procList[1024], int fd[2]);
void WritePipeProcess(processLog procList[1024], int fd[2]);
void ReadPipeProcess(processLog frontEndProcList[15], int fd[2]);
unsigned long int getUpTime();
int compare(const void *a, const void *b);

#endif