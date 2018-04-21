#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fcntl.h>
#include <unistd.h>

#define SIZEBUFF 512

int main( int argc, char *argv[] ) {
	int sockfd, filefd;
	struct sockaddr_in dest_addr;
	struct hostent *server;
	char buffer[SIZEBUFF];
	char endflag[] = {"end-------"};
    uint16_t puerto = 5000;
    socklen_t size_direccion;

    char * nombre, * path;

    if (argc < 4)
    {
        perror("Uso, host,path name\n");
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

    path = argv[2];
    nombre = argv[3];

    size_direccion = sizeof( dest_addr );

    /* comunicacion previa */
    if (sendto(sockfd, (void *) nombre , strlen(nombre), 0, (struct sockaddr *) &dest_addr, size_direccion) < 0)
    {
        perror("Escritura en socket");
        exit(1);
    }

    if (recvfrom(sockfd, (void *) buffer, SIZEBUFF, 0, (struct sockaddr *) &dest_addr, &size_direccion) < 0)
    {
        perror("Lectura de socket");
        exit(1);
    }

    if (!strncmp(buffer, "ok", 2))
        printf("Filename sent.\n");
    else
        exit(1);

    filefd = open(path, O_RDONLY);
    if(filefd < 0)
    {
        perror("open file");
        exit(1);
    }
    memset(buffer, 0, SIZEBUFF);

    while ( read(filefd, buffer, SIZEBUFF) > 0)
    {
        if (sendto(sockfd, (void *) buffer, SIZEBUFF, 0, (struct sockaddr *) &dest_addr, size_direccion) < 0)
        {
            perror("Escritura en socket");
            exit(1);
        }
        memset(buffer, 0, SIZEBUFF);
    }

    memset(buffer, 0, SIZEBUFF);
    strcat(buffer, endflag);

    if (sendto(sockfd, (void *) buffer, strlen(endflag), 0, (struct sockaddr *) &dest_addr, size_direccion) < 0)
    {
        perror("Escritura en socket");
        exit(1);
    }

    return 0;
} 
