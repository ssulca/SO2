#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define BUFSIZE 256
//connect desk@192.168.1.10:6020
int main() {
    //char prompt[] = {"connect desk@192.168.1.10:6020"};
    char prompt[BUFSIZE];
    char* user;
    char* ip;
    char* pueto;

    printf("prompt > ");
    fgets(prompt,BUFSIZE, stdin);
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
