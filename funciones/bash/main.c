#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
char * argv[] = {"/home/sergio/CLionProjects/OSystem/TP1SO_Socket/Server/bash", 0};
int main()
{
    int pid, status;


    if ( (pid = fork() ) < 0 )
    {
        printf("Fork error \n");
        exit(1);
    }
    if(pid == 0)	 {
        execv(argv[0], argv);
        printf("Exec error \n");
        exit(1);
    } else
        wait(&status);
    printf("Hello there! \n");
    return 0;
}