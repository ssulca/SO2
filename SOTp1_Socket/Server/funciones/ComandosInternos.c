//
// Created by tincho on 16/09/17.
//
int comandosInternos(char** args)
{
    if (strcmp(args[0], "exit") == 0)
        return 1;
    /* ejecuto el comando interno cd */
    if (strcmp(args[0], "cd") == 0)
    {
        bash_cd(&args[1]);
        return 2;
    }
    return 0;
}