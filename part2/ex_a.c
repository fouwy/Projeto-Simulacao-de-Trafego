#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "Lista_ligada.h"

void addValueToHistogram(int *histograma, double value);
void writeToFileUnformatted(FILE *file_pointer, double value);
void generateNewEvent(lista *eventos, int lambda);

#define DEBUG 0
#define dm 0.008

//TODO: Entrada -> N canais
//      Por lambda a 200
//      d = -dm*ln(u) , u diferente do usado para calcular c
//      var busy-> incrementa sempre que chegar chamada, decrementa
//      sempre que for antedida
//        if(==N) nao antende mais chamadas, incrementa blocked
int main(int argc, char *argv[])
{
    int i = 0;
    int busy = 0, blocked = 0;
    int lambda, amostras, n_channels;
    
    double total_c = 0.0;
    double c, d, u1, u2, big_d;

    FILE *fp;
    lista *eventos;

    if (argc < 3)
    {
        printf("Usage: [progname] [lambda] [N canais] [numero de amostras]\n");
        return 0;
    }

    lambda = atoi(argv[1]);
    n_channels = atoi(argv[2]);
    amostras = atoi(argv[3]);

    if (DEBUG)
        printf("Lambda = %d\nAmostras = %d\nN Canais = %d\n", lambda, amostras, n_channels);

    srand(time(NULL));

    fp = fopen("part2_a_log.txt", "w");
    
    //Adicionar o primeiro evento no tempo = 0
    eventos = adicionar(NULL, CHEGADA, total_c);
    busy++;
    generateNewEvent(eventos, lambda);

    while (i < amostras)
    {
        //Remove o evento anterior e coloca o novo
        eventos = remover(eventos);

        switch(eventos->tipo)
        {
            case CHEGADA:
                i++;
                busy++;
                generateNewEvent(eventos, lambda);
                break;
            case PARTIDA:
                busy--;
                break;
        }

        if (busy > n_channels)
        {
            busy = n_channels;
            blocked++;
        }

        //FIXME: o que fazer com isto? mudar de c para eventos->qlq coisa
        writeToFileUnformatted(fp, c);
    }

    if(DEBUG)
        printf("media d: %f\n", big_d/(double)amostras);

    //Calcular valor médio entre chegada de eventos
    printf("Valor médio entre chegada de eventos: %.2f\n", total_c / ((double)amostras));

    fclose(fp);
    return 0;
}

//Usado para criar um histograma no Excel.
//Envia para o ficheiro apontado por file_pointer os valores de c separados por espaços
void writeToFileUnformatted(FILE *file_pointer, double value)
{
    char value_char[100];
    sprintf(value_char, "%.2f", value);
    fputs(value_char, file_pointer);
    fputs(" ", file_pointer);
}

void generateNewEvent(lista *eventos, int lambda)
{
    double c, d, u1, u2;
    double total_c;

    //Gerar numero aleatório entre 0 e 1
    u1 = (double)random() / RAND_MAX;
    u2 = (double)random() / RAND_MAX;

    //Calcular duracao do pacote
    d = -dm * log(u2);

    //Calcular intervalo entre chegada de eventos
    c = (-1 / (double)lambda) * log(u1);
    if (DEBUG)
    {
        printf("u = %f\t", u1);
        printf("c = %f\n", c);
    }
        
    total_c = eventos->tempo;

    //Gerar partida deste evento e chegada do proximo
    adicionar(eventos, PARTIDA, total_c + d);
    adicionar(eventos, CHEGADA, total_c + c);
}