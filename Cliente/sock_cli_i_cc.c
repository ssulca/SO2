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
#include <libgen.h>

#define BUFF_MAX 1024
#define PROMPT_MAX 256
#define USER_MAX 32
#define IP_MAX 13
#define PORT_MAX 6
#define PASS_MAX 16
#define BUFFUDP_MAX 2048
#define PORT_UDP 5000

int command(char* u, char* i, char* p);
int fun_sock(char* ip, char* user,char* port);
int authentication(int sockfd, char* buffer, char* user);
int get_pass( char *buffer);
int download (char* name);

int main( int argc, char *argv[] )
{
    int terminar = 0;

    /*nuevos varaibles */
    char user[USER_MAX];
    char ip[IP_MAX];
    char port[PORT_MAX];

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

/**
 * Analiza el comando ingresado por el prompt
 * @param u char* contiene el nombre del usuario ingresado
 * @param i char* contiene la ip ingresada
 * @param p char* contiene el puerto ingresado
 * @return int -1 si el comando no se ingreso corretamente,
 * 0 de lo contrario
 */
int command(char* u, char* i, char* p)
{
    char prompt[PROMPT_MAX];
    char *user;
    char *ip;
    char *port;

    printf("> ");
    fgets(prompt,PROMPT_MAX, stdin);

    /*comado para salir de consola*/
    if (strstr(prompt, "exit") == prompt)
        return 1;

    /*verificar el comando connect*/
    if (strstr(prompt, "connect ") != prompt)
      {
        perror("Comando no reconocido");
        return -1;
      }
    user = strstr(prompt, " " );
    /*verificar el ingreso usuario*/
    if (user == NULL)
      {
        perror("usuario");
        return -1;
      }
    user = user + 1;
    ip = strstr(prompt, "@" );
    /*verificar el ingreso ip*/
    if(ip == NULL)
      {
        perror("ip");
        return -1;
      }
    port = strstr(prompt, ":" );
    /*verificar el ingreso puerto*/
    if(port == NULL)
      {
        perror("puerto");
        return -1;
      }

    *ip = '\0';
    *port = '\0';

    ip = ip+1;
    port = port+1;
    /*copiar a los buffers pasados*/
    strcpy(u,user);
    strcpy(i,ip);
    strcpy(p,port);

    return 0;
}

/**
 * funcion que se conecta cone el sevidor
 * @param ip char*, ip del server
 * @param user char*, nombre del del server
 * @param port char*, puerto del servidor a conectarse
 * @return int , -1 si no se pudo conectar , 0 si la conexion  se dio
 * y finalizo correctamente
 */
int fun_sock(char* ip, char* user,char* port)
{
    /* file descriptor del socket, puerto de conxion*/
    int     sockfd,
            puerto;
    ssize_t readbytes = 0;
    char buffer[BUFF_MAX];
    /*buffer entrada*/
    char buffin[BUFF_MAX];

    /* Estructura del socket del serv*/
    struct  sockaddr_in serv_addr;
    /* structura, contiene datos del host remoto*/
    struct  hostent     *server;
    /* arreglo de descripores para poll */
    struct  pollfd      pfds[2];


    /* convierto numero entero*/
    puerto = atoi(port);
    if (puerto == 0)
      {
        perror("Error al convertir el puerto");
        exit( EXIT_FAILURE);
      }
    /*creacion del socket */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
      {
        perror( "ERROR apertura de socket" );
        exit( EXIT_FAILURE);
      }
    /*nombre del host que resuelve por IP */
    server = gethostbyname( ip );

    if (server == NULL)
      {
        fprintf( stderr,"Error, no existe el host\n" );
        return -1;
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
        exit( EXIT_FAILURE);
      }

    /*proceso de autenticacion */
    if (authentication(sockfd, buffer,user) < 0)
      {
        printf("# Nombre de usuario y/o contraseña incorrecto.\n");
        return -1;
      }

    pfds[0].fd = sockfd;
    pfds[0].events = POLLIN;
    pfds[1].fd = STDIN_FILENO;
    pfds[1].events = POLLIN;

    while(1)
      {
        /* examina los descriptors para el evento datos para leer
         * escuha a los descritores, args, arreglo de descriotres
         * 2, catidad de descriotres , -1 sin timeout */
        poll(pfds , 2 ,-1);

        if(pfds[0].revents  != 0) /*evento lectura del socket*/
          {
            memset(buffin, '\0', BUFF_MAX);
              /*leer socket*/
              if((readbytes = read(sockfd, buffin, BUFF_MAX)) < 0)
              {
                perror("lectura de socket");
                exit(EXIT_FAILURE);
              }
              /*escribir en la salid*/
            if( write(STDOUT_FILENO, buffin, (size_t)readbytes) < 0)
              {
                perror("escritura de socket");
                exit(EXIT_FAILURE);
              }
          }

        if(pfds[1].revents  != 0)/*evento lectura de la entrada prompt*/
          {
            memset(buffer, '\0', BUFF_MAX);
            /*evento lectura de entrada*/
            if((readbytes = read(STDIN_FILENO, buffer, BUFF_MAX)) < 0)
              {
                perror("lectura de socket");
                exit(EXIT_FAILURE);
              }
            /*verificar ingreso el comando descarga */
            if(strstr(buffer, "descarga ") == buffer)
              {
                if (write(sockfd, "descarga::", strlen("descarga::")) < 0)
                  {
                    perror("escritura de socket");
                    exit(EXIT_FAILURE);
                  }
                /* inciar proceso descarga */
                download(basename(buffer + strlen("descarga ")));
                /* inciar proceso descarga */
                if( write(STDOUT_FILENO, buffin, (size_t)readbytes) < 0) /*escribo la ultima salida del server*/
                  {
                    perror("escritura de socket");
                    exit(EXIT_FAILURE);
                  }
                continue;
              }
            /* escribir socket */
            if (write(sockfd, buffer, (size_t )readbytes) < 0 )
              {
                perror("escritura de socket");
                exit(EXIT_FAILURE);
              }
            /* verificar comando exit */
            if (strstr(buffer, "exit") == buffer)
                break;
          }
      }
    return 0;
}

/**
 * funcion de autenticacion con el servidor.
 * @param sockfd int , file descriptor de comunicacion.
 * @param buffer char* , buffer
 * @param user char*, nombre del usuario del servidor a conectarse
 * @return int -1 si no fue aceptado , 0 de lo contrario
 */
int authentication(int sockfd, char* buffer, char* user)
{
    /* proceso de  atenticacion ,envio de usuario*/
    if ( write( sockfd, user, strlen(user)) < 0 )
      {
        perror( "escritura de socket" );
        return -1;
      }
    if ( read( sockfd, buffer, BUFF_MAX ) < 0 )
      {
        perror( "lectura de socket" );
        return -1;
      }
    if (strstr(buffer, "unknown") != NULL) /*no existe el usuario*/
        return -1 ;

    memset(buffer,'\0',BUFF_MAX);
    /* obtner passwrod */
    get_pass(buffer);

    if ( write( sockfd, buffer, strlen(buffer)) < 0 )
      {
        perror( "escritura de socket" );
        return -1;
      }

    if ( read( sockfd, buffer, BUFF_MAX ) < 0 )
      {
        perror( "lectura de socket" );
        return -1;
      }

    if (strstr(buffer, "accepted") != NULL) /*aceptado*/
        return 0;
    /* cualqier error*/
    return -1;
}

/**
 * obtiene la contraseña del prompt ocultado la misma.
 * @param buffer char* buffer obtiene la contraseña del prompt
 * @return 0 , si es obtiene la contraseña , error en caso contrario.
 */
int get_pass( char *buffer)
{
    char passwd[PASS_MAX];
    char *in = passwd;
    struct termios tty_orig;
    char c;
    int readbytes = 0;

    /*obtiene los parámetros asociados referidos stdin  y
     * los almacena en la estructura termios */
    tcgetattr( STDIN_FILENO, &tty_orig );
    struct termios  tty_work = tty_orig;

    memset(passwd,'\0',PASS_MAX);

    puts("# password:");
    tty_work.c_lflag &= ~( ECHO | ICANON );  /* | ISIG )*/
    tty_work.c_cc[ VMIN ]  = 1;
    tty_work.c_cc[ VTIME ] = 0;

    /* establece los parámetros asociados con el prompt  */
    tcsetattr( STDIN_FILENO, TCSAFLUSH, &tty_work );

    while (1)
      {
        if ((readbytes = read(STDIN_FILENO, &c, sizeof c) > 0))/* leer caracter de entrada */
          {
            if ('\n' == c)
                break;
            *in++ = c;
            /* ocultar password cabiar caracteres por "*" */
            write(STDOUT_FILENO, "*", 1);
          }
        else if (readbytes < 0)
          {
            perror("lectura de stdin");
            exit(EXIT_FAILURE);
          }
      }

    /* retorna  los parámetros asociados con el prompt  */
    tcsetattr( STDIN_FILENO, TCSAFLUSH, &tty_orig );

    *in = '\0';
    fputc('\n', stdout);
    strcpy(buffer, passwd);
    return 0;
}

/**
 * funcion tranferencia de archivo.
 * @param name char*, nombre el archivo a descargar.
 * @return int 0 , si concreta la descarga ,-1 en caso contrario
 */
int download (char* name)
{
    int     sockfd,
            readbytes;
    char    endflag[] = {"end-------"};
    char    ackflag[] = {"ack-------"};
    char    buffer[BUFFUDP_MAX];
    struct  sockaddr_in serv_addr;
    struct  timeval     timeout = {10, 0}; //set timeout for 10 seconds
    FILE*   fout;
    socklen_t   size_direccion;
    uint16_t    puerto = PORT_UDP;

    //unsigned  int fileSize;
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
      {
        perror("ERROR en apertura de socket");
        return -1;
      }

    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(struct timeval));

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

    size_direccion = sizeof( struct sockaddr );

    memset( buffer, 0, BUFFUDP_MAX );

    /* Comunicacion previa */
    if ( recvfrom( sockfd, buffer, BUFFUDP_MAX-1, 0, (struct sockaddr *)&serv_addr, &size_direccion ) < 0)
      {
        perror( "lectura de socket or timeout" );
          return -1 ;
      }
    if(strstr(buffer,"path") != buffer) /*el server requiere el nombre del archivo */
        return -1;

    printf("# Recibir archivo %s.\n", name);
    /* eviar el nombre del archivo a descargar */
    if (sendto(sockfd, (void *)name, strlen(name), 0, (struct sockaddr *)&serv_addr, size_direccion ) < 0)
      {
        perror( "escritura en socket" );
        exit(EXIT_FAILURE);
      }

    name[strlen(name)-1] = '\0';
    printf("# Downloading...\n");

    /* Abre el arivo o lo crea */
    fout = fopen(name,"wb");

    if(fout == NULL){
        perror("file open error");
        close(sockfd);
        return -1;
    }

    memset( buffer, 0, BUFFUDP_MAX );
    while ((readbytes = (int)recvfrom(sockfd, buffer, BUFFUDP_MAX, 0,(struct sockaddr *)&serv_addr,
            &size_direccion)) > 0)
      {
        if( strstr(buffer, endflag) == buffer)/* verificar si termino la descarga */
          {
            printf("# Descarga completa.\n");
            fclose(fout);
            close(sockfd);
            return 0;
          }
        /* escribir datos en el archivo */
        fwrite(buffer, 1, (size_t )readbytes,fout);
        memset( buffer, 0, BUFFUDP_MAX );

        if (sendto(sockfd, (void *) ackflag, strlen(ackflag), 0, (struct sockaddr *) &serv_addr, size_direccion) < 0)
          {
              perror("Escritura en socket");
              fclose(fout);
              exit(EXIT_FAILURE);
          }
          memset(buffer, 0, BUFFUDP_MAX);
      }
    if(readbytes < 0) /* error de lectura del socket o cumplir el timeout */
        perror( "lectura de socket or timeout" );
    close(sockfd);
    fclose(fout);
    /*cualquier otro error*/
    return -1;
}
