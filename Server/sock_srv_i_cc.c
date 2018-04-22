//
// Created by sergio on 30/03/18.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h> /*declaraciones de read and write*/
#include <poll.h>
#include <pwd.h>
#include <netdb.h> /*descarga*/
#include <fcntl.h>
#include <arpa/inet.h>

#define DEF_SIZE 1024
#define SIZEBUFF 1024
#define PORTUDP 5000

int authentication(int newsockfd, char* buffer, char* pass);
int downolad(int newsockfd, char *ip);
void SIGCHLDHandler(int s);

int main( int argc, char *argv[] )
{
    /* Atributos de atenticacion del server*/
    char pass[] ={"server"};

    int sockfd, newsockfd, clilen, pid;
    char buffer[DEF_SIZE];
    /* Estructura del socket del cliente */
    struct sockaddr_in serv_addr, cli_addr;
    ssize_t readbytes = 0;
    /* files descriptors pipe */
    int tob[2], formb[6];
    /* atributos para la ejecucion del bash */
    char * argv_h[] = {"../Server/bash", 0};
    /* estructura para la funcion poll, cientiene 2 fd que escuchara*/
    struct pollfd pfds[2];
    uint16_t puerto = 6020;

    signal(SIGCHLD,SIGCHLDHandler);

    memset(buffer, '\0', DEF_SIZE);

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        perror(" apertura de socket ");
        exit( 1 );
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
        exit(1);
    }

    printf("Proceso: %d - socket disponible: %d\n", getpid(), ntohs(serv_addr.sin_port));
    /*escucho hasta 5 peticiones*/
    listen(sockfd, 5);
    clilen = sizeof(cli_addr);

    while(1)
    {
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, (socklen_t*)&clilen);
        if (newsockfd < 0)
        {
            perror("accept");
            exit(1);
        }

        pid = fork();
        if (pid < 0)
        {
            perror("fork");
            exit(1);
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
            if (pipe( tob ) < 0 ) {
                perror( "apertura pipe" );
                exit(1);
            }
            if (pipe( formb ) < 0 ) {
                perror( "apertura pipe" );
                exit(1);
            }
            /*Agregados */
            pfds[0].fd = newsockfd;
            pfds[0].events = POLLIN;

            pfds[1].fd = formb[0];
            pfds[1].events = POLLIN;

            pid = fork();
            if (pid < 0)
            {
                printf("Fork error \n");
                exit(1);
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
                    poll(pfds , 2 ,-1);

                    if(pfds[1].revents  != 0)
                    {
                        if((readbytes = read(formb[0], buffer, DEF_SIZE)) >= 0)
                            write(newsockfd, buffer, (size_t )readbytes);

                    }

                    if(pfds[0].revents  != 0)
                    {
                        if ((readbytes = read(newsockfd, buffer, DEF_SIZE)) < 0)
                        {
                            perror("lectura de socket");
                            exit(EXIT_FAILURE);
                        }
                        if(strstr(buffer,"descarga::") == buffer)
                        {
                            printf("comand descarga\n");
                            downolad(newsockfd, inet_ntoa(cli_addr.sin_addr));
                            continue;
                        }
                        write(tob[1], buffer, (size_t) readbytes);

                        if (strstr(buffer, "exit") != NULL)
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

int authentication(int newsockfd, char* buffer, char* pass)
{
    struct passwd *pwd;

    pwd = getpwuid(geteuid());
    /*usuario*/
    if (read(newsockfd, buffer, DEF_SIZE - 1) < 0)
    {
        perror("lectura de socket");
        return -1;
    }

    if (strcmp(pwd->pw_name, buffer) != 0)
    {
        printf("%s\n", buffer);
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
    if (read(newsockfd, buffer, DEF_SIZE - 1) < 0)
    {
        perror("lectura de socket");
        return -1;
    }

    if (strcmp(pass, buffer) == 0)
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

int downolad(int newsockfd, char* ip)
{

    char path[DEF_SIZE];
    uint16_t port = PORTUDP;
    char vagrant[]= {"/home/sergio/CLionProjects/OSystem/TP1SO_Socket/Server/Vagrantfile"};
    int sockfd, filefd;
    struct sockaddr_in dest_addr;
    struct hostent *server;
    char buffer[SIZEBUFF];
    char endflag[] = {"end-------"};
    printf("%s\n",ip);
    socklen_t size_direccion;
    int n;
    /* Recivo el path absoluto por tcp*/
    memset(path, '\0', DEF_SIZE);
    memset(buffer, '\0', SIZEBUFF);



    /*buffer[n]=0;
    strcpy(path,"./");
    strcat(path, buffer);*/
    printf("path %s\n",vagrant);
    /* busca por ip */
    server = gethostbyname(ip);
    if ( server == NULL ) {
        perror("ERROR, no existe el host\n");
        return -1;
    }

    sockfd = socket( AF_INET, SOCK_DGRAM, 0 );
    if (sockfd < 0) {
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
    /* comunicacion previa */
    if (sendto(sockfd, "path" , strlen("path"), 0, (struct sockaddr *) &dest_addr, size_direccion) < 0)
    {
        perror("Escritura en socket");
        return -1;
    }

    if (recvfrom(sockfd, (void *) buffer, SIZEBUFF, 0, (struct sockaddr *) &dest_addr, &size_direccion) < 0)
    {
        perror("Lectura de socket");
        return -1;
    }

    printf("%s\n",buffer);

    filefd = open(vagrant, O_RDONLY);
    if(filefd < 0)
    {
        perror("open file");
        return -1;
    }

    memset(buffer, 0, SIZEBUFF);

    while ( read(filefd, buffer, SIZEBUFF) > 0)
    {
        if (sendto(sockfd, (void *) buffer, SIZEBUFF, 0, (struct sockaddr *) &dest_addr, size_direccion) < 0)
        {
            perror("Escritura en socket");
            return -1;
        }
        memset(buffer, 0, SIZEBUFF);
        printf("env..\n");
    }

    if (sendto(sockfd, (void *) endflag, strlen(endflag), 0, (struct sockaddr *) &dest_addr, size_direccion) < 0)
    {
        perror("Escritura en socket");
        return -1;
    }
    printf("findescarga\n");
    return 0;
}

void SIGCHLDHandler(int s)
{
    wait(NULL);
}
