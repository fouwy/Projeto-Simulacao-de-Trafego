#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

void addValueToHistogram(int *histograma, double value);
void writeToFileUnformatted(FILE *file_pointer, double value);

#define DEBUG 0
#define DELTA 0.5
#define N 10

int main(int argc, char *argv[])
{
    int lambda, amostras;
    double soma = 0.0;
    double c, u;
    int *histograma;
    FILE *fp, *fp2;

    if (argc < 2)
    {
        printf("Usage: [progname] [lambda] [numero de amostras]\n");
        return 1;
    }

    histograma = (int *)calloc(N, sizeof(int));

    lambda = atoi(argv[1]);
    amostras = atoi(argv[2]);
    if (DEBUG)
        printf("Lambda = %d\nAmostras = %d\n", lambda, amostras);

    srand(time(NULL));

    fp2 = fopen("./c_values_log.txt", "w");

    for (int i = 0; i < amostras; i++)
    {
        //Gerar numero aleatório entre 0 e 1
        u = (double)random() / RAND_MAX;
        if (DEBUG)
            printf("u = %f\t", u);

        //Calcular intervalo entre chegada de eventos
        c = (-1 / (double)lambda) * log(u);
        if (DEBUG)
            printf("c = %f\n", c);

        soma += c;
        writeToFileUnformatted(fp2, c);
        addValueToHistogram(histograma, c);
    }

    //Calcular valor médio entre chegada de eventos
    printf("Valor médio entre chegada de eventos: %.2f\n", soma / ((double)amostras));

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