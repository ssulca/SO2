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
/* como el NULL declarado en las otras funciones es un puntero, molestan los warnings
   defino NULL_I como un nulo para comparacion de valores de datos */
#define NULL_I 0


void SIGCHLDHandler(int s);

int main() {
    char input[BUFSIZE];
    char *args[20];
    /*buffer para el  Pc name*/
    char hostname[BUFSIZE];
    /* get PC name */
    gethostname(hostname, BUFSIZE-1);
    /* Asigna un Handler para la señal de terminacion de procesos hijos, permite
       permite limpiar los procesos zombies "dinamicamente" */
    signal(SIGCHLD,SIGCHLDHandler);

    while(1) {
        //printf("\033[33m%s@%s \033[34m%s $ \033[37m", getpwuid(geteuid())->pw_name, hostname, getcwd(NULL, 0));
        write(STDOUT_FILENO,".\n",3);
        /* pide el comando. se hace con fgets xq permite leer la linea completa */
        fgets(input, BUFSIZE, stdin);
        if(input[0]=='\n')
            continue;
        /* como el string se toma desde archivo, se debe borrar el ultimo caracter(\n)
           xq trae problemas para la lectura de los argumentos*/
        input[strlen(input) - 1] = NULL_I;
        /* comandos divididos por & */
        char *comands[]={NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};
        /* indica si los procesos se ejecutaran concurrentemente */
        int concurrentFlag=0;
        if(strstr(input," &")!='\0')
            concurrentFlag=1;

        comands[0]=strtok_r(input, "&", comands+1);
        int index=0;
        do {
            /* comandos divididos por ampersand */
            char* pipes[10]={NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};
            /* indica si el ultimo comando genero un pipe,
               de ser cierto contiene el descriptor de la salida del pipe */
            int pipePrevio=NULL_I;
            pipes[0]=strtok_r(comands[index], "|",pipes+1);
            int pipeIndex=0;

            do {
                int fdin=NULL_I;
                int fdout =NULL_I;
                if(pipePrevio!=NULL_I)
                {
                    fdin=pipePrevio;
                    pipePrevio=NULL_I;
                }
                if(pipes[pipeIndex+1]!=NULL && *pipes[pipeIndex+1]!='\0')/* si existe un siguiente comando */
                {            /* luego del operador pipe,  crea un pipe y cambia la stdout del proceso actual */
                    int fd[2];                     /* y guarda la salida del pipe para le proceso posterior */
                    pipe(fd);
                    fdout=fd[1];
                    pipePrevio=fd[0];
                    concurrentFlag=0;
                }
                /* verifica si hay redireccoin de stdout y std in */
                findRedirectCommand(pipes[index], &fdin, &fdout);
                /* toma el comando. primer cadena antes de un espacio. */
                args[0] = strtok(pipes[pipeIndex], SEG_DELIMITADOR);
                /* como el parametro es NULL, toma la ultma cadena q se le paso y sigue avanzando */
                args[1] = strtok(NULL," ");
                for (int i = 2; i < 20 && args[i] != NULL; i++) /* obtengo todos los argumentos */
                    args[i] = strtok(NULL, " ");

                int interno;
                if ( (interno =comandosInternos(args)) == 0)
                    findAndExecute(args, concurrentFlag, fdin , fdout);
                else if(interno==1)
                    return 0;

                pipeIndex++;
            }
            while(pipes[pipeIndex]!=NULL && *pipes[pipeIndex]!='\0');
            index++;
        }
        while(comands[index]!=NULL && *comands[index]!='\0');
        //write(STDOUT_FILENO,"\n",2);
    }
}

/**
 *  handler de señal, lee el status del proceso terminado.
 */
void SIGCHLDHandler(int s)
{
    wait(NULL);
}