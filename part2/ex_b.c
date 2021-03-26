#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "Lista_ligada.h"

void addValueToHistogram(int *histograma, double value);
void writeToFileUnformatted(FILE *file_pointer, double value);
double generateNewEvent(lista *eventos, lista *waiting, int lambda, int isDelayed);
double getTime(lista *apontador, int position);

#define DEBUG 0
#define dm 0.008
#define true 1
#define false 0
#define null NULL

int main(int argc, char *argv[])
{
    int sample_counter = 0;
    int busy = 0, delay_counter = 0, processed_delays = 0, Ax_counter = 0;
    int lambda, amostras, n_channels;
    int DELAY_FLAG = false;

    double Ax = 0;
    double current_time = 0.0;
    double sum_delay = 0.0;
    double c;
    double delays[1000] = {0.0};

    FILE *fp, *fp2;
    lista *eventos, *waiting, *delay_to_process;

    if (argc < 4)
    {
        printf("Usage: [progname] [lambda] [N canais] [numero de amostras] [delay]\n");
        return 0;
    }

    lambda = atoi(argv[1]);
    n_channels = atoi(argv[2]);
    amostras = atoi(argv[3]);
    Ax = atof(argv[4]);

    if (DEBUG)
        printf("Lambda = %d\nAmostras = %d\nN Canais = %d\n", lambda, amostras, n_channels);

    srand(time(null));

    fp = fopen("part2_a_log.txt", "w");
    fp2 = fopen("delays.txt", "w");
    
    waiting = adicionar(null, 0, 0);

    //Adicionar o primeiro evento no tempo = 0
    eventos = adicionar(null, CHEGADA, current_time);
    busy++;

    generateNewEvent(eventos, waiting, lambda, DELAY_FLAG);

    while (sample_counter < amostras)
    {
        if (DEBUG)
        {
            imprimir(eventos);
            printf("Busy: %d\n", busy);
        }
        //Remove o evento anterior e coloca o novo
        eventos = remover(eventos);

        switch(eventos->tipo)
        {
            case CHEGADA:
                sample_counter++;
                busy++;
                if (busy > n_channels)
                {
                    busy = n_channels;
                    delay_counter++;
                    DELAY_FLAG = true;
                }
                c = generateNewEvent(eventos, waiting, lambda, DELAY_FLAG);
                DELAY_FLAG=false;

                //writeToFileUnformatted(fp, c);
                break;
            case PARTIDA:
                //TODO: a dif entre tempo do pacote e a average se > 0 delayed++ e P(A>0)=delayed/amostras
                if (processed_delays < delay_counter) 
                {
                    double delay_time;
                    double queue_placed_time = 0.0;
                    
                    queue_placed_time = getTime(waiting, processed_delays);
                    delay_time = eventos->tempo - queue_placed_time;

                    if (delay_time > Ax)
                        Ax_counter++;

                    delays[processed_delays] = delay_time;
                    sum_delay += delays[processed_delays];
                    writeToFileUnformatted(fp2, delays[processed_delays]);
                    adicionar(eventos, PARTIDA, eventos->tempo - dm * log((double)random() / RAND_MAX));
                    processed_delays++;
                    busy++;

                    if (DEBUG)
                        printf("tempo: %lf, proc_delay: %d\n", queue_placed_time, processed_delays);
                }
                    
                busy--;
                break;
        }
        
        if (DEBUG)
        {
            printf("proccessed: %d \tdelay_counter: %d\n", processed_delays, delay_counter);
        }
    }

    if (DEBUG)
    {
        printf("WAITING\n");
        imprimir(waiting);
    }
    
    current_time = eventos->tempo;

    //Calcular valor médio entre chegada de eventos
    printf("Valor médio entre chegada de eventos: %.6f\n", current_time / ((double)amostras));
    printf("Delays: %d\n", delay_counter);
    printf("Ax: %d\n", Ax_counter);
    printf("Probabilidade de atraso de pacote: %.3f\n", (double)delay_counter / (double)amostras);
    //Calcular media de atrasos dos pacotes
    printf("Media de atraso de pacote: %.6f\n", sum_delay / (double)amostras);

    printf("Probablidade do atraso do pacote ser maior que %.3f: %.3f\n", Ax, (double)Ax_counter/(double)delay_counter);
    fclose(fp);
    return 0;
}

//Usado para criar um histograma no Excel.
//Envia para o ficheiro apontado por file_pointer os valores de c separados por espaços
void writeToFileUnformatted(FILE *file_pointer, double value)
{
    char value_char[100];
    sprintf(value_char, "%.6f", value);
    fputs(value_char, file_pointer);
    fputs(" ", file_pointer);
}

/*Gera uma partida e uma chegada a partir da chegada recebida
  Retorna o valor de c
*/
double generateNewEvent(lista *eventos, lista *waiting, int lambda, int isDelayed)
{
    double c, d, u1, u2;
    double current_time;

    //Gerar numero aleatório entre 0 e 1
    u1 = (double)random() / RAND_MAX;
    u2 = (double)random() / RAND_MAX;

    //Calcular duracao do pacote
    d = -dm * log(u2);

    //Calcular intervalo entre chegada de eventos
    c = (-1 / (double)lambda) * log(u1);
        
    current_time = eventos->tempo;

    //Gerar partida deste evento e chegada do proximo
    if (isDelayed)
        adicionar(waiting, 0, current_time);
    else
        adicionar(eventos, PARTIDA, current_time + d);
        

    adicionar(eventos, CHEGADA, current_time + c);

    return c;
}

double getTime(lista *apontador, int position)
{   
    int i = 0;
    double time;
    while (apontador != NULL)
    {
        time = apontador->tempo;
        apontador = (lista *)apontador->proximo;

        if (i > position)
            break;
        i++;
    }
    return time;
}