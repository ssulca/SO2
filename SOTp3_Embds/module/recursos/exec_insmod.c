#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int main(int argc, char const *argv[])
{
	char arg[64];
	char *path[] = {"/sbin/insmod",NULL,NULL};

	path[1] = arg; /* redirecciono el puntero al argumento */
	strcpy(arg,argv[1]); /* copio el nuevo path*/

    execvp(path[0],path);
    perror("Exec error \n");
    exit(1);
}