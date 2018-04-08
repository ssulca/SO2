
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h> /*declaraciones de read and write*/
#define TAM 256

void xfer_data(int srcfd, int tgtfd);

int main( int argc, char *argv[] ) {

	int sockfd, newsockfd, puerto, clilen, pid;
	char buffer[TAM];
    /* Estructura del socket del cliente */
    struct sockaddr_in serv_addr, cli_addr;
    int readbytes = 0;
    /* files descriptors pipe */
    int tob[2], formb[2];
    int pidv;
    /* argumentos para el bash */
    char * argv_h[] = {"/home/sergio/CLionProjects/OSystem/TP1SO_Socket/Server/bash", 0};

    memset(buffer, '\0', TAM);

    if (argc < 2)
    {
        fprintf(stderr, "Uso: %s <puerto>\n", argv[0]);
		exit( 1 );
	}

	sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0)
    {
		perror(" apertura de socket ");
		exit( 1 );
	}

    /* Limpieza de la estructura */
	memset((char *)&serv_addr, 0, sizeof(serv_addr));
	puerto = 6020;

    /* Carga de la familia de direccioens */
    serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;

    /* Carga del número de puerto format big endian */
	serv_addr.sin_port = htons((uint16_t)puerto);

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
		newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
		if (newsockfd < 0)
        {
			perror("accept cli");
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
            /* pipes full duplex */
            pipe( tob );
            pipe( formb );

            pid = vfork();
            if (pid < 0) {
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
                    if((readbytes = read(formb[0], buffer, TAM))>0)
                        write(newsockfd, buffer, readbytes);

                    if((readbytes = read(newsockfd, buffer, TAM))>0)
                        write(tob[1], buffer, strlen(buffer));

                    if (strstr(buffer, "exit") != NULL)
                        break;
                }

                close(tob[1]);
                close(formb[0]);
            }
            waitpid( pid, NULL, 0 );
        }
		else /* Proceso padre del socket*/
        {
			printf( "SERVIDOR: Nuevo cliente, que atiende el proceso hijo: %d\n", pid);
			close(newsockfd);
		}
	}
	return 0; 
}

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