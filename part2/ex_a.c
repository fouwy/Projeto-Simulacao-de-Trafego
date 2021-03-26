#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "Lista_ligada.h"

void addValueToHistogram(int *histograma, double value);
void writeToFileUnformatted(FILE *file_pointer, double value);
double generateNewEvent(lista *eventos, int lambda, int isBlocked);

#define DEBUG 0
#define dm 0.008
#define TRUE 1
#define FALSE 0

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
    int BLOCKED_FLAG = false;

    double current_time = 0.0;
    double c;

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
    eventos = adicionar(NULL, CHEGADA, current_time);
    busy++;
    generateNewEvent(eventos, lambda, BLOCKED_FLAG);

    while (i < amostras)
    {
        if (DEBUG)
        {
            imprimir(eventos);
            printf("Busy: %d\n", busy);
        }
        //Remove o evento anterior e coloca o novo
        eventos = remover(eventos);

        switch (eventos->tipo)
        {
        case CHEGADA:
            i++;
            busy++;
            if (busy > n_channels)
            {
                busy = n_channels;
                blocked++;
                BLOCKED_FLAG = true;
            }
            c = generateNewEvent(eventos, lambda, BLOCKED_FLAG);
            BLOCKED_FLAG = false;

            writeToFileUnformatted(fp, c);
            break;
        case PARTIDA:
            busy--;
            break;
        }
    }

    current_time = eventos->tempo;

    //Calcular valor médio entre chegada de eventos
    printf("Valor médio entre chegada de eventos: %.6f\n", current_time / ((double)amostras));
    printf("Blocks: %d\n", blocked);
    printf("Probabilidade de blocking: %.3f\n", (double)blocked / (double)amostras);
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
double generateNewEvent(lista *eventos, int lambda, int isBlocked)
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
    if (DEBUG)
    {
        printf("u = %f\t", u1);
        printf("c = %f\n", c);
    }

    current_time = eventos->tempo;

    //Gerar partida deste evento e chegada do proximo
    if (!isBlocked)
        adicionar(eventos, PARTIDA, current_time + d);
    adicionar(eventos, CHEGADA, current_time + c);

    return c;
}