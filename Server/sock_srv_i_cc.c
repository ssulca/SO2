//
// Created by sergio on 30/03/18.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <poll.h>
#include <pwd.h>
#include <netdb.h>
#include <fcntl.h>
#include <arpa/inet.h>

#define BUFF_MAX 512
#define BUFFUDP_MAX 1024
#define PORT_UDP 5000
#define PORT_TCP 6020

int authentication(int newsockfd, char* buffer, char* pass);
int downolad(char *pathtk, char *ip);
void SIGCHLDHandler(int s);

int main( int argc, char *argv[] )
{
    /* Atributos de atenticacion del server*/
    int     sockfd,
            newsockfd,
            clilen,
            pid;
    /* files descriptors pipe */
    int     tob[2],
            formb[6];
    ssize_t readbytes = 0;

    char    buffer[BUFF_MAX];
    char    path[BUFF_MAX];
    char    pass[] = {"server"};
    /* atributos para la ejecucion del bash */
    char    *argv_h[] = {"../Server/bash", 0};
    /*  variables para manter el cwd del bash*/
    char    *pathtk;
    /* Estructura del socket del cliente */
    struct  sockaddr_in     serv_addr,
                            cli_addr;
    /* estructura para la funcion poll, cientiene 2 fd que escuchara*/
    struct  pollfd pfds[2];

    uint16_t puerto = PORT_TCP;
    /*Asigna un Handler para la señal de terminacion de procesos hijos*/
    signal(SIGCHLD,SIGCHLDHandler);

    memset(buffer, '\0', BUFF_MAX);

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
      {
        perror(" apertura de socket ");
        exit(EXIT_FAILURE);
      }

    /* Limpieza de la estructura */
    memset((char *)&serv_addr, 0, sizeof(serv_addr));

    /* Carga de la familia de direccioens */
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;

    /* Carga del número de puerto format big endian */
    serv_addr.sin_port = htons(puerto);

    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
      {
        perror( "ligadura error al construir socket server" );
        exit( EXIT_FAILURE);
      }

    printf("# Proceso: %d - socket disponible: %d.\n", getpid(), ntohs(serv_addr.sin_port));
    /*escucho hasta 5 peticiones*/
    listen(sockfd, 5);
    clilen = sizeof(cli_addr);

    while(1)
      {
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, (socklen_t*)&clilen);
        if (newsockfd < 0)
          {
            perror("accept");
            exit( EXIT_FAILURE);
          }

        pid = fork();
        if (pid < 0)
          {
            perror("fork");
            exit( EXIT_FAILURE);
          }

        if (pid == 0) /* Proceso hijo del socket */
          {
            close(sockfd);

            if (authentication(newsockfd, buffer, pass)<0)
              {
                printf("coexion succes\n");
                exit(1);
              }

            /* pipes full duplex */
            if (pipe( tob ) < 0 )
              {
                perror( "apertura pipe" );
                exit( EXIT_FAILURE);
              }
            if (pipe( formb ) < 0 )
              {
                perror( "apertura pipe" );
                exit( EXIT_FAILURE);
              }
            /*arreglo de descripores para la funcion poll */
            pfds[0].fd = newsockfd;     /*agrego el filedescritor a escuchar*/
            pfds[0].events = POLLIN;    /*agrego el el evento a escuchar (entrada)*/
            pfds[1].fd = formb[0];
            pfds[1].events = POLLIN;

            pid = fork();
            if (pid < 0)
              {
                printf("Fork error \n");
                exit( EXIT_FAILURE);
              }
            if (pid == 0 ) /* proceso hijo ejecuta bash*/
              {
                close(newsockfd);
                /* cerrar el lado de escritura del pipe y  de lectura del pipe */
                close( tob[1] );
                close( formb[0] );

                /* Redireccion de stdout y stdin */
                dup2(tob[0], STDIN_FILENO);
                fclose(stdout);
                dup2(formb[1], STDOUT_FILENO);

                /* ejecucion de bash */
                execvp(argv_h[0], argv_h);
                printf("Exec error \n");

                close( tob[0] );
                close( formb[1] );
                exit(1);
              }
            else /* padre encargado de la comuniacion con el socket */
              {
                /* cerramos el lado de lectura del pipe  y escritura del pipe */
                close( tob[0] );
                close( formb[1] );
                while(1)
                  {
                    /*escuha a los descritores, args, arreglo de descriotres
                     * 2, catidad de descriotres , -1 sin timeout*/
                    poll(pfds , 2 ,-1);

                    if(pfds[1].revents  != 0) /*eveto de lectura del pipe*/
                      {
                        if((readbytes = read(formb[0], buffer, BUFF_MAX)) < 0)
                          {
                            perror("lectura de pipe");
                            exit(EXIT_FAILURE);
                          }
                        if(strstr(buffer,"/home/")!= NULL) /*salida contien el cwd*/
                          {
                            memset(path,'\0',BUFF_MAX);
                            strcpy(path,buffer);
                            pathtk = strtok(path, " ");
                            /*obtengo solo el path*/
                            pathtk = strtok(NULL, " ");
                          }
                        if(write(newsockfd, buffer, (size_t)readbytes) < 0)
                          {
                            perror("escritura de socket");
                            exit(EXIT_FAILURE);
                          }
                      }

                    if(pfds[0].revents  != 0)
                      {
                        if ((readbytes = read(newsockfd, buffer, BUFF_MAX)) < 0)
                          {
                              perror("lectura de socket");
                              exit(EXIT_FAILURE);
                          }
                        if(strstr(buffer,"descarga::") == buffer)
                          {
                            printf("# solicitud descarga.\n");
                            downolad(pathtk, inet_ntoa(cli_addr.sin_addr));
                            continue;
                          }
                        if (write(tob[1], buffer, (size_t) readbytes) < 0)
                          {
                            perror("escritura de pipe");
                            exit(EXIT_FAILURE);
                          }
                        if (strstr(buffer, "exit") == buffer)
                            break;
                      }
                  }

                close(tob[1]);
                close(formb[0]);
                wait(NULL);
              }
          }
        else /* Proceso padre del socket*/
          {
            printf( "SERVIDOR: Nuevo cliente, que atiende el proceso hijo: %d\n", pid);
            close(newsockfd);
          }
      }
    return 0;
}

/**
 *
 * @param newsockfd int , file descritor del socekt
 * @param buffer char*, buffer obtener datos pasado del cliente user, pass
 * @param pass char*, password de acceso del servidor
 * @return int , 0 exitosos , -1 de contrario.
 */
int authentication(int newsockfd, char* buffer, char* pass)
{
    /*obtener el usuario corriente*/
    struct passwd *pwd;

    pwd = getpwuid(geteuid());
    /*usuario*/
    if (read(newsockfd, buffer, BUFF_MAX - 1) < 0)
      {
        perror("lectura de socket");
        return -1;
      }

    if (strcmp(pwd->pw_name, buffer) != 0) /*comparo usuario*/
      {
        if (write(newsockfd, "unknown", 8) < 0)
            perror("escritura en socket");
        return -1;
      }
    else
      {
        if (write(newsockfd, "know", 5) < 0)
          {
            perror("escritura en socket");
            return -1;
          }
      }
    /* pass autentication */
    if (read(newsockfd, buffer, BUFF_MAX - 1) < 0)
      {
        perror("lectura de socket");
        return -1;
      }

    if (strcmp(pass, buffer) == 0)/*verifico password*/
      {
        if (write(newsockfd, "accepted", 9) < 0)
        {
            perror("escritura en socket");
            return -1;
        }
        return 0;
      }
    else
      {
        if (write(newsockfd, "rejected", 9) < 0)
            perror("escritura en socket");
        return -1;
      }
}

/**
 * funcion descarga archivo.
 * @param pathtk char* , path actual del bash
 * @param ip char*, ip del cliente
 * @return int 0 , descarga completa, -1 caso contrario.
 */
int downolad(char *pathtk, char *ip)
{
    char path[BUFFUDP_MAX];
    char buffer[BUFFUDP_MAX];
    char endflag[] = {"end-------"};
    uint16_t port = PORT_UDP;
    int     sockfd,
            filefd;

    socklen_t size_direccion;

    struct sockaddr_in  dest_addr;
    struct hostent      *server;

    /* Recivo el path absoluto por tcp*/
    memset(path, '\0', BUFFUDP_MAX);
    memset(buffer, '\0', BUFFUDP_MAX);

    server = gethostbyname(ip);
    if ( server == NULL )
      {
        perror("ERROR, no existe el host\n");
        return -1;
      }

    sockfd = socket( AF_INET, SOCK_DGRAM, 0 );
    if (sockfd < 0)
      {
        perror( "apertura de socket" );
        return -1;
      }

    /* Carga de la familia de direccioens */
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(port);
    dest_addr.sin_addr = *( (struct in_addr *)server->h_addr );
    memset( &(dest_addr.sin_zero), '\0', 8 );

    size_direccion = sizeof( dest_addr );

    /* comunicacion previa */
    if (sendto(sockfd, "path" , strlen("path"), 0, (struct sockaddr *) &dest_addr, size_direccion) < 0)
      {
        perror("Escritura en socket");
        exit(EXIT_FAILURE);
      }

    if (recvfrom(sockfd, (void *) buffer, BUFFUDP_MAX-1, 0, (struct sockaddr *) &dest_addr, &size_direccion) < 0)
      {
        perror("Lectura de socket");
        exit(EXIT_FAILURE);
      }

    buffer[strlen(buffer)-1]='\0';
    /*obtengo el path absoluto*/
    strcpy(path,pathtk);
    strcat(path, "/");
    strcat(path, buffer);
    printf("path %s\n",path);
    /* busca por ip */


    filefd = open(path, O_RDONLY);
    if(filefd < 0)
      {
        perror("open file");
        return -1 ;
      }

    memset(buffer, 0, BUFFUDP_MAX);

    while ( read(filefd, buffer, BUFFUDP_MAX) > 0)
      {
        if (sendto(sockfd, (void *) buffer, BUFFUDP_MAX, 0, (struct sockaddr *) &dest_addr, size_direccion) < 0)
          {
            perror("Escritura en socket");
            exit(EXIT_FAILURE);
          }
        memset(buffer, 0, BUFFUDP_MAX);
      }

    if (sendto(sockfd, (void *) endflag, strlen(endflag), 0, (struct sockaddr *) &dest_addr, size_direccion) < 0)
      {
        perror("Escritura en socket");
        exit(EXIT_FAILURE);
      }
    printf("fin descarga\n");
    close(sockfd);
    return 0;
}

/**
 * Asigna un Handler para la señal de terminacion de procesos hijos, permite
 * permite limpiar los procesos zombies "dinamicamente"
 */
void SIGCHLDHandler(int s)
{
    wait(NULL);
}
