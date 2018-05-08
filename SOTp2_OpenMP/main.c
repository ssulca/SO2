#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <memory.h>

#define SAMPLES_MAX 800

#define GATE_DENOM 500 /* 2 x 250 */

int main() {

    long    file_size;
    long    ptr_buffer = 0;

    char    *buffer; /* buffer para recuperar los datos */

    int     pulsos = 0; /* cantidad de pulsos */
    int     a_gates[SAMPLES_MAX]; /* vector de gates */
    int     pos_pulso[SAMPLES_MAX]; /* posicion en memoria de cada pulso */
    int     gate_resto[SAMPLES_MAX];

    size_t  read_bytes;
    uint16_t valid_samples[SAMPLES_MAX];

    FILE    *filep;

    filep = fopen ("./pulsos.iq", "rb");
    if(filep == NULL){
        perror("opening file ERROR");
        exit(EXIT_FAILURE);
    }

    fseek ( filep, 0L, SEEK_END ); /**/
    file_size = ftell ( filep );
    fseek ( filep, 0, SEEK_SET );

    buffer = (char *) malloc((size_t)file_size + 1);
    if ( buffer == NULL) {
        printf("# Memory error malloc! \n" );
        fclose (filep);
        exit(EXIT_FAILURE);
    }

    fread ( buffer, (size_t)file_size, 1, filep);
    /*if (read_bytes != file_size)
    {
        printf ("ERROR LEYENDO ARCHIVO \n");
        free (buffer);
        fclose (filep);
        exit(EXIT_FAILURE);
    }*/

    for ( int i = 0; ptr_buffer < file_size; i++ ) {
        pos_pulso[i] = ptr_buffer + sizeof(uint16_t);
        /* Obtengo cantidad de muestras */
        memmove ( &valid_samples[i], &buffer[ptr_buffer], sizeof(uint16_t) );
        /* actualizo puntero, 4 por F y Q de V y H */
        ptr_buffer += sizeof(uint16_t) + 4 * valid_samples[i] * sizeof(float);
        /* Calculo de Gate para cada pulso */
        a_gates[i] =  valid_samples[i] / GATE_DENOM;
        gate_resto[i] = valid_samples[i] % GATE_DENOM;
        pulsos ++;
    }

    printf("numero de pulsos %i\n",pulsos);
    fclose (filep);
    //printf("Hello, World!\n");


    free (buffer);
    for(int i = 0; i<10; i++){
        printf("valids[%i] = %i\n",i,valid_samples[i]);
    }
    for(int i = 0; i<10; i++){
        printf("gates[%i] = %i , resto = %i\n",i,a_gates[i], gate_resto[i]);
    }
    return 0;
}