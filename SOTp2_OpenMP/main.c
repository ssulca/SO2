#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <memory.h>
#include <math.h>

#define PULSOS_MAX 800

#define GATE_MAX 500 /* 2 x 250 */
#define GRADOS_MAX 8

struct complex
{
    float phas;
    float quad;
};

double modulo(struct complex znum);

int main()
{

    long    file_size,
            ptr_buffer = 0;

    void    *buffer; /* buffer para recuperar los datos */

    int     a_gates[PULSOS_MAX], /* vector de gates */
            pos_pulso[PULSOS_MAX], /* posicion en memoria de cada pulso */
            resto,
            resto_add,
            gate_local,
            valids_count;

    double  pulsos_v_gate[PULSOS_MAX][GATE_MAX],
            pulsos_h_gate[PULSOS_MAX][GATE_MAX],
            cont_v,
            cont_h;

    double  autocorr_v[GATE_MAX],
            autocorr_h[GATE_MAX],
            sumador_v,
            sumador_h;

    struct complex muestra_z;

    uint16_t valid_samples[PULSOS_MAX];

    FILE    *filep;

    filep = fopen ("/home/sergio/CLionProjects/sulca/SOTp2_OpenMP/pulsos.iq", "rb");
    if(filep == NULL)
    {
        perror("# opening file ERROR");
        exit(EXIT_FAILURE);
    }

    fseek ( filep, 0L, SEEK_END );
    file_size = ftell ( filep );
    fseek ( filep, 0, SEEK_SET );

    buffer = (char *) malloc((size_t)file_size + 1);
    if ( buffer == NULL)
    {
        perror("# Memory error malloc! \n" );
        fclose (filep);
        exit(EXIT_FAILURE);
    }

    fread(buffer, (size_t)file_size, 1, filep);
    fclose(filep);

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

    for (int index_pulso = 0; index_pulso < PULSOS_MAX; index_pulso++)
    {
        resto = valid_samples[index_pulso] % GATE_MAX;
        ptr_buffer = pos_pulso[index_pulso];
        valids_count = 0;
        resto_add = 0;

        for (int  index_gate = 0; index_gate < GATE_MAX ; index_gate++)
        {
            resto_add += resto;
            if (resto_add  >= GATE_MAX)
            {
                gate_local = a_gates[index_pulso] + 1;
                resto_add -= GATE_MAX;
            }
            else
                gate_local = a_gates[index_pulso];

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

    for ( int i = 0; i < GATE_MAX  ; i++ )
    {
        sumador_v = 0;
        sumador_h = 0;

        for ( int j = 0; j < PULSOS_MAX - 1; j++ )
        {
            sumador_v += pulsos_v_gate[i][j] * pulsos_v_gate[i][j + 1];
            sumador_h += pulsos_h_gate[i][j] * pulsos_h_gate[i][j + 1];
        }
        autocorr_v[i] = sumador_v / PULSOS_MAX;
        autocorr_h[i] = sumador_h / PULSOS_MAX;
    }

    for (int k = 0; k < 10; k++) {
        printf("correlacion_h[%i]= %lf\n", k , autocorr_h[k]);
        printf("correlacion_v[%i]= %lf\n", k , autocorr_v[k]);
    }

    return 0;
}

double modulo(struct complex znum)
{
    return sqrt( pow(znum.phas,2) + pow(znum.quad, 2));
}