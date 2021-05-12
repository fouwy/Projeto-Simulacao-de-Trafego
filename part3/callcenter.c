/**
 * Lambda = 80 calls/hour
 * 30% general purpose / 70% area-specific
 * 
 * GENERAL PURPOSE:
 * Exponential distribution with average of 2 min
 * Min duration -> 1 min
 * Max duration -> 5 min
 * If not in limits, re-roll
 *  
 * AREA-SPECIFIC:
 * Before being area-specific, the call is general purpose with these params:
 * Gaussian distribution with average of 1 min and std dev = 20 sec
 * Min duration -> 30 sec
 * Max duration -> 120 sec
 * ----
 * After being transferd to area-specific:
 * Exponential distribution with average of 2 min 30 sec
 * Min duration -> 1 min
 * Max duration -> No Max
 * 
 * 
 * QUEUES:
 * General purposes has a waiting queue with finite lenght
 * Area-specific has infinite lenght waiting queue
 * 
 * Prediction of the average waiting time in the queue by using running
 * average of the time spent waiting by other callers
 **/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "Lista_ligada.h"

void addValueToHistogram(int *histograma, double value);
void writeToFileUnformatted(FILE *file_pointer, double value);
double generateNewEvent(lista *eventos, lista *waiting, int lambda, int isDelayed, int amountInQueue);
double getTime(lista *apontador, int position);
double getCallDuration(int type);
int getCallType();

#define DEBUG 0
#define true 1
#define false 0
#define null NULL

int queue_size = 0;

int main(int argc, char *argv[])
{
    int sample_counter = 0;
    int busy = 0, delay_counter = 0, block_counter = 0, processed_delays = 0, Ax_counter = 0;
    int lambda, amostras, n_channels;
    int DELAY_FLAG = false;

    double Ax = 0;                  
    double current_time = 0.0;          
    double sum_delay = 0.0;
    double c;
    double delays[2000] = {0.0};

    FILE *fp, *fp2;
    lista *eventos, *waiting, *delay_to_process;
    lista *transfered = null;

    //AREA SPECIFIC
    int delay_areaSpec_counter, areaSpec_processed_delays;
    int areaSpec_busy, n_areaSpec_channels;

    if (argc < 6)
    {
        printf("Usage: [progname] [lambda] [N canais] [numero de amostras] [delay] [tamanho da fila]\n");
        return 0;
    }

    lambda = atoi(argv[1]);
    n_channels = atoi(argv[2]);
    amostras = atoi(argv[3]);
    Ax = atof(argv[4]);
    queue_size = atoi(argv[5]);

    if (DEBUG)
        printf("Lambda = %d\nAmostras = %d\nN Canais = %d\n", lambda, amostras, n_channels);

    srand(time(null));

    fp = fopen("part2_a_log.txt", "w");
    fp2 = fopen("delays.txt", "w");
    
    waiting = adicionar(null, 0, 0);

    //Adicionar o primeiro evento no tempo = 0
    eventos = adicionar(null, CHEGADA, current_time);
    busy++;

    generateNewEvent(eventos, waiting, lambda, DELAY_FLAG, null);

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
            int amountInQueue;
            case CHEGADA:
                amountInQueue = delay_counter - processed_delays;

                sample_counter++;
                busy++;

                if (busy > n_channels)
                {
                    busy = n_channels;

                    if (amountInQueue < queue_size)
                        delay_counter++;
                    else
                        block_counter++;
                        
                    DELAY_FLAG = true;
                }
                c = generateNewEvent(eventos, waiting, lambda, DELAY_FLAG, delay_counter - processed_delays);
                DELAY_FLAG=false;

                //writeToFileUnformatted(fp, c);
                break;
            case GP:
                if (processed_delays < delay_counter) 
                {
                    double delay_time;
                    double queue_placed_time = 0.0;
                    int call_type;
                    
                    queue_placed_time = getTime(waiting, processed_delays);
                    delay_time = eventos->tempo - queue_placed_time;

                    if (delay_time > Ax)
                        Ax_counter++;

                    delays[processed_delays] = delay_time;
                    sum_delay += delays[processed_delays];

                    writeToFileUnformatted(fp2, delays[processed_delays]);

                    //Generate new arrival event
                    call_type = getCallType();
                    adicionar(eventos, call_type, eventos->tempo + getCallDuration(call_type));
                    processed_delays++;
                    busy++;

                    if (DEBUG)
                        printf("GP:tempo: %lf, proc_delay: %d\n", queue_placed_time, processed_delays);
                }
                    
                busy--;
                break;

            case GP_AS:
                transfered = adicionar(transfered, CHEGADA, eventos->tempo);
                break;

        }

        if (transfered != null)
        {
            switch (transfered->tipo)
            {
            case CHEGADA:
                areaSpec_busy++;

                if (areaSpec_busy > n_areaSpec_channels)
                {
                    areaSpec_busy = n_channels;
                    delay_areaSpec_counter++;  
                }
                else
                    transfered = adicionar(transfered, PARTIDA, eventos->tempo + getCallDuration(AS));
                break;
            
            case PARTIDA:
                
                if (areaSpec_processed_delays < delay_areaSpec_counter)
                {
                    //TODO: falta metricas dos delays, nao sei se é parecido com o caso GENERAL
                    adicionar(eventos, PARTIDA, eventos->tempo + getCallDuration(AS));
                    areaSpec_processed_delays++;
                    areaSpec_busy++;
                }

                areaSpec_busy--;
                break;
            }

            transfered = remover(transfered);
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
    printf("Probabilidade de atraso de pacote: %.3f\n", (double)delay_counter / (double)(amostras - block_counter));
    //Calcular media de atrasos dos pacotes
    printf("Media de atraso de pacote: %.6f\n", sum_delay / (double)amostras);

    printf("Probablidade do atraso do pacote ser maior que %.3f: %.3f\n", Ax, (double)Ax_counter/(double)delay_counter);
    printf("Blocks: %d\n", block_counter);
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
double generateNewEvent(lista *eventos, lista *waiting, int lambda, int isDelayed, int amountInQueue)
{
    double c, d, u1, u2, call_type;
    double current_time;

    int type;

    type = getCallType();

    //Calcular duracao do pacote
    d = getCallDuration(type);

    //Calcular intervalo entre chegada de eventos
    u1 = (double)random() / RAND_MAX;
    c = (-1 / (double)lambda) * log(u1);
        
    current_time = eventos->tempo;

    //Verifica o evento é delayed 
    if (isDelayed)
    {   //adiciona à waiting queue 
        if (amountInQueue < queue_size)               
            adicionar(waiting, 0, current_time);
    }
    else //Gerar partida deste evento
        adicionar(eventos, type, current_time + d);
        
    //Gerar chegada do proximo evento
    adicionar(eventos, CHEGADA, current_time + c);

    return c;
}

int getCallType()
{   
    int type;
    double call_type = (double)random() / RAND_MAX;

    if (call_type <= 0.3)
        type = GP;
    else 
        type = GP_AS;

    return type;
}

double getCallDuration(int type)
{
    double u, u1, u2, duration = 0.0;
    double theta, R;
    //Gerar numero aleatório entre 0 e 1
    u = (double)random() / RAND_MAX;
    u1 = (double)random() / RAND_MAX;
    u2 = (double)random() / RAND_MAX;

    if (type == GP) 
    {
        while (duration < 60 || duration > 300)
            duration = -120 * log(u);
    }
    else if (type == AS)
    {
         while (duration < 60)
            duration = -150 * log(u);
    }
    else 
    {
        while (duration < 30 || duration > 120)
        {
            theta = 2 * M_PI * u1;
            R = sqrt(-2 * log(u2));

            /*  Z = R*cos(theta) -> standart normal dist
                X = Z*dev + avg -> normal dist N(avg, dev^2)
            */
            duration = 20 * R * cos(theta) + 60;
        }
    }
    
    return duration;
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

