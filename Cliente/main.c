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

#define SIZEBUFFER 256
#define SIZEPASS 16
//connect desk@192.168.1.10:6020
int main(){
    struct passwd *pwd;
    pwd = getpwuid(geteuid());
    char user[16];
    memset(user,'\0',16);
    read(STDIN_FILENO,user,15);
    user[strlen(user)-1] ='\0';
    printf("%s\n",user);

    printf("%s\n",pwd->pw_name);


    if(strcmp(pwd->pw_name,user)==0)
        printf("exito\n");
    else
        printf("nbas\n");
}

int main2() {
    //char prompt[] = {"connect desk@192.168.1.10:6020"};
    char prompt[SIZEBUFFER];
    char* user;
    char* ip;
    char* pueto;

    printf("prompt > ");
    fgets(prompt,SIZEBUFFER, stdin);
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
