#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#define SIZEBUFF 256

int main( int argc, char *argv[] ) {
	int sockfd, tamano_direccion;
	struct sockaddr_in dest_addr;
	struct hostent *server;
	char buffer[SIZEBUFF];
	uint16_t puerto = 5000;

    if (argc < 2)
    {
        perror("Uso %s host\n");
		exit(0);
	}
    /* busca por ip */
    server = gethostbyname( argv[1] );
	if ( server == NULL ) {
		perror("ERROR, no existe el host\n");
		exit(0);
	}

	sockfd = socket( AF_INET, SOCK_DGRAM, 0 );
	if (sockfd < 0) {
		perror( "apertura de socket" );
		exit( 1 );
	}

    /* Carga de la familia de direccioens */
	dest_addr.sin_family = AF_INET;
	dest_addr.sin_port = htons(puerto);
	dest_addr.sin_addr = *( (struct in_addr *)server->h_addr );
	memset( &(dest_addr.sin_zero), '\0', 8 );

	printf( "Ingrese el mensaje a transmitir: " );
	memset( buffer, 0, SIZEBUFF );
	fgets( buffer, SIZEBUFF, stdin );

	tamano_direccion = sizeof( dest_addr );

	if (sendto(sockfd, (void *)buffer, SIZEBUFF, 0,
        (struct sockaddr *)&dest_addr, (socklen_t )tamano_direccion ) < 0 )
    {
		perror( "Escritura en socket" );
		exit( 1 );
	}
	memset( buffer, 0, sizeof( buffer ) );

	if (recvfrom( sockfd, (void *)buffer, SIZEBUFF, 0,
        (struct sockaddr *)&dest_addr, (socklen_t*)&tamano_direccion ) < 0 )
    {
		perror( "Lectura de socket" );
		exit( 1 );
	}
	printf( "Respuesta: %s\n", buffer );
	return 0;
} 
