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

int process(const uint16_t *valid_samples, const int * pos_pulso, const int * a_gates,
            double pulsos_v_gate[ALL_PULSOS][GATE_MAX], double pulsos_h_gate[ALL_PULSOS][GATE_MAX],
            const char * buffer);

int correlacion(double autocorr_v[GRADOS][GATE_MAX],double  autocorr_h[GRADOS][GATE_MAX],
                double pulsos_v_gate[ALL_PULSOS][GATE_MAX], double pulsos_h_gate[ALL_PULSOS][GATE_MAX]);

int main( int argc, char *argv[] )
{
    int     file_size,
            ptr_buffer = 0;

    char    *buffer; /* buffer para recuperar los datos */

    int     a_gates[ALL_PULSOS], /* vector de gates */
            pos_pulso[ALL_PULSOS]; /* posicion en memoria de cada pulso */

    int     grado_aux = 82; /* aux para almacenar grado */
    int     thilo = 0;

    double  pulsos_v_gate[ALL_PULSOS][GATE_MAX],
            pulsos_h_gate[ALL_PULSOS][GATE_MAX];

    double  autocorr_v[GRADOS][GATE_MAX], /* autocorrelacion del canal horizontal*/
            autocorr_h[GRADOS][GATE_MAX]; /* autocorrelacion del canal vertical*/

    uint16_t valid_samples[ALL_PULSOS]; /* vector de todos los valid samples*/

    FILE    *filein; /* Puntero de archivo de lectura*/
    FILE    *fileout; /* Puntero de archivo de escritura pos-processamiento*/

    double start = omp_get_wtime();
    double tock;

    if(argc > 1)
        thilo = (int) strtol(argv[1],NULL,10);

    if (thilo == 0)
        thilo = 1;

    omp_set_num_threads(thilo);

    filein = fopen ("./pulsos.iq", "rb");
    if(filein == NULL)
      {
        perror("# opening file ERROR");
        exit(EXIT_FAILURE);
      }

    /* obtengo el tam del archivo en bytes */
    fseek ( filein, 0L, SEEK_END );
    file_size = (int)ftell ( filein );
    fseek ( filein, 0, SEEK_SET );

    buffer = (char *) calloc(1, (size_t)file_size + 1);
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

    tock = omp_get_wtime();
    printf("# Lectura = %lfs\n",tock-start);
    /* procesameiento para obtener la matriz pulso gate */
    tock = omp_get_wtime();

    process(valid_samples, pos_pulso, a_gates, pulsos_v_gate, pulsos_h_gate, buffer);
    free (buffer);

    printf("# Process = %lfs\n",omp_get_wtime() - tock);

    /* calculo de la autocorrelacion para cada grado_gate  */
    tock = omp_get_wtime();

    correlacion(autocorr_v, autocorr_h ,pulsos_v_gate , pulsos_h_gate);

    printf("# Correlacion = %lfs\n",omp_get_wtime() - tock);

    tock = omp_get_wtime();

    fileout = fopen("./proccess.outln","wb");
    for (int idx_grado = 0; idx_grado < GRADOS; idx_grado++)
      {
        grado_aux ++;
        fwrite(&grado_aux, sizeof(int), 1,fileout); /*se escribe en numero de grado al que pertenece los datos */
        fwrite(autocorr_h[idx_grado], sizeof(double), GATE_MAX, fileout); /*gates del canar h*/
        fwrite(autocorr_v[idx_grado], sizeof(double), GATE_MAX, fileout); /*gates del canar v*/
      }
    fclose(fileout);
    printf("# Escitura = %lfs\n",omp_get_wtime() - tock);
    printf("# Total = %lfs\n",omp_get_wtime() - start);
    return 0;
}

/**
 * procesameiento para obtener la matriz pulso gate
 * @param valid_samples uiint16_t, vector que contiene los valid samples de cada pulso.
 * @param pos_pulso int , vector posicon en memoria de cada pulso
 * @param a_gates int , vector que contiene la cantindad de  gate tentativos
 * @param pulsos_v_gate int, matriz que contiene la pulsos por pulso del canal v
 * @param pulsos_h_gate int, matriz que contiene la pulsos por pulso del canal h
 * @param buffer
 * @return
 */
int process(const uint16_t *valid_samples, const int * pos_pulso, const int * a_gates,
            double pulsos_v_gate[ALL_PULSOS][GATE_MAX], double pulsos_h_gate[ALL_PULSOS][GATE_MAX],
            const char * buffer)
{
    int resto = 0,
        resto_add = 0,
        gate_local = 0,
        valids_count = 0,
        ptr_buffer = 0;

    double  cont_v = 0,
            cont_h = 0;

    struct complex muestra_z; /* estructura para obtener los datos de una muestra */

    #pragma omp parallel for private(resto, ptr_buffer,valids_count,resto_add,gate_local) \
    reduction(+: cont_v, cont_h)
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
                cont_v += sqrt(pow(muestra_z.phas,2) + pow(muestra_z.quad, 2)); /*modulo para el canal v*/

                memmove(&muestra_z,
                        &buffer[ptr_buffer + valid_samples[idx_puls] * sizeof(struct complex)],
                        sizeof(struct complex));
                cont_h += sqrt(pow(muestra_z.phas,2) + pow(muestra_z.quad, 2)); /* modulo para el canal h*/

                ptr_buffer += sizeof(struct complex);
                valids_count++;
              }
            /* obtencion de gates por pulso (media aritmetica) */
            pulsos_v_gate[idx_puls][idx_gate] = cont_v / gate_local;
            pulsos_h_gate[idx_puls][idx_gate] = cont_h / gate_local;
          }
      }
    return 0;
}

/**
 * calculo de la autocorrelacion para cada grado_gate
 * @param autocorr_v double, matriz en donde se almacena la acurrelacion del canal v
 * @param autocorr_h double, matriz en donde se almacena la acurrelacion del canal h
 * @param pulsos_v_gate int, matriz que contiene la pulsos por pulso del canal v
 * @param pulsos_h_gate int, matriz que contiene la pulsos por pulso del canal h
 * @return 0
 */
int correlacion(double autocorr_v[GRADOS][GATE_MAX],double  autocorr_h[GRADOS][GATE_MAX],
                double pulsos_v_gate[ALL_PULSOS][GATE_MAX], double pulsos_h_gate[ALL_PULSOS][GATE_MAX])
{
    double  sumador_v = 0,
            sumador_h = 0;

    #pragma omp parallel for reduction(+:sumador_v, sumador_h)
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
            autocorr_v[idx_grado][idx_gate] = sumador_v / PULSOS;
            autocorr_h[idx_grado][idx_gate] = sumador_h / PULSOS;
          }
      }

    return 0;
}
