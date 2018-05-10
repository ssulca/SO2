#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <memory.h>
#include <math.h>

#define SAMPLES_MAX 800

#define GATE_MAX 500 /* 2 x 250 */

struct complex
{
    float phas;
    float quad;
};

double modulo(struct complex znum);

int main()
{

    long    file_size;
    long    ptr_buffer = 0;

    char    *buffer; /* buffer para recuperar los datos */

   // int     pulsos = 0; /* cantidad de pulsos */
    int     a_gates[SAMPLES_MAX]; /* vector de gates */
    int     pos_pulso[SAMPLES_MAX]; /* posicion en memoria de cada pulso */

    int     resto_add,
            gate_local,
            valids_count;

    double  cont_v = 0,
            cont_h = 0 ;

    double   pulsos_v_gate[SAMPLES_MAX][GATE_MAX];
    double   pulsos_h_gate[SAMPLES_MAX][GATE_MAX];

    struct complex muestra_z;
    //size_t  read_bytes;
    uint16_t valid_samples[SAMPLES_MAX];

    FILE    *filep;

    filep = fopen ("./pulsos.iq", "rb");
    if(filep == NULL)
    {
        perror("opening file ERROR");
        exit(EXIT_FAILURE);
    }

    fseek ( filep, 0L, SEEK_END ); /**/
    file_size = ftell ( filep );
    fseek ( filep, 0, SEEK_SET );

    buffer = (char *) malloc((size_t)file_size + 1);
    if ( buffer == NULL)
    {
        printf("# Memory error malloc! \n" );
        fclose (filep);
        exit(EXIT_FAILURE);
    }

    fread ( buffer, (size_t)file_size, 1, filep);

    for (int i = 0; ptr_buffer < file_size; i++)
    {
        pos_pulso[i] = ptr_buffer + sizeof(uint16_t);
        /* Obtengo cantidad de muestras */
        memmove ( &valid_samples[i], &buffer[ptr_buffer], sizeof(uint16_t) );
        /* actualizo puntero, 4 por F y Q de V y H */
        ptr_buffer += sizeof(uint16_t) + 4 * valid_samples[i] * sizeof(float);
        /* Calculo de Gate para cada pulso */
        a_gates[i] =  valid_samples[i] / GATE_MAX;
    }

    //printf("numero de pulsos %i\n",pulsos);
    fclose (filep);

    for (int index_pulso = 0; index_pulso < SAMPLES_MAX; index_pulso++)
    {
        resto_add = valid_samples[index_pulso] % GATE_MAX;
        ptr_buffer = pos_pulso[index_pulso];
        valids_count = 0;

        for (int  index_gate = 0; index_gate < GATE_MAX ; index_gate++)
        {
            if (resto_add  >= GATE_MAX)
            {
                gate_local = a_gates[index_pulso] + 1;
                resto_add -= GATE_MAX;
            }
            else {
                gate_local = a_gates[index_pulso];
            }

            cont_v = 0;
            cont_h = 0;

            for ( int i = 0; i < gate_local && valids_count < valid_samples[index_pulso]; i++)
            {
                memmove(&muestra_z, &buffer[ptr_buffer], sizeof (struct complex));
                cont_v += modulo(muestra_z);

                memmove(&muestra_z,
                        &buffer[ptr_buffer + valid_samples[index_pulso] * sizeof(struct complex)],
                        sizeof(struct complex));
                cont_h += modulo(muestra_z);

                ptr_buffer += sizeof(struct complex);
                valids_count ++;
            }
            pulsos_v_gate[index_pulso][index_gate] = cont_v / gate_local;
            pulsos_h_gate[index_pulso][index_gate] = cont_h / gate_local;
        }
    }


    free (buffer);
    //printf("Hello, World!\n");

    for (int j = 0; j < 10 ; ++j) {
        for (int i = 0; i < 10 ; ++i) {
            printf("h_M[puso %i][gate %i] = %f\n",j ,i , pulsos_h_gate[j][i]);
            printf("v_M[puso %i][gate %i] = %f\n",j ,i , pulsos_v_gate[j][i]);
        }
    }

    return 0;
}

double modulo(struct complex znum)
{
    return sqrt( pow(znum.phas,2) + pow(znum.quad, 2));
}