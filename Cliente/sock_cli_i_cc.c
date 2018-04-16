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
#include <termios.h>


#define BUFSIZE 512
#define PROMBUF 256
#define USERBUF 32
#define IPBUF 15
#define PORTBUF 6

int command(char* u, char* i, char* p);
int fun_sock(char* ip, char* user,char* port);
int authentication(int sockfd, char* buffer, char* user);
int get_pass( char *pas);

int main( int argc, char *argv[] ) {
    int terminar = 0;


    /*nuevos varaibles */
    char user[USERBUF];
    char ip[IPBUF];
    char port[PORTBUF];

    while(1){
        terminar = command(user, ip, port);
        if (terminar < 0)
            continue;
        if (terminar == 1)
            break;
        if(fun_sock(ip, user, port) == 0)
            break;
    }
    return 0;
    /* convierto numero entero*/

}

int fun_sock(char* ip, char* user,char* port)
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

    /* Agregados */
    struct pollfd pfds[2];

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

    /*proceso de autenticacion */
    if (authentication(sockfd, buffer,user)<0)
        return -1;

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

    }
    return 0;
}

int command(char* u, char* i, char* p)
{
    char prompt[PROMBUF];
    char *user;
    char *ip;
    char *port;

    printf("> ");
    fgets(prompt,PROMBUF, stdin);

    if (strstr(prompt, "exit") == NULL)
    {
        perror("Comando no reconocido");
        return 1;
    }

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

int authentication(int sockfd, char* buffer, char* user)
{
    /* proceso de  atenticacion ,envio de usuario*/
    if ( write( sockfd, user, strlen(user)) < 0 ) {
        perror( "escritura de socket" );
        return -1;
    }
    if ( read( sockfd, buffer, BUFSIZE ) < 0 ) {
        perror( "lectura de socket" );
        return -1;
    }
    if (strstr(buffer, "unknown") != NULL) /*no existe el usuario*/
    {
        printf("unknown\n");
        return -1 ;
    }

    while(1){
        memset(buffer,'\0',BUFSIZE);
        get_pass(buffer);
        if ( write( sockfd, buffer, BUFSIZE) < 0 ) {
            perror( "escritura de socket" );
            return -1;
        }

        if ( read( sockfd, buffer, BUFSIZE ) < 0 ) {
            perror( "lectura de socket" );
            return -1;
        }
        if (strstr(buffer, "rejected") != NULL) /*no existe el usuario*/
            return -1 ;
        if (strstr(buffer, "accepted") != NULL) /*aceptado*/
            break;
    }
    return 0;

}
int get_pass( char *pas)
{
    char passwd[16];
    char *in = passwd;
    struct termios  tty_orig;
    char c;
    tcgetattr( STDIN_FILENO, &tty_orig );
    struct termios  tty_work = tty_orig;

    puts("Please input password:");
    tty_work.c_lflag &= ~( ECHO | ICANON );  // | ISIG );
    tty_work.c_cc[ VMIN ]  = 1;
    tty_work.c_cc[ VTIME ] = 0;
    tcsetattr( STDIN_FILENO, TCSAFLUSH, &tty_work );

    while (1) {
        if (read(STDIN_FILENO, &c, sizeof c) > 0) {
            if ('\n' == c) {
                break;
            }
            *in++ = c;
            write(STDOUT_FILENO, "*", 1);
        }
    }

    tcsetattr( STDIN_FILENO, TCSAFLUSH, &tty_orig );

    *in = '\0';
    fputc('\n', stdout);
    strcpy(pas,passwd);
    // if you want to see the result:
    // printf("Got password: %s\n", passwd);

    return 0;
}
