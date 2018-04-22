//
// Created by sergio on 30/03/18.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <poll.h>
#include <termios.h>
#include <fcntl.h>
#include <libgen.h>

#define SIZEBUFFER 512
#define SIZEPROMPT 256
#define SIZEUSER 32
#define SIZEIP 13
#define SIZEPORT 6
#define SIZEPASS 16
#define SIZEBUTP 1024
#define PORTUDP 5000

int command(char* u, char* i, char* p);
int fun_sock(char* ip, char* user,char* port);
int authentication(int sockfd, char* buffer, char* user);
int get_pass( char *buffer);
int download (int tcpsockfd , char* name);

int main( int argc, char *argv[] ) {
    int terminar = 0;

    /*nuevos varaibles */
    char user[SIZEUSER];
    char ip[SIZEIP];
    char port[SIZEPORT];

    while(1)
    {
        terminar = command(user, ip, port);
        if (terminar < 0)
            continue;
        if (terminar == 1)
            break;
        if(fun_sock(ip, user, port) == 0)
            break;
    }
    return 0;
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

    char buffer[SIZEBUFFER];

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
    /*nombre del host que resuelve por IP */
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
    if (authentication(sockfd, buffer,user) < 0) {
        printf("nombre de usuario y/o contraseña incorrecto\n");
        return -1;
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
            memset(buffer, '\0', SIZEBUFFER);
            if((readbytes = read(sockfd, buffer, SIZEBUFFER)) >= 0)
                write(STDOUT_FILENO, buffer, (size_t)readbytes);
        }
        if(pfds[1].revents  != 0)
        {
            memset(buffer, '\0', SIZEBUFFER);
            if((readbytes = read(STDIN_FILENO, buffer, SIZEBUFFER)) < 0)
            {
                perror("lectura de socket");
                exit(EXIT_FAILURE);
            }
            if(strstr(buffer, "descarga ") == buffer)
            {
                write(sockfd, "descarga::", strlen("descarga::"));
                download(sockfd, basename(buffer + strlen("descarga ")));
                continue;
            }
            write(sockfd, buffer, (size_t )readbytes);

            if (strstr(buffer, "exit") != NULL)
                break;
        }
    }
    return 0;
}

int command(char* u, char* i, char* p)
{
    char prompt[SIZEPROMPT];
    char *user;
    char *ip;
    char *port;

    printf("> ");
    fgets(prompt,SIZEPROMPT, stdin);

    if (strstr(prompt, "exit") == prompt)
    {
        return 1;
    }

    if (strstr(prompt, "connect ") != prompt)
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
    if ( read( sockfd, buffer, SIZEBUFFER ) < 0 ) {
        perror( "lectura de socket" );
        return -1;
    }
    if (strstr(buffer, "unknown") != NULL) /*no existe el usuario*/
        return -1 ;

    memset(buffer,'\0',SIZEBUFFER);
    get_pass(buffer);

    if ( write( sockfd, buffer, strlen(buffer)) < 0 )
    {
        perror( "escritura de socket" );
        return -1;
    }

    if ( read( sockfd, buffer, SIZEBUFFER ) < 0 )
    {
        perror( "lectura de socket" );
        return -1;
    }

    if (strstr(buffer, "accepted") != NULL) /*aceptado*/
        return 0;
    /* cualqier error*/
    return -1;
}
int get_pass( char *buffer)
{
    char passwd[SIZEPASS];
    char *in = passwd;
    struct termios tty_orig;
    char c;

    tcgetattr( STDIN_FILENO, &tty_orig );
    struct termios  tty_work = tty_orig;

    memset(passwd,'\0',SIZEPASS);

    puts("password:");
    tty_work.c_lflag &= ~( ECHO | ICANON );  /* | ISIG )*/
    tty_work.c_cc[ VMIN ]  = 1;
    tty_work.c_cc[ VTIME ] = 0;
    tcsetattr( STDIN_FILENO, TCSAFLUSH, &tty_work );

    while (1)
    {
        if (read(STDIN_FILENO, &c, sizeof c) > 0)
        {
            if ('\n' == c)
                break;
            *in++ = c;
            write(STDOUT_FILENO, "*", 1);
        }
    }
    tcsetattr( STDIN_FILENO, TCSAFLUSH, &tty_orig );

    *in = '\0';
    fputc('\n', stdout);
    strcpy(buffer, passwd);
    return 0;
}

int download (int tcpsockfd , char* name)
{
    int sockfd, readbytes, filefd;
    char buffer[SIZEBUTP];
    struct sockaddr_in serv_addr;
    char endflag[] = {"end-------"};

    uint16_t  puerto = PORTUDP;
    socklen_t size_direccion;


    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
    {
        perror("ERROR en apertura de socket");
        return -1;
    }

    memset( &serv_addr, 0, sizeof(serv_addr) );

    /* Carga de la familia de direccioens */
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(puerto);
    memset( &(serv_addr.sin_zero), '\0', 8 );

    if( bind( sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr) ) < 0 )
    {
        perror( "ERROR en binding" );
        return -1;
    }
    printf( "Socket disponible: %d\n", ntohs(serv_addr.sin_port) );

    size_direccion = sizeof( struct sockaddr );

    memset( buffer, 0, SIZEBUTP );

    readbytes = (int)recvfrom( sockfd, buffer, SIZEBUTP-1, 0, (struct sockaddr *)&serv_addr, &size_direccion );
    if ( readbytes < 0)
    {
        perror( "lectura de socket" );
        exit( 1 );
    }
    if(strstr(buffer,"path") != buffer)
        return -1;

    printf("Recivir archivo %s\n", name);
    buffer[readbytes] = '\0';

    if (sendto(sockfd, (void *)name, strlen(name), 0, (struct sockaddr *)&serv_addr, size_direccion ) < 0)
    {
        perror( "escritura en socket" );
        exit( 1 );
    }

    name[strlen(name)-1]='\0';
    printf("Download...\n");

    filefd = open(name, O_RDWR | O_CREAT, 0666);

    memset( buffer, 0, SIZEBUTP );

    while ((readbytes = (int)recvfrom(sockfd, buffer, SIZEBUTP-1, 0,(struct sockaddr *)&serv_addr,
                                      &size_direccion)) > 0)
    {
        printf("recibiendo\n");
        if ( readbytes < 0)
        {
            perror( "lectura de socket" );
            return -1;
        }
        buffer[readbytes] = '\0';

        if( strstr(buffer, endflag) == buffer){
            printf("end recv\n");
            break;
        }

        if(write(filefd, buffer, readbytes) < 0 ){
            perror("escritura en socket");
            return -1;
        }
        memset( buffer, 0, SIZEBUTP );
    }
    printf("end file\n");

    close(filefd);

    return 0;
}
