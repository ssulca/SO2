//
// Created by bangho on 08/09/17.
//

#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include "buscarArchivo.c"
#define NULL_I 0//como el NULL declarado en las otras funciones es un puntero, molestan los warnings
                //defino NULL_I como un nulo para comparacion de valores de datos
int ejecutar (char *path, char *args[], int flag, int fdin , int fdout);

void findAndExecute(char** args,int concurrentFlag,int fdin , int fdout){
    char p[1024];
    if (buscarArchivo(args[0], p) == 0) {

        args[0]=p;
        ejecutar(p, args,concurrentFlag,fdin ,fdout);

    } else {
        printf("\nno existe\n");
    }
}
int ejecutar (char *path, char *args[], int flag,int fdin , int fdout){
    int pid;
    int status;

    pid = vfork();
    if (pid<0) {
        printf("Fork error \n");
        exit(1);
    }
    if(pid == 0){ /* Child executes here */

        if(fdin!=NULL_I){  //Redireccion de la stdin

            dup2(fdin,STDIN_FILENO);
        }
        if(fdout!=NULL_I){ //Redireccion de la stdout

            dup2(fdout,STDOUT_FILENO);

        }
        execvp(path, args);
        printf("Exec error \n");
        exit(1);
    } else {    /* Parent executes here */
        if(fdin!=NULL_I){
            close(fdin);
        }
        if(fdout!=NULL_I){
            close(fdout);
        }
        if(!flag)
            waitpid(pid,&status,NULL);
        //wait(&status);
    }
    return 0;

}