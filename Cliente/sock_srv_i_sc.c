#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>

#define SIZEBUFF 512

int main( int argc, char *argv[] )
{
	int sockfd, readbytes, filefd;
    char buffer[SIZEBUFF];
    struct sockaddr_in serv_addr;
    char endflag[] = {"end-------"};
    char okflag[] = {"ok"};
    uint16_t  puerto = 5000;
    socklen_t size_direccion;

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

    memset( buffer, 0, SIZEBUFF );

    readbytes = (int)recvfrom( sockfd, buffer, SIZEBUFF-1, 0, (struct sockaddr *)&serv_addr, &size_direccion );
    if ( readbytes < 0)
    {
        perror( "lectura de socket" );
        exit( 1 );
    }


    printf("Recivir archivo %s\n",buffer);
    buffer[readbytes] = '\0';
    if (sendto(sockfd, (void *)okflag, strlen(okflag), 0, (struct sockaddr *)&serv_addr, size_direccion ) < 0)
    {
        perror( "escritura en socket" );
        exit( 1 );
    }
    filefd = open(buffer,O_RDWR | O_CREAT, 0666);

    memset( buffer, 0, SIZEBUFF );

    while ((readbytes = (int)recvfrom(sockfd, buffer, SIZEBUFF-1, 0,(struct sockaddr *)&serv_addr,
                                      &size_direccion)) > 0)
    {
        if ( readbytes < 0)
        {
            perror( "lectura de socket" );
            exit( 1 );
        }
        buffer[readbytes] = '\0';

        if( strstr(buffer, endflag) != NULL){
            break;
        }

        if(write(filefd, buffer, readbytes) < 0 ){
            perror("escritura en socket");
            exit(1);
        }
        memset( buffer, 0, SIZEBUFF );
    }
    close(filefd);

	return 0;
} 
