#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "Lista_ligada.h"

void addValueToHistogram(int *histograma, double value);
void writeToFileUnformatted(FILE *file_pointer, double value);

#define DEBUG 0
#define DELTA 0.5
#define N 10
#define TIME_INTERVAL 0.01

int main(int argc, char *argv[])
{
    int lambda, amostras;
    int n_amostras = 0, i = 0;
    double total_c = 0.0;
    double c, u;
    int *histograma;
    FILE *fp, *fp2;
    lista *eventos;

    if (argc < 2)
    {
        printf("Usage: [progname] [lambda] [numero de time_intervals]\n");
        return 1;
    }

    histograma = (int *)calloc(N, sizeof(int));

    lambda = atoi(argv[1]);
    amostras = atoi(argv[2]);
    if (DEBUG)
        printf("Lambda = %d\nAmostras = %d\n", lambda, amostras);

    srand(time(NULL));

    fp2 = fopen("./c_values_log_parteb.txt", "w");

    //Adicionar o primeiro evento no tempo = 0
    eventos = adicionar(NULL, CHEGADA, total_c);

    while (n_amostras < amostras)
    {
        double current_time = (double)i * TIME_INTERVAL;
        //Gerar numero aleatório entre 0 e 1
        u = (double)random() / RAND_MAX;
        if (DEBUG)
            printf("u = %f\t", u);

        //Calcular intervalo entre chegada de eventos usando a
        //definicao de Poisson Process
        if (u <= TIME_INTERVAL * lambda)
        {
            c = current_time - eventos->tempo;
            total_c += c;
            //Remove o evento anterior e coloca o novo
            remover(eventos);
            eventos = adicionar(NULL, CHEGADA, current_time);

            writeToFileUnformatted(fp2, c);
            addValueToHistogram(histograma, c);
            n_amostras++;
        }
        i++;
    }

    //Calcular valor médio entre chegada de eventos
    printf("Valor médio entre chegada de eventos: %.2f\n", total_c / ((double)amostras));

    if (DEBUG)
    {
        printf("Histograma\n");
        for (int i = 0; i < N; i++)
        {
            printf("%d -> %d\n", i, histograma[i]);
        }
    }

    histograma[N - 1] = amostras;

    //Guardar histograma num ficheiro com os valores separados por espaço
    fp = fopen("./histograma_log.txt", "w");
    for (int i = 0; i < N; i++)
    {
        char value[5];
        sprintf(value, "%d", histograma[i]);
        fputs(value, fp);
        fputs(" ", fp);
    }

    fclose(fp);
    fclose(fp2);
    free(histograma);
    return 0;
}

void addValueToHistogram(int *histograma, double value)
{
    for (int i = 0; i < N - 1; i++)
    {
        if (value < (double)(i + 1) * DELTA)
        {
            histograma[i] += 1;
            return;
        }
    }
}

//Usado para criar um histograma no Excel
void writeToFileUnformatted(FILE *file_pointer, double value)
{
    char value_char[100];
    sprintf(value_char, "%.2f", value);
    fputs(value_char, file_pointer);
    fputs(" ", file_pointer);
}