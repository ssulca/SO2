//
// Created by sergio on 12/05/18.
//

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <memory.h>

int main()
{

    long   file_size,
           ptr_buffer = 0;

    double  coorre[500]; /*correalacion del canal h del angulo 83*/
    char    *buffer;

    int correlacion_pos[8];
    int angulos[8];

    FILE    *filein;

    filein = fopen ("./proccess.outln", "rb");
    if(filein == NULL)
    {
        perror("# opening file ERROR");
        exit(EXIT_FAILURE);
    }

    fseek ( filein, 0L, SEEK_END );
    file_size = ftell ( filein );
    fseek ( filein, 0, SEEK_SET );

    buffer = (char *) malloc((size_t)file_size + 1);
    if ( buffer == NULL)
    {
        perror("# Memory error malloc! \n" );
        fclose (filein);
        exit(EXIT_FAILURE);
    }

    fread(buffer, (size_t)file_size, 1, filein);
    fclose(filein);

    for (int i = 0; ptr_buffer < file_size; i++)
    {
        correlacion_pos[i] = ptr_buffer + sizeof(int);
        memmove ( &angulos[i], &buffer[ptr_buffer], sizeof(int));
        ptr_buffer += sizeof(int) + 2*500 * sizeof(double);
    }

    for (int j = 0; j < 8; ++j) {
        printf("angulo %i\n",angulos[j]);
    }
    ptr_buffer = correlacion_pos[0];
    for ( int i = 0; i < 500 ; i++)
    {
        memmove(&coorre[i], &buffer[ptr_buffer], sizeof(double));
        ptr_buffer += sizeof(double);
        printf("coore = %lf\n",coorre[i]);
    }

    return 0;
}