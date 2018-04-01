//
// Created by sergio on 30/03/18.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
/*declaraciones de las funciones rear and write*/
#include <unistd.h>

#define BUFSIZE 512
#define PROMBUF 256
#define USERBUF 32
#define IPBUF 15
#define PORTBUF 6

int command(char* u, char* i, char* p);

int main( int argc, char *argv[] ) {
    /* file descriptor del socket, puerto de conxion*/
    int sockfd, puerto, n;
    /* Estructura del socket del cliente*/
    struct sockaddr_in serv_addr;
    /* structura, contiene datos del host remoto*/
    struct hostent *server;
    int terminar = 0;

    char buffer[BUFSIZE];

    /*nuevos varaibles */
    char user[USERBUF];
    char ip[IPBUF];
    char port[PORTBUF];

    do{
        terminar = command(user, ip, port);
    }while (terminar != 0);


    puerto = atoi( port ); /* convierto numero entero*/
    if (puerto == 0){
        perror("Error al convertir el puerto");
        exit(1);
    }
    /*creacion del socket */
    sockfd = socket( AF_INET, SOCK_STREAM, 0 );
    if ( sockfd < 0 ){
        perror( "ERROR apertura de socket" );
        exit( 1 );
    }
    /*nombre del host que resuelve DNS por IP pasada por argv*/
    server = gethostbyname( ip );

    if (server == NULL) {
        fprintf( stderr,"Error, no existe el host\n" );
        exit( 0 );
    }

    /* Limpieza de la estructura */
    memset( (char *) &serv_addr, '0', sizeof(serv_addr) ); /*todos los valores en cero*/
    /* Carga de la familia de direccioens */
    serv_addr.sin_family = AF_INET;
    bcopy( (char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length );
    /* Carga del número de puerto format big Endian*/
    serv_addr.sin_port = htons( puerto );


    if ( connect( sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr ) ) < 0 ) {
        perror( "conexion" );
        exit( 1 );
    }

    while(1) {
        printf( "Ingrese el mensaje a transmitir: " );
        memset( buffer, '\0', BUFSIZE );
        fgets( buffer, BUFSIZE-1, stdin );

        n = write( sockfd, buffer, strlen(buffer) );
        if ( n < 0 ) {
            perror( "escritura de socket" );
            exit( 1 );
        }

        // Verificando si se escribió: fin
        buffer[strlen(buffer)-1] = '\0';
        if( !strcmp( "fin", buffer ) ) {
            terminar = 1;
        }

        memset( buffer, '\0', BUFSIZE );
        n = read( sockfd, buffer, BUFSIZE );
        if ( n < 0 ) {
            perror( "lectura de socket" );
            exit( 1 );
        }
        printf( "Respuesta %i: %s\n",n, buffer );
        if( terminar ) {
            printf( "Finalizando ejecución\n" );
            exit(0);
        }
    }


    return 0;
}

int command(char* u, char* i, char* p){
    char prompt[PROMBUF];
    char *user;
    char *ip;
    char *port;

    printf("prompt > ");
    fgets(prompt,PROMBUF, stdin);
    if (strstr(prompt, "connect") == NULL){
        perror("Comando no reconocido");
        return -1;
    }
    user = strstr(prompt, " " );
    if (user == NULL){
        perror("usuario");
        return -1;
    }
    user = user+1;
    ip = strstr(prompt, "@" );
    if(ip == NULL){
        perror("ip");
        return -1;
    }
    port = strstr(prompt, ":" );
    if(port == NULL){
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