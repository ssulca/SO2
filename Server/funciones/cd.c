//
// Created by tincho on 07/09/17.
//

#include <unistd.h>//para hostname y user name
#include <stdio.h>
#define BUFSIZE 1024

char *bash_cdHome(char *PATH);

int bash_cd(char **PATH)
{
    if (PATH[0] == NULL) {
        PATH[0]=getpwuid(geteuid ())->pw_dir;
    }
    else if(strstr( PATH[0],"~/" )!='\0') {
            PATH[0]=bash_cdHome(strstr( PATH[0],"~/" )+1);
        }

    if (chdir(PATH[0]) != 0) {
            perror("bash");
    }

    //  getpwuid(geteuid ())->pw_dir
    return 1;
}

char *bash_cdHome(char *PATH){
    int bufsize = BUFSIZE;
    char *buffer = malloc(sizeof(char) * bufsize);
    strcpy(buffer,getpwuid(geteuid ())->pw_dir);//obtengo el /home/userX
    strncat(buffer, PATH, bufsize);
    return buffer;
}
