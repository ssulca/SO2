#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <memory.h>
#include <math.h>

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

    double  pulsos_v_gate[ALL_PULSOS][GATE_MAX],
            pulsos_h_gate[ALL_PULSOS][GATE_MAX],
            cont_v,
            cont_h;

    double  autocorr_v[GRADOS][GATE_MAX],
            autocorr_h[GRADOS][GATE_MAX],
            sumador_v,
            sumador_h;

    struct complex muestra_z;

    uint16_t valid_samples[ALL_PULSOS];

    FILE    *filein;
    FILE    *fileout;

    filein = fopen ("/home/sergio/CLionProjects/sulca/SOTp2_OpenMP/pulsos.iq", "rb");
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
        pos_pulso[i] = ptr_buffer + sizeof(uint16_t);
        /* Obtengo cantidad de muestras */
        memmove ( &valid_samples[i], &buffer[ptr_buffer], sizeof(uint16_t) );
        /* actualizo puntero, 4 por F y Q de V y H */
        ptr_buffer += sizeof(uint16_t) + 4 * valid_samples[i] * sizeof(float);
        /* Calculo de Gate para cada pulso */
        a_gates[i] =  valid_samples[i] / GATE_MAX;
    }

    for (int idx_puls = 0; idx_puls < ALL_PULSOS; idx_puls++)
    {
        resto = valid_samples[idx_puls] % GATE_MAX;
        ptr_buffer = pos_pulso[idx_puls];
        valids_count = 0;
        resto_add = 0;

        for (int  idx_gate = 0; idx_gate < GATE_MAX ; idx_gate++)
        {
            resto_add += resto;
            if (resto_add  >= GATE_MAX)
            {
                gate_local = a_gates[idx_puls] + 1;
                resto_add -= GATE_MAX;
            }
            else
                gate_local = a_gates[idx_puls];

            cont_v = 0;
            cont_h = 0;

            for ( int i = 0; i < gate_local && valids_count < valid_samples[idx_puls]; i++)
            {
                memmove(&muestra_z, &buffer[ptr_buffer], sizeof (struct complex));
                cont_v += modulo(muestra_z);

                memmove(&muestra_z,
                        &buffer[ptr_buffer + valid_samples[idx_puls] * sizeof(struct complex)],
                        sizeof(struct complex));
                cont_h += modulo(muestra_z);

                ptr_buffer += sizeof(struct complex);
                valids_count ++;
            }
            pulsos_v_gate[idx_puls][idx_gate] = cont_v / gate_local;
            pulsos_h_gate[idx_puls][idx_gate] = cont_h / gate_local;
        }
    }

    free (buffer);

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

    int aux = 0;
    fileout = fopen("./proccess.outln","wb");
    for (int idx_grado = 0; idx_grado < GRADOS; idx_grado++)
    {
        aux = idx_grado + 83;
        fwrite(&aux, sizeof(int), 1,fileout);
        fwrite(autocorr_h[idx_grado], sizeof(double), GATE_MAX, fileout);
        fwrite(autocorr_v[idx_grado], sizeof(double), GATE_MAX, fileout);

    }
    fclose(fileout);
    return 0;
}

double modulo(struct complex znum)
{
    return sqrt( pow(znum.phas,2) + pow(znum.quad, 2));
}