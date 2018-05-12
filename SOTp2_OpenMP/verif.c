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
    char * buffer;
    int  correlacion_pos[8];
    int angulos[8];
    FILE    *filein;
    FILE    *fileout;

    filein = fopen ("/home/sergio/CLionProjects/sulca/SOTp2_OpenMP/proccess.outln", "rb");
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
        /* Obtengo cantidad de muestras */
        memmove ( &angulos[i], &buffer[ptr_buffer], sizeof(int));
        /* actualizo puntero, 4 por F y Q de V y H */
        ptr_buffer += sizeof(int) + 2*500 * sizeof(double);
        /* Calculo de Gate para cada pulso */
    }
    for (int j = 0; j < 8; ++j) {
        printf("angulo %i\n",angulos[j]);
    }
    return 0;
}