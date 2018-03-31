#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pwd.h>
#include <unistd.h>
#include "funciones/cd.c"
#include "funciones/execute.c"
#include "funciones/ComandosInternos.c"
#include "funciones/findRedirectCommand.c"
#define BUFSIZE 1024
#define SEG_DELIMITADOR " \n"
#define NULL_I 0 //como el NULL declarado en las otras funciones es un puntero, molestan los warnings
                //defino NULL_I como un nulo para comparacion de valores de datos

//Ver la funcion dup2() para redirigir la stdin y stdout
//usar los flags WarningON, y cppcheck


void SIGCHLDHandler(int s);

int main() {
    char input[BUFSIZE];
    char *args[20];
    char hostname[BUFSIZE];// buffer para el  Pc name

    gethostname(hostname, BUFSIZE-1);//get PC name

    signal(SIGCHLD,SIGCHLDHandler); // Asigna un Handler para la señal de terminacion de procesos hijos, permite
                                    //permite limpiar los procesos zombies "dinamicamente"

    while(1) {
        printf("\033[33m%s@%s \033[34m%s $ \033[37m", getpwuid(geteuid())->pw_name, hostname, getcwd(NULL, 0));

        fgets(input, BUFSIZE, stdin);// pide el comando. se hace con fgets xq permite leer la linea completa
        input[strlen(input) - 1] = NULL_I;//como el string se toma desde archivo, se debe borrar el ultimo caracter(\n)
                                        // xq trae problemas para la lectura de los argumentos

        char *comands[]={NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};//comandos divididos por ampersand
        int concurrentFlag=0;//indica si los procesos se ejecutaran concurrentemente;
        if(strstr(input," &")!='\0'){
            concurrentFlag=1;
        }
        comands[0]=strtok_r(input, "&", comands+1);
        int index=0;


        do {
            char* pipes[10]={NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};//comandos divididos por ampersand
            int pipePrevio=NULL_I;//indica si el ultimo comando genero un pipe,
                                // de ser cierto contiene el descriptor de la salida del pipe

            pipes[0]=strtok_r(comands[index], "|",pipes+1);
            int pipeIndex=0;

            do {
                int fdin=NULL_I;
                int fdout =NULL_I;
                if(pipePrevio!=NULL_I){
                    fdin=pipePrevio;
                    pipePrevio=NULL_I;
                }
                if(pipes[pipeIndex+1]!=NULL && *pipes[pipeIndex+1]!='\0'){ // si existe un siguiente comando luego del operador pipe,
                    int fd[2];                                              //crea un pipe y cambia la stdout del proceso actual
                    pipe(fd);                                               //y guarda la salida del pipe para le proceso posterior
                    fdout=fd[1];
                    pipePrevio=fd[0];
                    concurrentFlag=0;
                }

                findRedirectCommand(pipes[index], &fdin, &fdout); //verifica si hay redireccoin de stdout y std in


                args[0] = strtok(pipes[pipeIndex], SEG_DELIMITADOR);//toma el comando. primer cadena antes de un espacio.
                args[1] = strtok(NULL," "); //como el parametro es NULL, toma la ultma cadena q se le paso y sigue avanzando
                for (int i = 2; i < 20 && args[i] != NULL; i++) {// obtengo todos los argumentos
                    args[i] = strtok(NULL, " ");
                }
                int interno;
                if ( (interno =comandosInternos(args)) == 0) {

                    findAndExecute(args, concurrentFlag, fdin , fdout);
                }else if(interno==1){
                    return 0;
                }
                pipeIndex++;
            }while(pipes[pipeIndex]!=NULL && *pipes[pipeIndex]!='\0');

            index++;
        }while(comands[index]!=NULL && *comands[index]!='\0');

        }


}
void SIGCHLDHandler(int s){  // handler de señal, lee el status del proceso terminado.
    wait(NULL);//printf("%d",s);
    }