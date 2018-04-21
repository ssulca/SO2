#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#define SIZEBUFF 256

int main( int argc, char *argv[] )
{
	int sockfd, size_direccion;
    char buffer[SIZEBUFF];
    struct sockaddr_in serv_addr;
    uint16_t  puerto = 5000;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd < 0)
	{
		perror("ERROR en apertura de socket");
		exit( 1 );
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
		exit( 1 );
	}

    printf( "Socket disponible: %d\n", ntohs(serv_addr.sin_port) );

	size_direccion = sizeof( struct sockaddr );
	while ( 1 ) {
		memset( buffer, 0, SIZEBUFF );

		if (recvfrom( sockfd, buffer, SIZEBUFF-1, 0,
            (struct sockaddr *)&serv_addr, (socklen_t *)&size_direccion ) < 0)
        {
			perror( "lectura de socket" );
			exit( 1 );
		}
		printf( "Recibí: %s", buffer );

		if (sendto(sockfd, (void *)"Obtuve su mensaje", 18, 0,
            (struct sockaddr *)&serv_addr, (socklen_t)size_direccion ) < 0)
        {
			perror( "escritura en socket" );
			exit( 1 );
		}
	}
	return 0;
} 
