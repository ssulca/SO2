//
// Created by sergio on 6/05/18.
//
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <memory.h>
#include <math.h>
#include <omp.h>

#define ALL_PULSOS 800
#define PULSOS 100
#define GATE_MAX 500 /* 2 x 250 */
#define GRADOS 8

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

    char    *buffer; /* buffer para recuperar los datos */

    int     a_gates[ALL_PULSOS], /* vector de gates */
            pos_pulso[ALL_PULSOS], /* posicion en memoria de cada pulso */
            resto,
            resto_add,
            gate_local,
            valids_count;

    int     grado_aux = 82; /* aux para almacenar grado */

    double  pulsos_v_gate[ALL_PULSOS][GATE_MAX],
            pulsos_h_gate[ALL_PULSOS][GATE_MAX],
            cont_v,
            cont_h;

    double  autocorr_v[GRADOS][GATE_MAX], /* autocorrelacion del canal horizontal*/
            autocorr_h[GRADOS][GATE_MAX], /* autocorrelacion del canal vertical*/
            sumador_v,
            sumador_h;

    struct complex muestra_z; /* estructura para obtener los datos de una muestra */

    uint16_t valid_samples[ALL_PULSOS]; /* vector de todos los valid samples*/

    FILE    *filein; /* Puntero de archivo de lectura*/
    FILE    *fileout; /* Puntero de archivo de escritura pos-processamiento*/

    filein = fopen ("./pulsos.iq", "rb");
    if(filein == NULL)
    {
        perror("# opening file ERROR");
        exit(EXIT_FAILURE);
    }

    /* obtengo el tam del archivo en bytes */
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

    /* lectura total del archivo */
    fread(buffer, (size_t)file_size, 1, filein);
    fclose(filein);

    for (int i = 0; ptr_buffer < file_size; i++)
    {
        pos_pulso[i] = ptr_buffer + sizeof(uint16_t); /* Guarda la posicion en memoria de los pulsos */
        memmove ( &valid_samples[i], &buffer[ptr_buffer], sizeof(uint16_t) ); /* Obtengo cantidad de muestras */

        /* actualizo puntero, 4 por F y Q de V y H */
        ptr_buffer += sizeof(uint16_t) + 4 * valid_samples[i] * sizeof(float);
        a_gates[i] =  valid_samples[i] / GATE_MAX; /* Calculo de Gate tentativo para cada pulso */
    }

    #pragma omp parallel for private(resto, ptr_buffer,valids_count,resto_add,gate_local, cont_v, cont_h) \
    shared(valid_samples,pos_pulso, buffer, pulsos_v_gate, pulsos_h_gate)
    for (int idx_puls = 0; idx_puls < ALL_PULSOS; idx_puls++)
    {
        resto = valid_samples[idx_puls] % GATE_MAX; /* calculo el resto de cada gate */
        ptr_buffer = pos_pulso[idx_puls];
        valids_count = 0; /* contador de muestras */
        resto_add = 0;

        for (int  idx_gate = 0; idx_gate < GATE_MAX ; idx_gate++)
        {
            resto_add += resto; /* acumula el resto */
            if (resto_add  >= GATE_MAX) /* si la acumulado es mayor al divisor se incremeta la cantidad */
            {                           /* de muestras a tomar */
                gate_local = a_gates[idx_puls] + 1;
                resto_add -= GATE_MAX;
            }
            else /* se mantiene el nuemro de muestras tentativos */
                gate_local = a_gates[idx_puls];

            cont_v = 0;
            cont_h = 0;

            for (int i = 0; i < gate_local && valids_count < valid_samples[idx_puls]; i++)
            {
                memmove(&muestra_z, &buffer[ptr_buffer], sizeof(struct complex)); /* obtencion de muestra */
                cont_v += modulo(muestra_z); /*se acumulan muestras en modulo para el canal v*/

                memmove(&muestra_z,
                        &buffer[ptr_buffer + valid_samples[idx_puls] * sizeof(struct complex)],
                        sizeof(struct complex));
                cont_h += modulo(muestra_z); /*se acumulan muestras en modulo para el canal h*/

                ptr_buffer += sizeof(struct complex);
                valids_count++;
            }
            /* obtencion de gates por pulso (media aritmetica) */
            pulsos_v_gate[idx_puls][idx_gate] = cont_v / gate_local;
            pulsos_h_gate[idx_puls][idx_gate] = cont_h / gate_local;
        }
    }

    free (buffer);

    /* calculo de la autocorrelacion para cada grado_gate  */
    #pragma omp parallel for private(sumador_v, sumador_h) shared(autocorr_v, autocorr_h, pulsos_h_gate, pulsos_v_gate)
    for (int idx_grado = 0; idx_grado < GRADOS; idx_grado++)
    {
        for ( int idx_gate = 0; idx_gate < GATE_MAX  ; idx_gate++ )
        {
            sumador_v = 0;
            sumador_h = 0;

            for ( int idx_pulso = 0; idx_pulso < (PULSOS - 1) ; idx_pulso++ )
            {
                sumador_v += pulsos_v_gate[(PULSOS * idx_grado) + idx_pulso][idx_gate] *
                             pulsos_v_gate[(PULSOS * idx_grado) + idx_pulso + 1][idx_gate];

                sumador_h += pulsos_h_gate[(PULSOS * idx_grado) + idx_pulso][idx_gate] *
                             pulsos_h_gate[(PULSOS * idx_grado) + idx_pulso + 1][idx_gate];
            }
            autocorr_v[idx_grado][idx_gate] = sumador_v / 100.0;
            autocorr_h[idx_grado][idx_gate] = sumador_h / 100.0;
        }
    }


    fileout = fopen("./proccess.outln","wb");

    for (int idx_grado = 0; idx_grado < GRADOS; idx_grado++)
    {
        grado_aux ++;
        fwrite(&grado_aux, sizeof(int), 1,fileout); /*se escribe en numero de grado al que pertenece los datos */
        fwrite(autocorr_h[idx_grado], sizeof(double), GATE_MAX, fileout); /*gates del canar h*/
        fwrite(autocorr_v[idx_grado], sizeof(double), GATE_MAX, fileout); /*gates del canar v*/
    }
    fclose(fileout);
    return 0;
}

/**
 * funcion para calular el modulo de un numero complejo
 * @param znum struct complex, numero complejo,
 * @return  double, el modulo del numero complejo.
 */
double modulo(struct complex znum)
{
    return sqrt( pow(znum.phas,2) + pow(znum.quad, 2));
}