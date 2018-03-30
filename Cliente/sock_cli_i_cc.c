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
#include <unistd.h> /*declaraciones de las funciones rear and write*/

#define Bufsize 256

int main( int argc, char *argv[] ) {
    int sockfd, puerto, n; /* file descriptor del socket, puerto de conxion*/
    struct sockaddr_in serv_addr; /* Estructura del socket del cliente*/
    struct hostent *server;
    int terminar = 0;

    char buffer[Bufsize];
    if ( argc < 3 ) {
        fprintf( stderr, "Uso %s host puerto\n", argv[0]);
        exit( 0 );
    }

    puerto = atoi( argv[2] ); /* convierto numero entero*/
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
    /*Obtencion del hostname pasado por agumento */
    server = gethostbyname( argv[1] );
    if (server == NULL) {
        fprintf( stderr,"Error, no existe el host\n" );
        exit( 0 );
    }
    /* inicializacion de los valores de la estructura socket cliente*/
    memset( (char *) &serv_addr, '0', sizeof(serv_addr) ); /*todos los valores en cero*/
    serv_addr.sin_family = AF_INET; /* */
    bcopy( (char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length );
    serv_addr.sin_port = htons( puerto );

    if ( connect( sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr ) ) < 0 ) {
        perror( "conexion" );
        exit( 1 );
    }

    while(1) {
        printf( "Ingrese el mensaje a transmitir: " );
        memset( buffer, '\0', Bufsize );
        fgets( buffer, Bufsize-1, stdin );

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

        memset( buffer, '\0', Bufsize );
        n = read( sockfd, buffer, Bufsize );
        if ( n < 0 ) {
            perror( "lectura de socket" );
            exit( 1 );
        }
        printf( "Respuesta: %s\n", buffer );
        if( terminar ) {
            printf( "Finalizando ejecución\n" );
            exit(0);
        }
    }
    return 0;
}
