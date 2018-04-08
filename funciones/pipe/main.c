#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <fcntl.h>
#define NULL_I 0//como el NULL declarado en las otras funciones es un puntero, molestan los warnings
#define SIZE 512

int ejecutar (char *path, char *args[], int flag,int fdin , int fdout);


int main( int argc, char **argv ) {
    pid_t pid;
    int tob[2], formb[2], readbytes;
    char buffer[SIZE];
    char * argv_h[] = {"/home/sergio/CLionProjects/OSystem/TP1SO_Socket/Server/bash", 0};

    pipe( tob );
    pipe( formb );

    //int flags = fcntl(formb[0], F_GETFL, 0);
    //if(fcntl(formb[0], F_SETFL, flags | O_NONBLOCK))


    pid = vfork();
    if (pid<0) {
        printf("Fork error \n");
        exit(1);
    }
    if (pid == 0 )
    { // hijo
        close( tob[1] ); /* cerramos el lado de escritura del pipe */
        close( formb[0] ); /* cerramos el lado de lectura del pipe */

        //readbytes = read( tob[0], buffer, SIZE );
        dup2(tob[0],STDIN_FILENO);
        dup2(formb[1],STDOUT_FILENO);
        //while( (readbytes = read( tob[0], buffer, SIZE ) ) > 0)
            //write( 1, buffer, readbytes );
        execvp(argv_h[0], argv_h);
        printf("Exec error \n");
        close( tob[0] );
        //strcpy( buffer, "Soy tu hijo hablandote por la otra tuberia.\n" );
        //write( formb[1], buffer, strlen( buffer ) );
        close( formb[1] );
        exit(1);
    }
    else
    { // padre
        close( tob[0] ); /* cerramos el lado de lectura del pipe */
        close( formb[1] ); /* cerramos el lado de escritura del pipe */
        while(1){

            if((readbytes = read(formb[0], buffer, SIZE))>0){
                write(STDOUT_FILENO, buffer, readbytes);
            }

            fgets(buffer,SIZE, stdin);
            write(tob[1], buffer, strlen(buffer));
            if (strstr(buffer, "exit") != NULL) {
                break;
            }


            /*while( (readbytes= read( formb[0], buffer, SIZE )) > 0) {
                write( STDOUT_FILENO, buffer, readbytes );
            }
            printf("padre fin\n");
            */
        }

        //write( tob[1], buffer, strlen( buffer ) );
        close( tob[1]);
        //while( (readbytes= read( formb[0], buffer, SIZE )) > 0)
        //   write( 1, buffer, readbytes );
        close( formb[0]);
    }
    waitpid( pid, NULL, 0 );
    return 0;
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

