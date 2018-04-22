#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <pwd.h>
#include <crypt.h>
#include <shadow.h>
#include <utmp.h>
#include <printf.h>

#define BUFF_MAX 256
#define PASS_MAX 16
//connect desk@192.168.1.10:6020
int main()
{
    /* inicializa el arreglo de cadena */
    char cadena[] = "Este es un enunciado con 7 tokens";
    char *ptrToken; /* crea un apuntador char */

    printf( "%s\n", cadena);

    ptrToken = strtok( cadena, " " ); /* comienza la divisiÃƒÂ³n en tokens del enunciado */
    ptrToken = strtok( NULL, " " );
    printf( "%s\n", ptrToken );

    return 0; /* indica terminaciÃƒÂ³n exitosa */

}
/*
int main2() {
    //char prompt[] = {"connect desk@192.168.1.10:6020"};
    char prompt[BUFF_MAX];
    char* user;
    char* ip;
    char* pueto;

    printf("prompt > ");
    fgets(prompt,BUFF_MAX, stdin);
    if (strstr(prompt, "connect") == NULL){
        perror("Comando no reconocido");
        exit(1);
    }
    user = strstr(prompt, " " );
    if (user == NULL){
        perror("usuario");
        exit(1);
    }
    user = user+1;
    ip = strstr(prompt, "@" );
    if(ip == NULL){
        perror("ip");
        exit(1);
    }
    pueto = strstr(prompt, ":" );
    if(pueto == NULL){
        perror("puerto");
        exit(1);
    }

    *ip = '\0';
    *pueto = '\0';

    ip = ip+1;
    pueto = pueto+1;

    printf("%s\n",user);
    printf("%s\n",ip);
    printf("%s\n",pueto);
    return 0;
}
*/