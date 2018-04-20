#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <termios.h>

#define SIZEBUFFER 256
#define SIZEPASS 16
//connect desk@192.168.1.10:6020
int main()
{
    char passwd[SIZEPASS];
    char *in = passwd;
    struct termios tty_orig;
    char c;

    tcgetattr( STDIN_FILENO, &tty_orig );
    struct termios  tty_work = tty_orig;

    puts("Please input password:");
    tty_work.c_lflag &= ~( ECHO | ICANON );  // | ISIG );
    tty_work.c_cc[ VMIN ]  = 1;
    tty_work.c_cc[ VTIME ] = 0;
    tcsetattr( STDIN_FILENO, TCSAFLUSH, &tty_work );

    while (1) {
        if (read(STDIN_FILENO, &c, sizeof c) > 0) {
            if ('\n' == c) {
                break;
            }
            *in++ = c;
            write(STDOUT_FILENO, "*", 1);
        }
    }

    tcsetattr( STDIN_FILENO, TCSAFLUSH, &tty_orig );
    *in = '\0';
    printf("\n%s\n",passwd);

    read(STDIN_FILENO, passwd, SIZEPASS);
    printf("%s\n",passwd);
    // if you want to see the result:
    // printf("Got password: %s\n", passwd);

    return 0;
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
