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
double generateNewEvent(lista *eventos, lista *waiting, double lambda, int isDelayed, int amountInQueue);
double getTime(lista *apontador, int position);
double getCallDuration(int type);
int getCallType();

#define DEBUG 0
#define true 1
#define false 0
#define null NULL

int LOST_FLAG = false;
int GP_AS_counter = 0;
int queue_size = 0, avg_counter = 0;
double *running_avg;
double *GP_AS_time;

int main(int argc, char *argv[])
{
    int sample_counter = 0;
    int busy = 0, delay_counter = 0, lost_counter = 0, 
        processed_delays = 0, GP_delay_counter = 0;
    int amostras, n_channels;
    int DELAY_FLAG = false;

    double current_time = 0.0;          
    double sum_delay = 0.0, sum_error = 0.0, sum_relative = 0.0;
    double *delays, *GP_delay;
    double *prediction;
    double *error, *relative_error;
    
    double lambda;

    FILE *fp, *fp2, *fp3, *fp4;
    lista *eventos, *waiting;
    lista *transfered, *waitingAS;

    //AREA SPECIFIC
    int delay_areaSpec_counter = 0, areaSpec_processed_delays = 0;
    int areaSpec_busy = 0, n_areaSpec_channels;
    int transfered_counter = 0;
    double sum_GP_AS_time = 0.0, sum_total_delay = 0.0;

    if (argc < 5) //<5 lambda [ponto 5.]
    {
        printf("Usage: [progname] [N canais GP] [N canais AS] [numero de amostras] [tamanho da fila]\n");
        return 0;
    }

    //lambda = atoi(argv[1]);
    lambda = 0.02222;
    n_channels = atoi(argv[1]);
    n_areaSpec_channels = atoi(argv[2]);
    amostras = atoi(argv[3]);
    queue_size = atoi(argv[4]);

    if (DEBUG)
        printf("Lambda = %f\nAmostras = %d\nN Canais = %d\n", lambda, amostras, n_channels);

    srand(time(null));

    delays = (double*)calloc(amostras, sizeof(double));
    running_avg = (double*)calloc(amostras, sizeof(double));
    prediction = (double*)calloc(amostras, sizeof(double));
    error = (double*)calloc(amostras, sizeof(double));
    relative_error = (double*)calloc(amostras, sizeof(double));
    GP_AS_time = (double*)calloc(amostras, sizeof(double));
    GP_delay = (double*)calloc(amostras, sizeof(double));

    fp = fopen("error.txt", "w");
    fp2 = fopen("delays.txt", "w");
    fp3 = fopen("predictions.txt", "w");
    fp4 = fopen("waitingAS.txt", "w");
    
    waiting = adicionar(null, 0, 0);
    waitingAS = adicionar(null, 0, 0);

    //Adicionar o primeiro evento no tempo = 0
    eventos = adicionar(null, CHEGADA, current_time);
    running_avg[0] = 0;
    avg_counter++;
    busy++;

    generateNewEvent(eventos, waiting, lambda, DELAY_FLAG, 0);

    while (eventos != null || transfered != null)
    {
        if (DEBUG)
        {
            imprimir(eventos);
            printf("Busy: %d\n", busy);
        }

        if (eventos != null)
        {
            current_time = eventos->tempo;
            //Remove o evento anterior e coloca o novo
            eventos = remover(eventos);
        }

        if (eventos != null)
        {
            switch(eventos->tipo)
            {
                int amountInQueue;
                double delay_time;
                double queue_placed_time;
                int call_type;
                int duration;
                
                case CHEGADA:

                    if (sample_counter >= amostras)
                        break;

                    amountInQueue = delay_counter - processed_delays;

                    sample_counter++;
                    busy++;

                    if (busy > n_channels)
                    {
                        busy = n_channels;

                        if (amountInQueue < queue_size) 
                        {
                            prediction[delay_counter] = running_avg[avg_counter-1];
                            writeToFileUnformatted(fp3, prediction[delay_counter]);
                            delay_counter++;
                            DELAY_FLAG = true;  
                        }   
                        else 
                        {
                            lost_counter++;
                            LOST_FLAG = true;
                        }
                        
                    }
                    //printf("CHEGADA:tempo: %d , busy: %d-> delay_counter:%d, processed_delays:%d\n", (int)eventos->tempo, busy, delay_counter, processed_delays);
                    generateNewEvent(eventos, waiting, lambda, DELAY_FLAG, delay_counter - processed_delays - 1); // -1 because we are not counting this event when calculating the current lenght of queue
                    DELAY_FLAG = false;
                    LOST_FLAG = false;

                    //writeToFileUnformatted(fp, c); 
                    break;
                case GP:
                    if (processed_delays < delay_counter) 
                    { 
                        queue_placed_time = getTime(waiting, processed_delays); //time returned from waiting list where: position = processed_delays
                        delay_time = eventos->tempo - queue_placed_time;        //diference from current-time and placement on waiting queue time
                        
                        delays[processed_delays] = delay_time;  //stores delays
                        sum_delay += delays[processed_delays];  

                        writeToFileUnformatted(fp2, delays[processed_delays]);

                        //Generate new arrival event
                        call_type = getCallType();
                        duration = getCallDuration(call_type);
                        adicionar(eventos, call_type, eventos->tempo + duration);

                        if (call_type == GP_AS)
                        {
                            GP_AS_time[GP_AS_counter] = delay_time; //+ duration;
                            GP_AS_counter++;
                        }
                        else 
                        {
                            GP_delay[GP_delay_counter] = delay_time;
                            GP_delay_counter++;
                        }

                        //Calculate running average of call duration
                        running_avg[avg_counter] = running_avg[avg_counter-1] * (double)avg_counter / (double)(avg_counter+1)
                                        + delay_time * 1 / (double)(avg_counter+1);
                        avg_counter++;
                        
                        processed_delays++;
                        busy++;

                        if (0)
                            printf("GP:tempo: %d, waiting: %d\n", (int)eventos->tempo, (int)queue_placed_time);
                    }

                    busy--;
                    break;

                case GP_AS:
                    if (processed_delays < delay_counter)
                    {                       
                        queue_placed_time = getTime(waiting, processed_delays); 
                        delay_time = eventos->tempo - queue_placed_time;    

                        delays[processed_delays] = delay_time;
                        sum_delay += delays[processed_delays];

                        writeToFileUnformatted(fp2, delays[processed_delays]);

                        //Generate new arrival event
                        call_type = getCallType();
                        duration = getCallDuration(call_type);
                        adicionar(eventos, call_type, eventos->tempo + duration);

                        if (call_type == GP_AS)
                        {
                            GP_AS_time[GP_AS_counter] = delay_time;// + duration;
                            GP_AS_counter++;
                        } 
                        else 
                        {
                            GP_delay[GP_delay_counter] = delay_time;
                            GP_delay_counter++;
                        }

                        //Calculate running average of call duration
                        running_avg[avg_counter] = running_avg[avg_counter-1] * (double)avg_counter / (double)(avg_counter+1)
                                        + delay_time * 1 / (double)(avg_counter+1);
                        avg_counter++;

                        processed_delays++;
                        busy++;

                        if (0)
                            printf("AS_gp:tempo: %d, waiting: %d\n", (int)eventos->tempo, (int)queue_placed_time);
                        
                    }
                    busy--; //general purpose part is processed when  
                    //transfer call 
                    transfered = adicionar(transfered, CHEGADA, eventos->tempo);
                    break;
            }
        }

        if (transfered != null)
        {
            switch (transfered->tipo)
            {
            double delay_time;
            double queue_placed_time;

            case CHEGADA:
                areaSpec_busy++;
                if (areaSpec_busy > n_areaSpec_channels)
                {   
                    areaSpec_busy = n_areaSpec_channels;
                    delay_areaSpec_counter++;
                    adicionar(waitingAS, 0, eventos->tempo);
                }
                else 
                {   
                    transfered_counter++;
                    transfered = adicionar(transfered, PARTIDA, eventos->tempo + getCallDuration(AS));
                }
                break;
            
            case PARTIDA:
                if (areaSpec_processed_delays < delay_areaSpec_counter)
                {   
                    queue_placed_time = getTime(waitingAS, areaSpec_processed_delays); 
                    delay_time = transfered->tempo - queue_placed_time;
                    GP_AS_time[transfered_counter] += delay_time;
                    transfered_counter++;

                    adicionar(transfered, PARTIDA, transfered->tempo + getCallDuration(AS));
                    areaSpec_processed_delays++;
                    areaSpec_busy++;

                    //printf("delay time AS: %f\n", delay_time);
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
   
    if (0)
        {
            printf("WAITING\n");
            imprimir(waitingAS);
        }

    for (int i=0; i<transfered_counter; i++) {
        sum_GP_AS_time += GP_AS_time[i];
        writeToFileUnformatted(fp4, GP_AS_time[i]);
    }

    for(int i= 0; i<GP_delay_counter; i++) {
        sum_total_delay += GP_delay[i];
    }

    sum_total_delay += sum_GP_AS_time;

    for (int i=0; i<delay_counter; i++) {
        error[i] = fabs(prediction[i] - delays[i]);
        relative_error[i] = error[i] / delays[i];
        sum_error += error[i];
        sum_relative += relative_error[i];
        writeToFileUnformatted(fp, error[i]);
    }

    //Calcular valor médio entre chegada de eventos
    //printf("Valor médio entre chegada de eventos: %.6f\n", current_time / ((double)amostras));
    
    printf("sample count: %d\n", sample_counter);
    printf("Operadores GP: %d, Operadores AS: %d, Tamanho da fila GP: %d\n", n_channels, n_areaSpec_channels, queue_size);

    printf("Probabilidade de atraso de chamada: %.2f %%\n", (double)delay_counter / (double)(amostras - lost_counter) * 100.0);
    printf("Probabilidade chamada é perdida: %.2f %%\n", (double)lost_counter / (double)amostras * 100.0);

    // printf("Delays: %d\n", delay_counter);
    // printf("Blocks: %d\n", lost_counter);
    // printf("sum_relative: %f\n", sum_relative);
    
    //Calcular media de atrasos dos pacotes
    printf("Média de atraso de chamada: %.2f seg\n", sum_delay / (double)delay_counter);
    // printf("Média de atraso de chamada(com delays de zero incl): %.3f seg\n", sum_delay / (double)(amostras - lost_counter));
    printf("Média de tempo entre a chegada de chamada ao GP e o atendimento no AS: %.2f seg\n", sum_GP_AS_time/(double)transfered_counter);
    printf("Média de erro absoluto de previsao de tempo de espera: %.2f seg\n", sum_error/(double)delay_counter);
    printf("Média de erro relativo de previsao de tempo de espera: %.2f %%\n", sum_error/sum_delay *100.0);
    printf("Média do atraso total das chamadas: %.2f seg\n", sum_total_delay/(double)(transfered_counter+GP_delay_counter));
    printf("transfered: %d\n", transfered_counter);
    printf("GP_delay_counter: %d\n", GP_delay_counter);

    // for (int i = 0; i<avg_counter; i++)
    //     printf("Running avg:%d --> %f\n", i, running_avg[i]);

    // for (int i = 0; i<delay_counter; i++)
    //     printf("Prediction avg:%d --> %f\n", i, prediction[i]);

    fclose(fp);
    fclose(fp2);
    fclose(fp3);
    fclose(fp4);
    return 0;
}

//Usado para criar um histograma no Excel.
//Envia para o ficheiro apontado por file_pointer os valores de c separados por espaços
void writeToFileUnformatted(FILE *file_pointer, double value)
{
    char value_char[100];
    sprintf(value_char, "%.6f", value);
    fputs(value_char, file_pointer);
    fputs("\n", file_pointer);
}

/*Gera uma partida e uma chegada a partir da chegada recebida
  Retorna o valor de c
*/
double generateNewEvent(lista *eventos, lista *waiting, double lambda, int isDelayed, int amountInQueue)
{
    double c, d, u1;
    double current_time;
    int type;

    type = getCallType();

    //Calcular duracao da chamada
    d = getCallDuration(type);

    //Calcular intervalo entre chegada de eventos
    u1 = (double)random() / RAND_MAX;
    c = (-1 / lambda) * log(u1);
        
    current_time = eventos->tempo;

    //Verifica o evento é delayed 
    if (isDelayed)
    {   //adiciona à waiting queue 
        if (amountInQueue < queue_size)  {    
            adicionar(waiting, 0, current_time);
        }
    }
    else if (LOST_FLAG == false) //Gerar partida deste evento
    {    
        if (type == GP_AS) 
        {
            // GP_AS_time[GP_AS_counter] = d;
            // GP_AS_counter++;
        }

        adicionar(eventos, type, current_time + d);
    }   
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

    if (type == GP) 
    {
        while (duration < 60 || duration > 300) {
             u = (double)random() / (double)RAND_MAX;
            duration = -120 * log(u);
        }
    }
    else if (type == AS)
    {
         while (duration < 60) {
            u = (double)random() / (double)RAND_MAX;
            duration = -150.0 * log(u);
         }
    }
    else 
    {
        while (duration < 30 || duration > 120)
        {
            u1 = (double)random() / (double)RAND_MAX;
            u2 = (double)random() / (double)RAND_MAX;
            theta = 2.0 * M_PI * u1;
            R = sqrt(-2.0 * log(u2));

            /*  Z = R*cos(theta) -> standart normal dist
                X = Z*dev + avg -> normal dist N(avg, dev^2)
            */
            duration = 20.0 * R * cos(theta) + 60.0;
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

