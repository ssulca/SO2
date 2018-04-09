//
// Created by sergio on 30/03/18.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
/*declaraciones de las funciones rear and write */
#include <unistd.h>
#include <poll.h>

#define BUFSIZE 512
#define PROMBUF 256
#define USERBUF 32
#define IPBUF 15
#define PORTBUF 6

int command(char* u, char* i, char* p);
void xfer_data(int srcfd, int tgtfd);

int main( int argc, char *argv[] )
{
    /* file descriptor del socket, puerto de conxion*/
    int sockfd, puerto;
    ssize_t readbytes = 0;
    /* Estructura del socket del serv*/
    struct sockaddr_in serv_addr;
    /* structura, contiene datos del host remoto*/
    struct hostent *server;
    int terminar = 0;

    char buffer[BUFSIZE];
    /*nuevos varaibles */
    char user[USERBUF];
    char ip[IPBUF];
    char port[PORTBUF];

    /* Agregados */
    struct pollfd pfds[2];

    do {
        terminar = command(user, ip, port);
    }
    while (terminar != 0);

    /* convierto numero entero*/
    puerto = atoi(port);
    if (puerto == 0)
    {
        perror("Error al convertir el puerto");
        exit(1);
    }
    /*creacion del socket */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        perror( "ERROR apertura de socket" );
        exit( 1 );
    }
    /*nombre del host que resuelve DNS por IP pasada*/
    server = gethostbyname( ip );

    if (server == NULL)
    {
        fprintf( stderr,"Error, no existe el host\n" );
        exit( 0 );
    }

    /* Limpieza de la estructura todos los valores en cero*/
    memset((char *) &serv_addr, '0', sizeof(serv_addr));
    /* Carga de la familia de direccioens */
    serv_addr.sin_family = AF_INET;
    /* function copies n bytes de la direccion from server to socket local */
    bcopy(server->h_addr, (char *)&serv_addr.sin_addr.s_addr, (size_t)server->h_length );
    /* Carga del número de puerto format big Endian*/
    serv_addr.sin_port = htons((uint16_t)puerto);

    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        perror( "conexion" );
        exit( 1 );
    }

    pfds[0].fd = sockfd;
    pfds[0].events = POLLIN;
    pfds[1].fd = STDIN_FILENO;
    pfds[1].events = POLLIN;


    while(1)
    {
        poll(pfds , 2 ,-1);
        if(pfds[0].revents  != 0)
        {
            memset(buffer, '\0', BUFSIZE);
            if((readbytes = read(sockfd, buffer, BUFSIZE)) >= 0)
                write(STDOUT_FILENO, buffer, (size_t)readbytes);
        }

        if(pfds[1].revents  != 0)
        {
            memset(buffer, '\0', BUFSIZE);
            if((readbytes = read(STDIN_FILENO, buffer, BUFSIZE)) >= 0)
                write(sockfd, buffer, (size_t )readbytes);

            if (strstr(buffer, "exit") != NULL)
                break;
        }
        /*if((readbytes = read(sockfd, buffer, BUFSIZE))>0)
            write(STDOUT_FILENO, buffer, (size_t)readbytes);

        if((readbytes = read(STDIN_FILENO, buffer, BUFSIZE))>0)
            write(sockfd, buffer, (size_t )readbytes);

        if (strstr(buffer, "exit") != NULL)
            break;*/
    }
    return 0;
}

int command(char* u, char* i, char* p)
{
    char prompt[PROMBUF];
    char *user;
    char *ip;
    char *port;

    printf("prompt > ");
    fgets(prompt,PROMBUF, stdin);
    if (strstr(prompt, "connect") == NULL)
    {
        perror("Comando no reconocido");
        return -1;
    }
    user = strstr(prompt, " " );
    if (user == NULL)
    {
        perror("usuario");
        return -1;
    }
    user = user + 1;
    ip = strstr(prompt, "@" );
    if(ip == NULL)
    {
        perror("ip");
        return -1;
    }
    port = strstr(prompt, ":" );
    if(port == NULL)
    {
        perror("puerto");
        return -1;
    }

    *ip = '\0';
    *port = '\0';

    ip = ip+1;
    port = port+1;

    strcpy(u,user);
    strcpy(i,ip);
    strcpy(p,port);

    return 0;
}

/**
 * fileno() Devuelve el número de descriptor de archivo asociado
 *
 */
void xfer_data(int srcfd, int tgtfd)
{
    char buf[1024];
    int cnt, len;
    /* leer desde el archivo stdin y escribir el archivo de stdout  */
    if((cnt = (int)read(srcfd, buf, sizeof(buf))) > 0)
    {
        if(len < 0)
            perror("helper.c:xfer_data:read");
        if((len = (int)write(tgtfd, buf, cnt)) != cnt)
            perror("helper.c:xfer_data:write");
    }
}